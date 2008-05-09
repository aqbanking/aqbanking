/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define AO_PROVIDER_HEAVY_DEBUG

#include "provider_p.h"
#include "account.h"
#include "queues_l.h"
#include "user.h"

#include <aqbanking/account_be.h>
#include <aqbanking/job_be.h>
#include <aqbanking/jobgetbalance_be.h>
#include <aqbanking/jobgettransactions_be.h>
#include <aqbanking/jobsingletransfer_be.h>
#include <aqbanking/jobsingledebitnote_be.h>
#include <aqbanking/value.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/bio_buffer.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/process.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/i18n.h>

#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18N_NOOP(msg) msg
#define I18S(msg) msg

#define I18N_NOOP(msg) msg



GWEN_INHERIT(AB_PROVIDER, AO_PROVIDER)



AB_PROVIDER *AO_Provider_new(AB_BANKING *ab){
  AB_PROVIDER *pro;
  AO_PROVIDER *dp;

  pro=AB_Provider_new(ab, "aqofxconnect");
  GWEN_NEW_OBJECT(AO_PROVIDER, dp);
  GWEN_INHERIT_SETDATA(AB_PROVIDER, AO_PROVIDER, pro, dp,
                       AO_Provider_FreeData);
  dp->bankingJobs=AB_Job_List2_new();
  dp->queue=AO_Queue_new();

  AB_Provider_SetInitFn(pro, AO_Provider_Init);
  AB_Provider_SetFiniFn(pro, AO_Provider_Fini);
  AB_Provider_SetUpdateJobFn(pro, AO_Provider_UpdateJob);
  AB_Provider_SetAddJobFn(pro, AO_Provider_AddJob);
  AB_Provider_SetExecuteFn(pro, AO_Provider_Execute);
  AB_Provider_SetResetQueueFn(pro, AO_Provider_ResetQueue);
  AB_Provider_SetExtendUserFn(pro, AO_Provider_ExtendUser);
  AB_Provider_SetExtendAccountFn(pro, AO_Provider_ExtendAccount);

  return pro;
}



void GWENHYWFAR_CB AO_Provider_FreeData(void *bp, void *p) {
  AO_PROVIDER *dp;

  dp=(AO_PROVIDER*)p;
  assert(dp);

  AO_Queue_free(dp->queue);
  AB_Job_List2_free(dp->bankingJobs);

  GWEN_FREE_OBJECT(dp);
}



int AO_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData) {
  AO_PROVIDER *dp;
  const char *logLevelName;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  logLevelName=getenv("AQOFXCONNECT_LOGLEVEL");
  if (logLevelName) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(logLevelName);
    if (ll!=GWEN_LoggerLevel_Unknown) {
      GWEN_Logger_SetLevel(AQOFXCONNECT_LOGDOMAIN, ll);
      DBG_WARN(AQOFXCONNECT_LOGDOMAIN,
               "Overriding loglevel for AqOFXConnect with \"%s\"",
               logLevelName);
    }
    else {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                "Unknown loglevel \"%s\"",
                logLevelName);
    }
  }

  dp->dbConfig=dbData;
  dp->lastJobId=GWEN_DB_GetIntValue(dp->dbConfig, "lastJobId", 0, 0);
  dp->connectTimeout=GWEN_DB_GetIntValue(dp->dbConfig, "connectTimeout", 0,
                                         AO_PROVIDER_CONNECT_TIMEOUT);
  dp->sendTimeout=GWEN_DB_GetIntValue(dp->dbConfig, "sendTimeout", 0,
                                      AO_PROVIDER_SEND_TIMEOUT);
  dp->recvTimeout=GWEN_DB_GetIntValue(dp->dbConfig, "recvTimeout", 0,
                                      AO_PROVIDER_RECV_TIMEOUT);

  return 0;
}



int AO_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData){
  AO_PROVIDER *dp;
  int errors=0;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Deinitializing AqOFXDC backend");

  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "lastJobId", dp->lastJobId);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "connectTimeout", dp->connectTimeout);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "sendTimeout", dp->sendTimeout);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "recvTimeout", dp->recvTimeout);

  dp->dbConfig=0;
  AO_Queue_Clear(dp->queue);
  AB_Job_List2_Clear(dp->bankingJobs);

  DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Deinit done");

  if (errors)
    return GWEN_ERROR_GENERIC;

  return 0;
}



int AO_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j, uint32_t guiid){
  AO_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  switch(AB_Job_GetType(j)) {
  case AB_Job_TypeGetBalance:
    /* no parameters to exchange */
    return 0;
  case AB_Job_TypeGetTransactions:
    /* no parameters to exchange */
    return 0;

  case AB_Job_TypeTransfer:
  case AB_Job_TypeDebitNote:
  default:
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "Job not supported (%d)",
             AB_Job_GetType(j));
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */
}



int AO_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j, uint32_t guiid){
  AO_PROVIDER *dp;
  AB_ACCOUNT *a;
  AB_USER *u;
  AO_USERQUEUE *uq;
  int doAdd=1;
  GWEN_DB_NODE *dbJob;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  a=AB_Job_GetAccount(j);
  assert(a);

  u=AB_Account_GetFirstUser(a);
  assert(u);

  dbJob=AB_Job_GetProviderData(j, pro);
  assert(dbJob);

  switch(AB_Job_GetType(j)) {
  case AB_Job_TypeGetBalance:
  case AB_Job_TypeGetTransactions:
    break;

  case AB_Job_TypeTransfer:
  case AB_Job_TypeDebitNote:
  default:
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "Job not supported (%d)",
             AB_Job_GetType(j));
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */


  uq=AO_Queue_GetUserQueue(dp->queue, u);
  assert(uq);

  if (AB_Job_GetType(j)==AB_Job_TypeGetBalance) {
    AB_JOB_LIST2_ITERATOR *jit;

    /* check whether a getBalance job already exists. If it does then
     * we don't have to send this job again, once is enough. */
    jit=AB_Job_List2_First(AO_UserQueue_GetJobs(uq));
    if (jit) {
      AB_JOB *uj;

      uj=AB_Job_List2Iterator_Data(jit);
      assert(uj);
      while(uj) {
	AB_JOB_TYPE jt;

	jt=AB_Job_GetType(uj);
	if (jt==AB_Job_TypeGetBalance ||
	    jt==AB_Job_TypeGetTransactions) {
	  if (AB_Job_GetAccount(j)==AB_Job_GetAccount(uj)) {
	    GWEN_DB_NODE *dbCurrJob;

	    dbCurrJob=AB_Job_GetProviderData(uj, pro);
	    assert(dbCurrJob);
	    GWEN_DB_SetIntValue(dbJob,
				GWEN_DB_FLAGS_OVERWRITE_VARS,
				"refJob",
				AB_Job_GetJobId(uj));
	    doAdd=0;
	    break;
	  }
	}
	uj=AB_Job_List2Iterator_Next(jit);
      } /* while */
      AB_Job_List2Iterator_free(jit);
    }
  }
  else if (AB_Job_GetType(j)==AB_Job_TypeGetTransactions) {
    AB_JOB_LIST2_ITERATOR *jit;
    const GWEN_TIME *tnew;

    /* check whether a getTransactions job already exists. If it does then
     * we don't have to send this job again, once is enough. */
    tnew=AB_JobGetTransactions_GetFromTime(j);
    jit=AB_Job_List2_First(AO_UserQueue_GetJobs(uq));
    if (jit) {
      AB_JOB *uj;

      uj=AB_Job_List2Iterator_Data(jit);
      assert(uj);
      while(uj) {
	AB_JOB_TYPE jt;

	jt=AB_Job_GetType(uj);
	if (jt==AB_Job_TypeGetTransactions) {
	  if (AB_Job_GetAccount(j)==AB_Job_GetAccount(uj)) {
	    GWEN_DB_NODE *dbCurrJob;

	    dbCurrJob=AB_Job_GetProviderData(uj, pro);
	    assert(dbCurrJob);

	    if (tnew) {
	      const GWEN_TIME *tcurr;

	      tcurr=AB_JobGetTransactions_GetFromTime(uj);
	      if (tcurr) {
		/* current job has a time */
		if (GWEN_Time_Diff(tcurr, tnew)>0.0) {
		  /* new time is before that of current job, replace */
		  GWEN_DB_SetIntValue(dbCurrJob,
				      GWEN_DB_FLAGS_OVERWRITE_VARS,
				      "refJob",
				      AB_Job_GetJobId(j));
		  AB_Job_List2_Erase(AO_UserQueue_GetJobs(uq), jit);
		  doAdd=1;
		  break;
		}
	      }
	      else {
		/* current job has no time, so replace by job with time */
		GWEN_DB_SetIntValue(dbCurrJob,
				    GWEN_DB_FLAGS_OVERWRITE_VARS,
				    "refJob",
				    AB_Job_GetJobId(j));
		AB_Job_List2_Erase(AO_UserQueue_GetJobs(uq), jit);
		doAdd=1;
		break;
	      }
	    }
	    else {
	      /* new job has no time, so don't add it */
	      doAdd=0;
	      GWEN_DB_SetIntValue(dbJob,
				  GWEN_DB_FLAGS_OVERWRITE_VARS,
				  "refJob",
				  AB_Job_GetJobId(uj));
	      break;
	    }
	  } /* if same account */
	} /* if GetTransactions */
	uj=AB_Job_List2Iterator_Next(jit);
      } /* while */
      AB_Job_List2Iterator_free(jit);
    }
  }

  if (doAdd) {
    /* only add to queue if needed */
    AO_UserQueue_AddJob(uq, j);
  }

  /* always add to linear list */
  AB_Job_List2_PushBack(dp->bankingJobs, j);
  return 0;
}



int AO_Provider_ResetQueue(AB_PROVIDER *pro){
  AO_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  AO_Queue_Clear(dp->queue);
  AB_Job_List2_Clear(dp->bankingJobs);

  return 0;
}



int AO_Provider_CountDoneJobs(AB_JOB_LIST2 *jl){
  AB_JOB_LIST2_ITERATOR *jit;
  int cnt=0;

  jit=AB_Job_List2_First(jl);
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while(uj) {
      AB_JOB_STATUS js;

      js=AB_Job_GetStatus(uj);
      if (js==AB_Job_StatusFinished ||
	  js==AB_Job_StatusError)
	cnt++;
      uj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  return cnt;
}



AB_JOB *AO_Provider_FindJobById(AB_JOB_LIST2 *jl, uint32_t jid) {
  AB_JOB_LIST2_ITERATOR *jit;

  jit=AB_Job_List2_First(jl);
  if (jit) {
    AB_JOB *j;

    j=AB_Job_List2Iterator_Data(jit);
    assert(j);
    while(j) {
      if (AB_Job_GetJobId(j)==jid) {
	AB_Job_List2Iterator_free(jit);
        return j;
      }
      j=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  return 0;
}



int AO_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx,
			uint32_t guiid){
  AO_PROVIDER *dp;
  int oks=0;
  int errors=0;
  AB_JOB_LIST2_ITERATOR *jit;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  rv=AO_Provider_ExecQueue(pro, ctx, guiid);
  if (!rv)
    oks++;
  else {
    errors++;
    if (rv==GWEN_ERROR_USER_ABORTED) {
      AO_Queue_Clear(dp->queue);
      AB_Job_List2_Clear(dp->bankingJobs);
      return rv;
    }
  }

  /* set results in referencing jobs, too */
  jit=AB_Job_List2_First(dp->bankingJobs);
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while(uj) {
      if (AB_Job_GetStatus(uj)==AB_Job_StatusSent) {
	AB_JOB *rj;
	uint32_t rjid;

	rj=uj;
        /* find referenced job (if any) */
	do {
	  GWEN_DB_NODE *dbT;

	  dbT=AB_Job_GetProviderData(rj, pro);
	  assert(dbT);
	  rjid=GWEN_DB_GetIntValue(dbT, "refJob", 0, 0);
	  if (rjid) {
	    rj=AO_Provider_FindJobById(dp->bankingJobs, rjid);
	  }
	} while(rjid && rj);

	if (rj && rj!=uj) {
	  /* found referenced job, copy status and result text */
	  DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
		    "Copying status from referenced job");
	  AB_Job_SetStatus(uj, AB_Job_GetStatus(rj));
	  AB_Job_SetResultText(uj, AB_Job_GetResultText(rj));
	}
	if (AB_Job_GetStatus(uj)==AB_Job_StatusSent)
	  AB_Job_SetStatus(uj, AB_Job_StatusFinished);
      }
      uj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  rv=AB_Banking_ExecutionProgress(AB_Provider_GetBanking(pro), 0);
  if (rv==GWEN_ERROR_USER_ABORTED) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "User aborted");
    return rv;
  }

  AO_Queue_Clear(dp->queue);
  AB_Job_List2_Clear(dp->bankingJobs);

  if (!oks && errors) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Not a single job succeeded");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int AO_Provider__ProcessImporterContext(AB_PROVIDER *pro,
					AB_USER *u,
					AB_IMEXPORTER_CONTEXT *ictx,
					uint32_t guiid){
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  assert(pro);
  assert(ictx);

  ai=AB_ImExporterContext_GetFirstAccountInfo(ictx);
  if (!ai) {
    DBG_INFO(0, "No accounts");
  }
  while(ai) {
    const char *country;
    const char *bankCode;
    const char *accountNumber;

    country=AB_User_GetCountry(u);
    if (!country)
      country="us";
    bankCode=AB_ImExporterAccountInfo_GetBankCode(ai);
    if (!bankCode || !*bankCode)
      bankCode=AB_User_GetBankCode(u);
    accountNumber=AB_ImExporterAccountInfo_GetAccountNumber(ai);
    if (bankCode && accountNumber) {
      AB_ACCOUNT *a;
      const char *s;

      a=AB_Banking_FindAccount(AB_Provider_GetBanking(pro),
                               AQOFXCONNECT_BACKENDNAME,
                               country, bankCode, accountNumber);
      if (!a) {
        char msg[]=I18N_NOOP("Adding account %s to bank %s");
        char msgbuf[512];

        DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Adding account %s to bank %s",
                  accountNumber, bankCode);

        /* account does not exist, add it */
        a=AB_Banking_CreateAccount(AB_Provider_GetBanking(pro),
                                   AQOFXCONNECT_BACKENDNAME);
        assert(a);
        AB_Account_SetCountry(a, country);
        AB_Account_SetBankCode(a, bankCode);
        AB_Account_SetAccountNumber(a, accountNumber);
        AB_Account_SetUser(a, u);
        s=AB_ImExporterAccountInfo_GetBankName(ai);
        if (!s)
          s=bankCode;
        AB_Account_SetBankName(a, s);
        AB_Account_SetAccountType(a, AB_ImExporterAccountInfo_GetType(ai));

        snprintf(msgbuf, sizeof(msgbuf), I18N(msg),
                 accountNumber, bankCode);
	GWEN_Gui_ProgressLog(guiid,
			     GWEN_LoggerLevel_Notice,
			     msgbuf);
        AB_Banking_AddAccount(AB_Provider_GetBanking(pro),a );
      }
      else {
        DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
                  "Account %s at bank %s already exists",
                  accountNumber, bankCode);
      }
      /* update existing account */
      s=AB_ImExporterAccountInfo_GetBankName(ai);
      if (s) {
        AB_Account_SetBankName(a, s);
      }
      s=AB_ImExporterAccountInfo_GetAccountName(ai);
      if (s)
        AB_Account_SetAccountName(a, s);
    }
    else {
      DBG_WARN(AQOFXCONNECT_LOGDOMAIN,
                "BankCode or AccountNumber missing (%s/%s)",
                bankCode, accountNumber);
    }
    ai=AB_ImExporterContext_GetNextAccountInfo(ictx);
  } /* while accounts */

  return 0;
}



int AO_Provider_RequestAccounts(AB_PROVIDER *pro,
				AB_USER *u,
				uint32_t guiid) {
  AO_PROVIDER *dp;
  GWEN_BUFFER *reqbuf;
  GWEN_BUFFER *rbuf=NULL;
  int rv;
  uint32_t pid;
  AB_IMEXPORTER_CONTEXT *ictx;

  assert(u);
  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
			     GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			     GWEN_GUI_PROGRESS_SHOW_LOG |
                             GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
			     GWEN_GUI_PROGRESS_SHOW_ABORT,
			     I18N("Requesting account list"),
			     I18N("We are now requesting a list of "
				  "accounts\n"
				  "which can be managed via OFX.\n"
				  "<html>"
				  "We are now requesting a list of "
				  "accounts "
				  "which can be managed via <i>OFX</i>.\n"
				  "</html>"),
			     1,
			     guiid);
  ictx=AB_ImExporterContext_new();

  reqbuf=GWEN_Buffer_new(0, 2048, 0, 1);
  GWEN_Buffer_ReserveBytes(reqbuf, 1024);

  /* add actual request */
  rv=AO_Provider__AddAccountInfoReq(pro, guiid, reqbuf);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Error adding request element (%d)", rv);
    GWEN_Buffer_free(reqbuf);
    AB_ImExporterContext_free(ictx);
    GWEN_Gui_ProgressEnd(pid);
    return rv;
  }

  /* wrap message (adds headers etc) */
  rv=AO_Provider__WrapMessage(pro, u, reqbuf, guiid);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Error adding request element (%d)", rv);
    GWEN_Buffer_free(reqbuf);
    AB_ImExporterContext_free(ictx);
    GWEN_Gui_ProgressEnd(pid);
    return rv;
  }

  /* exchange mesages */
  rv=AO_Provider_SendAndReceive(pro, u,
				(const uint8_t*)GWEN_Buffer_GetStart(reqbuf),
				GWEN_Buffer_GetUsedBytes(reqbuf),
				&rbuf,
				guiid);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Error exchanging getAccounts-request (%d)", rv);
    GWEN_Buffer_free(reqbuf);
    AB_ImExporterContext_free(ictx);
    GWEN_Gui_ProgressEnd(pid);
    return rv;
  }
  else {
    AB_IMEXPORTER *importer;
    GWEN_DB_NODE *dbProfile;

    /* parse response */
    GWEN_Buffer_free(reqbuf);
    GWEN_Gui_ProgressLog(pid,
			 GWEN_LoggerLevel_Info,
			 I18N("Parsing response"));

    /* prepare import */
    importer=AB_Banking_GetImExporter(AB_Provider_GetBanking(pro), "ofx");
    if (!importer) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
		"OFX import module not found");
      GWEN_Buffer_free(rbuf);
      AB_ImExporterContext_free(ictx);
      GWEN_Gui_ProgressEnd(pid);
      return GWEN_ERROR_NOT_FOUND;
    }

    GWEN_Buffer_Rewind(rbuf);
    dbProfile=GWEN_DB_Group_new("profile");
    /* actually import */
    rv=AB_ImExporter_ImportBuffer(importer, ictx, rbuf, dbProfile, pid);
    GWEN_DB_Group_free(dbProfile);
    GWEN_Buffer_free(rbuf);
    if (rv<0) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
		"Error importing server response (%d)", rv);
      GWEN_Gui_ProgressLog(pid,
			   GWEN_LoggerLevel_Error,
			   I18N("Error parsing response"));
      AB_ImExporterContext_free(ictx);
      GWEN_Gui_ProgressEnd(pid);
      return rv;
    }

    /* create accounts */
    rv=AO_Provider__ProcessImporterContext(pro, u, ictx, pid);
    if (rv<0) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
		"Error importing accounts (%d)", rv);
      GWEN_Gui_ProgressLog(pid,
			   GWEN_LoggerLevel_Error,
			   I18N("Error importing accounts"));
      AB_ImExporterContext_free(ictx);
      GWEN_Gui_ProgressEnd(pid);
      return rv;
    }
  }

  AB_ImExporterContext_free(ictx);
  GWEN_Gui_ProgressEnd(pid);
  return rv;
}



int AO_Provider_RequestStatements(AB_PROVIDER *pro, AB_JOB *j,
				  AB_IMEXPORTER_CONTEXT *ictx,
				  uint32_t guiid) {
  AO_PROVIDER *dp;
  GWEN_BUFFER *reqbuf;
  GWEN_BUFFER *rbuf=0;
  int rv;
  AB_USER *u;
  AB_ACCOUNT *a;
  time_t t=0;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  /* get all data for the context */
  a=AB_Job_GetAccount(j);
  assert(a);
  u=AB_Account_GetFirstUser(a);
  assert(u);

  /* get from time */
  if (AB_Job_GetType(j)==AB_Job_TypeGetTransactions) {
    const GWEN_TIME *ti;

    ti=AB_JobGetTransactions_GetFromTime(j);
    if (ti)
      t=GWEN_Time_toTime_t(ti);
  }

  reqbuf=GWEN_Buffer_new(0, 2048, 0, 1);
  GWEN_Buffer_ReserveBytes(reqbuf, 1024);

  /* add actual request */
  rv=AO_Provider__AddStatementRequest(pro, j, guiid, reqbuf);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Error adding request element (%d)", rv);
    GWEN_Buffer_free(reqbuf);
    return rv;
  }

  /* wrap message (adds headers etc) */
  rv=AO_Provider__WrapMessage(pro, u, reqbuf, guiid);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Error adding request element (%d)", rv);
    GWEN_Buffer_free(reqbuf);
    return rv;
  }

  /* exchange messages */
  rv=AO_Provider_SendAndReceive(pro, u,
				(const uint8_t*)GWEN_Buffer_GetStart(reqbuf),
				GWEN_Buffer_GetUsedBytes(reqbuf),
				&rbuf,
				guiid);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Error exchanging getStatements-request (%d)", rv);
    GWEN_Gui_ProgressLog(guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("Error parsing server response"));
    GWEN_Buffer_free(rbuf);
    GWEN_Buffer_free(reqbuf);
    return rv;
  }
  else {
    AB_IMEXPORTER *importer;
    GWEN_DB_NODE *dbProfile;

    /* parse response */
    GWEN_Buffer_free(reqbuf);
    GWEN_Gui_ProgressLog(guiid,
			 GWEN_LoggerLevel_Info,
			 I18N("Parsing response"));

    /* prepare import */
    importer=AB_Banking_GetImExporter(AB_Provider_GetBanking(pro), "ofx");
    if (!importer) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
		"OFX import module not found");
      GWEN_Buffer_free(rbuf);
      return GWEN_ERROR_NOT_FOUND;
    }

    GWEN_Buffer_Rewind(rbuf);
    dbProfile=GWEN_DB_Group_new("profile");
    /* actually import */
    rv=AB_ImExporter_ImportBuffer(importer, ictx, rbuf, dbProfile, guiid);
    GWEN_DB_Group_free(dbProfile);
    GWEN_Buffer_free(rbuf);
    if (rv<0) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
		"Error importing server response (%d)", rv);
      GWEN_Gui_ProgressLog(guiid,
			   GWEN_LoggerLevel_Error,
			   I18N("Error parsing response"));
      return rv;
    }

    /* TODO: Maybe create accounts we received here */

  }

  return 0;
}



int AO_Provider_ExecUserQueue(AB_PROVIDER *pro,
                              AB_IMEXPORTER_CONTEXT *ctx,
			      AO_USERQUEUE *uq,
			      uint32_t guiid){
  AB_JOB_LIST2_ITERATOR *jit;
  AO_PROVIDER *dp;
  int errors=0;
  int oks=0;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  jit=AB_Job_List2_First(AO_UserQueue_GetJobs(uq));
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while(uj) {
      AB_JOB_TYPE jt;

      /* TODO: omit AB_Job_TypeGetBalance if there is
       * AB_Job_TypeGetTransactions for the same account
       */
      jt=AB_Job_GetType(uj);
      if (jt==AB_Job_TypeGetBalance || jt==AB_Job_TypeGetTransactions) {
        int rv;

	/* start new context */
	rv=AO_Provider_RequestStatements(pro, uj, ctx, guiid);
	if (rv==GWEN_ERROR_USER_ABORTED) {
	  DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "User aborted");
	  AB_Job_List2Iterator_free(jit);
	  return rv;
	}
	else if (rv==GWEN_ERROR_ABORTED) {
          DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Aborted");
          break;
        }

	if (!rv)
	  oks++;
	else
	  errors++;
	rv=GWEN_Gui_ProgressAdvance(guiid,
				    AO_Provider_CountDoneJobs(dp->bankingJobs));
	if (rv==GWEN_ERROR_USER_ABORTED) {
	  DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "User aborted");
	  AB_Job_List2Iterator_free(jit);
	  return rv;
	}
      }
      else {
	DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Unhandled job type \"%s\"",
		  AB_Job_Type2Char(jt));
      }
      uj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  return 0;
}



int AO_Provider_ExecQueue(AB_PROVIDER *pro,
			  AB_IMEXPORTER_CONTEXT *ctx,
			  uint32_t guiid) {
  AO_USERQUEUE *uq;
  AO_PROVIDER *dp;
  int errors=0;
  int oks=0;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  uq=AO_Queue_FirstUserQueue(dp->queue);
  while(uq) {
    rv=AO_Provider_ExecUserQueue(pro, ctx, uq, guiid);
    if (rv)
      errors++;
    else
      oks++;
    if (rv==GWEN_ERROR_USER_ABORTED) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "User aborted");
      return rv;
    }
    uq=AO_UserQueue_List_Next(uq);
  } /* while */

  if (!oks && errors) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Not a single job succeeded");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int AO_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
                           AB_PROVIDER_EXTEND_MODE em) {
  AO_User_Extend(u, pro, em);
  return 0;
}



int AO_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
                              AB_PROVIDER_EXTEND_MODE em){
  AO_Account_Extend(a, pro, em);
  return 0;
}


#include "network.c"
#include "request.c"


