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
#include "context_l.h"

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
#include <gwenhywfar/nettransportsock.h>
#include <gwenhywfar/net.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/nettransportssl.h>
#include <gwenhywfar/netconnectionhttp.h>

#include <libofx/libofx.h>

#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_I18N
# ifdef HAVE_LOCALE_H
#  include <locale.h>
# endif
# ifdef HAVE_LIBINTL_H
#  include <libintl.h>
# endif

# define I18N(msg) dgettext(PACKAGE, msg)
#else
# define I18N(msg) msg
#endif

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
  dp->banks=AO_Bank_List_new();
  dp->bankQueues=AO_BankQueue_List_new();
  dp->libId=GWEN_Net_GetLibraryId();

  AB_Provider_SetInitFn(pro, AO_Provider_Init);
  AB_Provider_SetFiniFn(pro, AO_Provider_Fini);
  AB_Provider_SetUpdateJobFn(pro, AO_Provider_UpdateJob);
  AB_Provider_SetAddJobFn(pro, AO_Provider_AddJob);
  AB_Provider_SetExecuteFn(pro, AO_Provider_Execute);
  AB_Provider_SetResetQueueFn(pro, AO_Provider_ResetQueue);
  AB_Provider_SetGetAccountListFn(pro, AO_Provider_GetAccountList);
  AB_Provider_SetUpdateAccountFn(pro, AO_Provider_UpdateAccount);

  return pro;
}



void AO_Provider_FreeData(void *bp, void *p) {
  AO_PROVIDER *dp;

  dp=(AO_PROVIDER*)p;
  assert(dp);

  AO_BankQueue_List_free(dp->bankQueues);
  AO_Bank_List_free(dp->banks);
  AB_Job_List2_free(dp->bankingJobs);

  GWEN_FREE_OBJECT(dp);
}



int AO_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData) {
  AO_PROVIDER *dp;
#ifdef HAVE_I18N
  const char *s;
#endif
  const char *logLevelName;
  GWEN_DB_NODE *dbT;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  logLevelName=getenv("AQOFXCONNECT_LOGLEVEL");
  if (logLevelName) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(logLevelName);
    if (ll!=GWEN_LoggerLevelUnknown) {
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

#ifdef HAVE_I18N
  setlocale(LC_ALL,"");
  s=bindtextdomain(PACKAGE,  LOCALEDIR);
  if (s) {
    DBG_NOTICE(AQOFXCONNECT_LOGDOMAIN, "Locale bound.");
    bind_textdomain_codeset(PACKAGE, "UTF-8");
  }
  else {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Error binding locale");
  }
#endif

  dp->dbConfig=dbData;
  dp->lastJobId=GWEN_DB_GetIntValue(dp->dbConfig, "lastJobId", 0, 0);
  dp->connectTimeout=GWEN_DB_GetIntValue(dp->dbConfig, "connectTimeout", 0,
                                         AO_PROVIDER_CONNECT_TIMEOUT);
  dp->sendTimeout=GWEN_DB_GetIntValue(dp->dbConfig, "sendTimeout", 0,
                                      AO_PROVIDER_SEND_TIMEOUT);
  dp->recvTimeout=GWEN_DB_GetIntValue(dp->dbConfig, "recvTimeout", 0,
                                      AO_PROVIDER_RECV_TIMEOUT);

  dbT=GWEN_DB_GetGroup(dbData, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "banks");
  if (dbT) {
    dbT=GWEN_DB_FindFirstGroup(dbT, "bank");
    while(dbT) {
      AO_BANK *b;

      b=AO_Bank_fromDb(pro, dbT);
      assert(b);
      AO_Bank_List_Add(b, dp->banks);
      dbT=GWEN_DB_FindNextGroup(dbT, "bank");
    }
  }

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

  GWEN_DB_ClearGroup(dbData, "banks");
  if (AO_Bank_List_GetCount(dp->banks)) {
    AO_BANK *b;
    GWEN_DB_NODE *dbG;

    dbG=GWEN_DB_GetGroup(dbData, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                         "banks");
    assert(dbG);
    b=AO_Bank_List_First(dp->banks);
    assert(b);
    while(b) {
      GWEN_DB_NODE *dbT;

      dbT=GWEN_DB_GetGroup(dbG, GWEN_PATH_FLAGS_CREATE_GROUP, "bank");
      if (AO_Bank_toDb(b, dbT)) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                  "Could not store bank \"%s\"",
                  AO_Bank_GetBankId(b));
        abort();
      }
      b=AO_Bank_List_Next(b);
    }
  }

  dp->dbConfig=0;
  AO_BankQueue_List_Clear(dp->bankQueues);
  AB_Job_List2_Clear(dp->bankingJobs);
  AO_Bank_List_Clear(dp->banks);

  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Deinit done");

  if (errors)
    return AB_ERROR_GENERIC;

  return 0;
}



AO_BANK *AO_Provider_FindMyBank(AB_PROVIDER *pro,
                                const char *country,
                                const char *bid) {
  AO_PROVIDER *dp;
  AO_BANK *b;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  b=AO_Bank_List_First(dp->banks);
  while(b) {
    const char *s;

    s=AO_Bank_GetCountry(b);
    if (s && strcasecmp(s, country)==0) {
      s=AO_Bank_GetBankId(b);
      if (s && strcasecmp(s, bid)==0) {
        /* bank has a bank id, compare it */
        break;
      }
      else {
        AB_ACCOUNT *a;

        /* bank has no bank id or it doesn't match the requested one.
         * Scan all accounts of this bank for the given bank id */
        a=AB_Account_List_First(AO_Bank_GetAccounts(b));
        while(a) {
          s=AB_Account_GetBankCode(a);
          if (s && strcasecmp(s, bid)==0)
            /* account's bank id matches the wanted one */
            break;
          a=AB_Account_List_Next(a);
        }
        if (a)
          break;
      }
    }
    b=AO_Bank_List_Next(b);
  }

  return b;
}



AB_ACCOUNT *AO_Provider_FindMyAccount(AB_PROVIDER *pro,
                                      const char *country,
                                      const char *bankCode,
                                      const char *accountNumber) {
  AO_PROVIDER *dp;
  AB_ACCOUNT *a;
  AO_BANK *b;

  assert(bankCode);
  assert(accountNumber);
  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  if (!country || !(*country))
    country="us";

  b=AO_Provider_FindMyBank(pro, country, bankCode);
  if (!b) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Bank \"%s/%s\" not found",
             country, bankCode);
    return 0;
  }

  a=AO_Bank_FindAccount(b, accountNumber);
  if (!a) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Backend account not found");
    return 0;
  }

  return a;
}



AB_ACCOUNT *AO_Provider_FindMyAccountByAccount(AB_PROVIDER *pro,
                                               AB_ACCOUNT *ba) {
  const char *country;
  const char *bankCode;
  const char *accountId;

  country=AB_Account_GetCountry(ba);
  if (!country || !*country)
    country="us";
  bankCode=AB_Account_GetBankCode(ba);
  assert(bankCode);
  accountId=AB_Account_GetAccountNumber(ba);
  assert(accountId);
  return AO_Provider_FindMyAccount(pro, country, bankCode, accountId);
}



int AO_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j){
  AO_PROVIDER *dp;
  AB_ACCOUNT *da;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  da=AO_Provider_FindMyAccountByAccount(pro, AB_Job_GetAccount(j));
  if (!da) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Account not managed by this backend");
    return AB_ERROR_INVALID;
  }

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
    return AB_ERROR_NOT_SUPPORTED;
  } /* switch */
}



AO_BANKQUEUE *AO_Provider_FindBankQueue(AB_PROVIDER *pro,
                                        const char *country,
                                        const char *bankId) {
  AO_PROVIDER *dp;
  AO_BANK *b;
  AO_BANKQUEUE *bq;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  b=AO_Provider_FindMyBank(pro, country, bankId);
  if (!b)
    return 0;
  bq=AO_BankQueue_List_First(dp->bankQueues);
  while(bq) {
    if (AO_BankQueue_GetBank(bq)==b)
      break;
    bq=AO_BankQueue_List_Next(bq);
  }

  return bq;
}



int AO_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j){
  AO_PROVIDER *dp;
  AB_ACCOUNT *da;
  const char *s;
  AO_BANKQUEUE *bq;
  AO_USERQUEUE *uq;
  int doAdd=1;
  GWEN_DB_NODE *dbJob;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  da=AO_Provider_FindMyAccountByAccount(pro, AB_Job_GetAccount(j));
  if (!da) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Account not managed by this backend");
    return AB_ERROR_INVALID;
  }

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
    return AB_ERROR_NOT_SUPPORTED;
  } /* switch */

  bq=AO_Provider_FindBankQueue(pro,
                               AB_Account_GetCountry(da),
                               AB_Account_GetBankCode(da));
  if (!bq) {
    AO_BANK *b;

    b=AO_Provider_FindMyBank(pro,
                             AB_Account_GetCountry(da),
                             AB_Account_GetBankCode(da));
    if (!b) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Bank not found");
      return AB_ERROR_INVALID;
    }
    bq=AO_BankQueue_new(b);
    AO_BankQueue_List_Add(bq, dp->bankQueues);
  }

  s=AO_Account_GetUserId(da);
  assert(s);
  uq=AO_BankQueue_FindUserQueue(bq, s);
  if (!uq) {
    AO_USER *u;
    AO_BANK *b;

    b=AO_BankQueue_GetBank(bq);
    assert(b);
    u=AO_Bank_FindUser(b, s);
    if (!u) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "User \"%s\" not found", s);
      return AB_ERROR_INVALID;
    }
    uq=AO_UserQueue_new(u);
    AO_BankQueue_AddUserQueue(bq, uq);
  }

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

  AO_BankQueue_List_Clear(dp->bankQueues);
  AB_Job_List2_Clear(dp->bankingJobs);

  return 0;
}



AB_ACCOUNT_LIST2 *AO_Provider_GetAccountList(AB_PROVIDER *pro){
  AO_PROVIDER *dp;
  AB_ACCOUNT_LIST2 *nl;
  AO_BANK *b;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  if (AO_Bank_List_GetCount(dp->banks)==0)
    return 0;

  nl=AB_Account_List2_new();
  b=AO_Bank_List_First(dp->banks);
  while(b) {
    AB_ACCOUNT *a;

    a=AB_Account_List_First(AO_Bank_GetAccounts(b));
    while(a) {
      AB_ACCOUNT *na;

      na=AB_Account_dup(a);
      AB_Account_List2_PushBack(nl, na);
      a=AB_Account_List_Next(a);
    }
    b=AO_Bank_List_Next(b);
  }

  if (AB_Account_List2_GetSize(nl)==0) {
    AB_Account_List2_free(nl);
    return 0;
  }

  return nl;
}



int AO_Provider_UpdateAccount(AB_PROVIDER *pro, AB_ACCOUNT *a){
  AO_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "TODO: UpdateAccount");
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



AB_JOB *AO_Provider_FindJobById(AB_JOB_LIST2 *jl, GWEN_TYPE_UINT32 jid) {
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



int AO_Provider_Execute(AB_PROVIDER *pro){
  AO_PROVIDER *dp;
  AO_BANKQUEUE *bq;
  int oks=0;
  int errors=0;
  AB_JOB_LIST2_ITERATOR *jit;
  GWEN_TYPE_UINT32 pid;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  pid=AB_Banking_ProgressStart(AB_Provider_GetBanking(pro),
			       I18N("Sending Requests"),
			       I18N("We are now sending all requests "
				    "to the banks.\n"
				    "<html>"
				    "We are now sending all requests "
				    "to the banks.\n"
				    "</html>"),
			       AB_Job_List2_GetSize(dp->bankingJobs));

  bq=AO_BankQueue_List_First(dp->bankQueues);
  while(bq) {
    int rv;

    rv=AO_Provider_ExecBankQueue(pro, bq);
    if (!rv)
      oks++;
    else
      errors++;
    if (rv==AB_ERROR_USER_ABORT) {
      AO_BankQueue_List_Clear(dp->bankQueues);
      AB_Job_List2_Clear(dp->bankingJobs);
      AB_Banking_ProgressEnd(AB_Provider_GetBanking(pro), pid);
      return rv;
    }

    bq=AO_BankQueue_List_Next(bq);
  }

  jit=AB_Job_List2_First(dp->bankingJobs);
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while(uj) {
      if (AB_Job_GetStatus(uj)==AB_Job_StatusSent) {
	AB_JOB *rj;
	GWEN_TYPE_UINT32 rjid;

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
	  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
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

  AB_Banking_ProgressAdvance(AB_Provider_GetBanking(pro),
			     0,
			     AO_Provider_CountDoneJobs(dp->bankingJobs));

  AO_BankQueue_List_Clear(dp->bankQueues);
  AB_Job_List2_Clear(dp->bankingJobs);
  AB_Banking_ProgressEnd(AB_Provider_GetBanking(pro), pid);

  if (!oks && errors) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Not a single job succeeded");
    return AB_ERROR_GENERIC;
  }

  return 0;
}



int AO_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a){
  AO_PROVIDER *dp;
  AO_BANK *b;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  b=AO_Provider_FindMyBank(pro,
                           AB_Account_GetCountry(a),
                           AB_Account_GetBankCode(a));
  if(!b) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Bank \"%s/%s\" not found",
              AB_Account_GetCountry(a),
              AB_Account_GetBankCode(a));
    return -1;
  }
  AO_Bank_AddAccount(b, a);
  return 0;
}



int AO_Provider_RemoveAccount(AB_PROVIDER *pro, AB_ACCOUNT *a){
  AO_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  AB_Account_List_Del(a);
  return 0;
}



int AO_Provider_HasAccount(AB_PROVIDER *pro,
                           const char *country,
                           const char *bankCode,
                           const char *accountNumber){
  AO_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  return (AO_Provider_FindMyAccount(pro, country, bankCode, accountNumber)!=
          0);
}




int AO_Provider_EncodeJob(AB_PROVIDER *pro,
                          AO_CONTEXT *ctx,
                          char **pData) {
  AB_JOB *j = 0;
  char *res=0;

  assert(pro);
  assert(ctx);
  j=AO_Context_GetJob(ctx);
  assert(j);
  switch(AB_Job_GetType(j)) {
  case AB_Job_TypeGetBalance:
    res=libofx_request_statement(AO_Context_GetFi(ctx),
                                 AO_Context_GetAi(ctx),
                                 0);
    break;

  case AB_Job_TypeGetTransactions: {
    const GWEN_TIME *ti;
    time_t secs=0;

    ti=AB_JobGetTransactions_GetFromTime(j);
    if (ti)
      secs=GWEN_Time_toTime_t(ti);
    res=libofx_request_statement(AO_Context_GetFi(ctx),
                                 AO_Context_GetAi(ctx),
                                 secs);
    break;
  }

  default:
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Unsupported job type (%d)",
              AB_Job_GetType(j));
    return AB_ERROR_INVALID;
  }

  if (res==0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "Could not create request for job");
    return AB_ERROR_GENERIC;
  }
  *pData=res;
  return 0;
}



int AO_Provider_ExtractHttpResponse(AB_PROVIDER *pro,
                                    GWEN_NETMSG *netmsg,
                                    GWEN_BUFFER *dbuf) {
  GWEN_DB_NODE *dbResponse;
  GWEN_BUFFER *mbuf;
  int pos;
  const char *s;
  int i;
  int status;
  char errBuf[256];
  char numbuf[16];
  const char *p;
  int isErr;
  AO_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  /* check HTTP response */
  dbResponse=GWEN_NetMsg_GetDB(netmsg);
  assert(dbResponse);

  /* client code */
  status=GWEN_DB_GetIntValue(dbResponse, "status/code", 0, -1);
  if (status==-1) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "No status, bad HTTP response, assuming HTTP 0.9 response");
    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                           0,
                           AB_Banking_LogLevelError,
                           I18N("No status, bad HTTP response, "
                                "assuming HTTP 0.9 response"));
    status=200;
  }
  snprintf(numbuf, sizeof(numbuf), "%d", status);
  DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "HTTP-Status: %d", status);

  isErr=(status<200 || status>299);

  p=GWEN_DB_GetCharValue(dbResponse, "status/text", 0, "");
  errBuf[0]=0;
  errBuf[sizeof(errBuf)-1]=0;
  if (isErr)
    snprintf(errBuf, sizeof(errBuf)-1,
             I18N("HTTP-Error: %d %s"),
             status, p);
  else
    snprintf(errBuf, sizeof(errBuf)-1,
             I18N("HTTP-Status: %d %s"),
             status, p);
  DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "%s", errBuf);

  if (isErr)
    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                           0,
                           AB_Banking_LogLevelError,
                           errBuf);
  else
    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                           0,
                           AB_Banking_LogLevelInfo,
                           errBuf);
  if (isErr) {
    FILE *f;

    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Saving response in \"/tmp/ofx_error_response.html\" ...");
    mbuf=GWEN_NetMsg_GetBuffer(netmsg);
    assert(mbuf);
    f=fopen("/tmp/ofx_error_response.html", "w+");
    if (!f) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fopen: %s", strerror(errno));
    }
    else {
      if (fwrite(GWEN_Buffer_GetStart(mbuf),
                 GWEN_Buffer_GetUsedBytes(mbuf),
                 1,
                 f)!=1) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fwrite: %s", strerror(errno));
      }
      if (fclose(f)) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fclose: %s", strerror(errno));
      }
    }
    return AB_ERROR_GENERIC;
  }

  /* status is ok or not needed, decode message */
  mbuf=GWEN_NetMsg_GetBuffer(netmsg);
  assert(mbuf);
#ifdef AO_PROVIDER_HEAVY_DEBUG
  DBG_NOTICE(AQOFXCONNECT_LOGDOMAIN, "Got this response: ");
  GWEN_Buffer_Dump(mbuf, stderr, 2);
#endif
  pos=GWEN_Buffer_GetBookmark(mbuf, 1);
  if (pos>=GWEN_Buffer_GetUsedBytes(mbuf)) {
    return AB_ERROR_GENERIC;
  }

  /* check whether the base64 stuff directly follows */
  p=GWEN_DB_GetCharValue(dbResponse, "header/Transfer-Encoding", 0, 0);
  if (p && strcasecmp(p, "chunked")==0) {
    const unsigned char *d;

    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Got chunked data");
    d=GWEN_Buffer_GetStart(mbuf)+pos;
    while(*d) {
      GWEN_TYPE_UINT32 len=0;
      GWEN_TYPE_UINT32 cpos;

      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                "Starting here: %d (%x)",
                d-(unsigned char*)(GWEN_Buffer_GetStart(mbuf)),
                d-(unsigned char*)(GWEN_Buffer_GetStart(mbuf)));

      /* get start of chunk size */
      while(*d && !isxdigit(*d))
        d++;
      if (*d==0)
        break;

      /* read chunk size */
      while(*d && isxdigit(*d)) {
        unsigned char c;

        c=toupper(*d)-'0';
        if (c>9)
          c-=7;
        len=(len<<4)+c;
        d++;
      }
      if (len==0)
        /* chunk size 0, end */
        break;
      /* read until rest of line */
      while (*d && *d !=10 && *d !=13)
        d++;
      if (*d==10 || *d==13)
        d++;
      if (*d==10 || *d==13)
        d++;
      cpos=d-(unsigned char*)(GWEN_Buffer_GetStart(mbuf));
      if (cpos+len>GWEN_Buffer_GetUsedBytes(mbuf)) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                  "Bad chunk size \"%d\" (only %d bytes left)",
                  len, GWEN_Buffer_GetUsedBytes(mbuf)-cpos);
        return AB_ERROR_BAD_DATA;
      }
      DBG_VERBOUS(AQOFXCONNECT_LOGDOMAIN, "Chunksize is %d (%x):", len, len);
      if (GWEN_Logger_GetLevel(AQOFXCONNECT_LOGDOMAIN)>=GWEN_LoggerLevelVerbous)
        GWEN_Text_DumpString(d, len, stderr, 4);
      GWEN_Buffer_AppendBytes(dbuf, d, len);
      d+=len;
      /* skip trailing CR/LF */
      if (*d==10 || *d==13)
        d++;
      if (*d==10 || *d==13)
        d++;
    } /* while */
  } /* if chunked */
  else {
    GWEN_Buffer_AppendBytes(dbuf,
                            GWEN_Buffer_GetStart(mbuf)+pos,
                            GWEN_Buffer_GetUsedBytes(mbuf)-pos);
  }

  /* cut off trailing zeros */
  s=GWEN_Buffer_GetStart(dbuf);
  i=GWEN_Buffer_GetUsedBytes(dbuf);
  while((--i)>0) {
    if (s[i]!=0)
      break;
  }
  if (i)
    GWEN_Buffer_Crop(dbuf, 0, i+1);
  else {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Empty message received");
    return AB_ERROR_BAD_DATA;
  }

  return 0;
}


void AO_Provider_AddBankCertFolder(AB_PROVIDER *pro,
                                   const AO_BANK *b,
                                   GWEN_BUFFER *nbuf) {
  const char *s;

  AB_Provider_GetUserDataDir(pro, nbuf);
  GWEN_Buffer_AppendString(nbuf, "/banks/");
  s=AO_Bank_GetCountry(b);
  assert(s);
  GWEN_Buffer_AppendString(nbuf, s);
  GWEN_Buffer_AppendByte(nbuf, '/');
  s=AO_Bank_GetBankId(b);
  assert(s);
  GWEN_Buffer_AppendString(nbuf, s);
  GWEN_Buffer_AppendByte(nbuf, '/');
  GWEN_Buffer_AppendString(nbuf, "/certs");
}




GWEN_NETCONNECTION *AO_Provider_CreateConnection(AB_PROVIDER *pro,
                                                 AO_USER *u) {
  AO_PROVIDER *dp;
  GWEN_NETTRANSPORT *tr;
  GWEN_NETCONNECTION *conn;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  AO_BANK *b;
  const char *bankAddr;
  int bankPort;
  AO_BANK_SERVERTYPE addrType;
  GWEN_ERRORCODE err;
  char *p;
  GWEN_BUFFER *nbuf;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  b=AO_User_GetBank(u);
  assert(b);

  /* create socket */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);

  /* create transport layer */
  addrType=AO_Bank_GetServerType(b);
  switch(addrType) {
  case AO_Bank_ServerTypeHTTP:
    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                           AB_Banking_LogLevelNotice,
                           I18N("Creating HTTP connection"));
    tr=GWEN_NetTransportSocket_new(sk, 1);
    break;

  case AO_Bank_ServerTypeHTTPS:
    nbuf=GWEN_Buffer_new(0, 64, 0, 1);
    AO_Provider_AddBankCertFolder(pro, b, nbuf);

    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                           AB_Banking_LogLevelNotice,
                           I18N("Creating HTTPS connection"));
    tr=GWEN_NetTransportSSL_new(sk,
                                GWEN_Buffer_GetStart(nbuf),
                                GWEN_Buffer_GetStart(nbuf),
                                0, 0, 0, 1);
    GWEN_Buffer_free(nbuf);
    break;

  default:
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Unknown server type %d", addrType);
    return 0;
  } /* switch */

  /* get address */
  bankAddr=AO_Bank_GetServerAddr(b);
  if (!bankAddr) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "User has no valid address settings");
    GWEN_NetTransport_free(tr);
    return 0;
  }

  /* get port */
  bankPort=AO_Bank_GetServerPort(b);
  if (bankPort<1) {
    /* set default port if none given */
    switch(addrType) {
    case AO_Bank_ServerTypeHTTP:  bankPort=80; break;
    case AO_Bank_ServerTypeHTTPS: bankPort=443; break;
    default:
      DBG_WARN(AQOFXCONNECT_LOGDOMAIN,
               "Unknown address type (%d), assuming HTTPS",
               addrType);
      bankPort=443;
      break;
    } /* switch */
  }

  /* extract address and local path from URL */
  nbuf=GWEN_Buffer_new(0, 64, 0, 1);
  GWEN_Buffer_AppendString(nbuf, bankAddr);
  p=strchr(GWEN_Buffer_GetStart(nbuf), '/');
  if (p)
    *p=0;
  if (AO_Bank_GetHttpHost(b)==0)
    /* set HTTP host if it not already is */
    AO_Bank_SetHttpHost(b, GWEN_Buffer_GetStart(nbuf));
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  err=GWEN_InetAddr_SetAddress(addr, GWEN_Buffer_GetStart(nbuf));
  /* resolve address */
  if (!GWEN_Error_IsOk(err)) {
    char dbgbuf[256];

    snprintf(dbgbuf, sizeof(dbgbuf)-1,
             I18N("Resolving hostname \"%s\" ..."),
             GWEN_Buffer_GetStart(nbuf));
    dbgbuf[sizeof(dbgbuf)-1]=0;
    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                           AB_Banking_LogLevelNotice,
                           dbgbuf);
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Resolving hostname \"%s\"",
             GWEN_Buffer_GetStart(nbuf));
    err=GWEN_InetAddr_SetName(addr, GWEN_Buffer_GetStart(nbuf));
    if (!GWEN_Error_IsOk(err)) {
      snprintf(dbgbuf, sizeof(dbgbuf)-1,
               I18N("Unknown hostname \"%s\""),
               GWEN_Buffer_GetStart(nbuf));
      dbgbuf[sizeof(dbgbuf)-1]=0;
      AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                             AB_Banking_LogLevelError,
                             dbgbuf);
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
                "Error resolving hostname \"%s\":",
                GWEN_Buffer_GetStart(nbuf));
      DBG_ERROR_ERR(AQOFXCONNECT_LOGDOMAIN, err);
      GWEN_Buffer_free(nbuf);
      GWEN_NetTransport_free(tr);
      return 0;
    }
    else {
      char addrBuf[256];
      GWEN_ERRORCODE err;

      err=GWEN_InetAddr_GetAddress(addr, addrBuf, sizeof(addrBuf)-1);
      addrBuf[sizeof(addrBuf)-1]=0;
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(AQOFXCONNECT_LOGDOMAIN, err);
      }
      else {
        snprintf(dbgbuf, sizeof(dbgbuf)-1,
                 I18N("IP address is %s"),
                 addrBuf);
        dbgbuf[sizeof(dbgbuf)-1]=0;
        AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                               AB_Banking_LogLevelNotice,
                               dbgbuf);
      }
    }
  }

  /* complete address, set address in transport layer */
  GWEN_InetAddr_SetPort(addr, bankPort);
  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Port is: %d", bankPort);
  GWEN_NetTransport_SetPeerAddr(tr, addr);
  GWEN_InetAddr_free(addr);

  /* create connection on top of it */
  conn=GWEN_NetConnectionHTTP_new(tr, 1,
                                  dp->libId,
                                  AO_Bank_GetHttpVMajor(b),
                                  AO_Bank_GetHttpVMinor(b));
  assert(conn);

  /* set default URL */
  GWEN_NetConnectionHTTP_SetDefaultURL(conn, "/");
  if (p)
    *p='/';
  if (p)
    if (*p)
      GWEN_NetConnectionHTTP_SetDefaultURL(conn, p);
  GWEN_Buffer_free(nbuf);

  /* resturn connection */
  return conn;
}



int AO_Provider_SendMessage(AB_PROVIDER *pro,
                            AO_USER *u,
                            GWEN_NETCONNECTION *conn,
                            const char *p,
                            unsigned int plen) {
  GWEN_BUFFER *nbuf;
  GWEN_DB_NODE *dbRequest;
  char numbuf[16];
  GWEN_NETTRANSPORT_STATUS nst;
  AO_BANK *b;
  const char *s;

  assert(u);
  b=AO_User_GetBank(u);
  assert(b);

  nbuf=GWEN_Buffer_new(0, plen, 0, 1);
  GWEN_Buffer_AppendBytes(nbuf, p, plen);

  dbRequest=GWEN_DB_Group_new("request");
  s=AO_Bank_GetHttpHost(b);
  if (s)
    GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "header/Host", s);
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "header/Pragma", "no-cache");
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "header/Cache-control", "no cache");

  snprintf(numbuf, sizeof(numbuf), "%d", GWEN_Buffer_GetUsedBytes(nbuf));
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "header/Content-type",
                       "application/x-ofx");
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "header/Accept",
                       "*/*, application/x-ofx");
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "header/Content-length",
                       numbuf);
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "header/Connection",
                       "close");
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "command/cmd", "POST");

  /* catch status changes */
  GWEN_NetConnection_WorkIO(conn);

  nst=GWEN_NetConnection_GetStatus(conn);
  if (nst==GWEN_NetTransportStatusPDisconnected) {
    /* connection down */
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "Connection is down");
    GWEN_Buffer_free(nbuf);
    return AB_ERROR_NETWORK;
  }

  if (GWEN_NetConnectionHTTP_AddRequest(conn,
                                        dbRequest,
                                        nbuf,
                                        0)) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not enqueue HTTP request");
    GWEN_DB_Group_free(dbRequest);
    return AB_ERROR_NETWORK;
  }

  GWEN_DB_Group_free(dbRequest);
  DBG_DEBUG(AQOFXCONNECT_LOGDOMAIN, "Message enqueued");
  return 0;
}



int AO_Provider_SendAndReceive(AB_PROVIDER *pro,
                               AO_USER *u,
                               const char *p,
                               unsigned int plen,
                               GWEN_BUFFER *rbuf) {
  AO_PROVIDER *dp;
  GWEN_NETCONNECTION *conn;
  GWEN_NETMSG *netmsg;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  if (1) {
    FILE *f;

    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Saving response in \"/tmp/ofx.log\" ...");
    f=fopen("/tmp/ofx.log", "a+");
    if (!f) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fopen: %s", strerror(errno));
    }
    else {
      fprintf(f, "\n\nSending:\n");
      fprintf(f, "-------------------------------------\n");
      if (fwrite(p,
		 plen,
                 1,
                 f)!=1) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fwrite: %s", strerror(errno));
      }
      if (fclose(f)) {
	DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fclose: %s", strerror(errno));
      }
    }
  }

  /* setup connection */
  conn=AO_Provider_CreateConnection(pro, u);
  if (!conn) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Could not create connection");
    return AB_ERROR_GENERIC;
  }

  /* connect */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                         0,
                         AB_Banking_LogLevelInfo,
                         I18N("Connecting..."));
  if (GWEN_NetConnection_Connect_Wait(conn, dp->connectTimeout)) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not start to connect to bank");
    GWEN_NetConnection_free(conn);
    return AB_ERROR_NETWORK;
  }

  /* send message */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                         0,
                         AB_Banking_LogLevelInfo,
                         I18N("Sending request..."));
  rv=AO_Provider_SendMessage(pro, u, conn, p, plen);
  if (rv) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "Error %d", rv);
    GWEN_NetConnection_StartDisconnect(conn);
    GWEN_NetConnection_free(conn);
    return rv;
  }

  /* flush connection buffers (make sure data is sent NOW */
  rv=GWEN_NetConnection_Flush(conn, dp->sendTimeout);
  if (rv) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "Error %d", rv);
    GWEN_NetConnection_StartDisconnect(conn);
    GWEN_NetConnection_free(conn);
    return rv;
  }

  /* wait for response */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                         0,
                         AB_Banking_LogLevelInfo,
                         I18N("Waiting for response..."));
  netmsg=GWEN_NetConnection_GetInMsg_Wait(conn, dp->recvTimeout);
  if (!netmsg) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "No response received");
    GWEN_NetConnection_StartDisconnect(conn);
    GWEN_NetConnection_free(conn);
    return AB_ERROR_NETWORK;
  }

  /* found a response, transform it */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                         0,
                         AB_Banking_LogLevelInfo,
                         I18N("Parsing response..."));
  rv=AO_Provider_ExtractHttpResponse(pro, netmsg, rbuf);
  if (rv) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Bad message received");
    GWEN_NetMsg_free(netmsg);
    GWEN_NetConnection_StartDisconnect(conn);
    GWEN_NetConnection_free(conn);
    return rv;
  }
  GWEN_NetMsg_free(netmsg);

  /* disconnect */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                         0,
                         AB_Banking_LogLevelInfo,
                         I18N("Disconnecting..."));
  if (GWEN_NetConnection_StartDisconnect(conn)) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "Could not start to disconnect connection");
  }

  /* release connection ressources */
  GWEN_NetConnection_free(conn);

  if (1) {
    FILE *f;

    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Saving response in \"/tmp/ofx.log\" ...");
    f=fopen("/tmp/ofx.log", "a+");
    if (!f) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fopen: %s", strerror(errno));
    }
    else {
      fprintf(f, "\n\nReceived:\n");
      fprintf(f, "-------------------------------------\n");
      if (fwrite(GWEN_Buffer_GetStart(rbuf),
		 GWEN_Buffer_GetUsedBytes(rbuf),
		 1,
		 f)!=1) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fwrite: %s", strerror(errno));
      }
      if (fclose(f)) {
	DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fclose: %s", strerror(errno));
      }
    }
  }

  return 0;
}



int AO_Provider_RequestAccounts(AB_PROVIDER *pro,
                                const char *country,
                                const char *bankId,
                                const char *userId) {
  AO_PROVIDER *dp;
  AO_CONTEXT *ctx;
  char *msg;
  GWEN_BUFFER *rbuf;
  int rv;
  AO_BANK *b;
  AO_USER *u;
  GWEN_TYPE_UINT32 pid;
  AB_IMEXPORTER_CONTEXT *ictx;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  b=AO_Provider_FindMyBank(pro, country, bankId);
  if (!b) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Bank \"%s\" not found", bankId);
    return AB_ERROR_INVALID;
  }
  u=AO_Bank_FindUser(b, userId);
  if (!u) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "User \"%s\" not found", userId);
    return AB_ERROR_INVALID;
  }

  pid=AB_Banking_ProgressStart(AB_Provider_GetBanking(pro),
                               I18N("Requesting account list"),
                               I18N("We are now requesting a list of "
                                    "accounts\n"
                                    "which can be managed via OFX.\n"
                                    "<html>"
                                    "We are now requesting a list of "
                                    "accounts "
                                    "which can be managed via <i>OFX</i>.\n"
                                    "</html>"),
			       1);
  ictx=AB_ImExporterContext_new();
  ctx=AO_Context_new(AO_User_GetBank(u), u, 0, ictx);
  assert(ctx);
  rv=AO_Context_Update(ctx);
  if (rv) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Error updating context");
    AO_Context_free(ctx);
    AB_Banking_ProgressEnd(AB_Provider_GetBanking(pro), pid);
    return rv;
  }

  msg=libofx_request_accountinfo(AO_Context_GetFi(ctx));
  if (!msg) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Could not generate getAccounts-request");
    AO_Context_free(ctx);
    AB_ImExporterContext_free(ictx);
    AB_Banking_ProgressEnd(AB_Provider_GetBanking(pro), pid);
    return AB_ERROR_GENERIC;
  }

  //fprintf(stderr, "Sending this: %s\n", msg);
  //AO_Context_free(ctx);
  //AB_Banking_ProgressEnd(AB_Provider_GetBanking(pro), pid);
  //return 0;

  rbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=AO_Provider_SendAndReceive(pro, u, msg, strlen(msg), rbuf);
  if (rv) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Error exchanging getAccounts-request (%d)", rv);
    GWEN_Buffer_free(rbuf);
    AO_Context_free(ctx);
    AB_ImExporterContext_free(ictx);
    AB_Banking_ProgressEnd(AB_Provider_GetBanking(pro), pid);
    return rv;
  }

  /* parse response */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                         0,
                         AB_Banking_LogLevelInfo,
                         I18N("Parsing response"));

  rv=libofx_proc_buffer(AO_Context_GetOfxContext(ctx),
                        GWEN_Buffer_GetStart(rbuf),
                        GWEN_Buffer_GetUsedBytes(rbuf));
  if (rv) {
    DBG_ERROR(0, "Error parsing data: %d", rv);
    rv=AB_ERROR_BAD_DATA;
  }
  GWEN_Buffer_free(rbuf);

  if (!rv) {
    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                           0,
                           AB_Banking_LogLevelInfo,
                           I18N("Processing response"));
    rv=AO_Context_ProcessImporterContext(ctx);
    if (rv) {
      DBG_ERROR(0, "Error pprocessing data: %d", rv);
      rv=AB_ERROR_BAD_DATA;
    }
  }

  AO_Context_free(ctx);
  AB_ImExporterContext_free(ictx);
  AB_Banking_ProgressEnd(AB_Provider_GetBanking(pro), pid);
  return rv;
}



int AO_Provider_RequestStatements(AB_PROVIDER *pro, AB_JOB *j,
				  AB_IMEXPORTER_CONTEXT *ictx) {
  AO_PROVIDER *dp;
  AO_CONTEXT *ctx;
  char *msg;
  GWEN_BUFFER *rbuf;
  int rv;
  AO_BANK *b;
  AO_USER *u;
  AB_ACCOUNT *ba;
  AB_ACCOUNT *a;
  const char *country;
  const char *bankId;
  const char *accountId;
  const char *userId;
  time_t t=0;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  /* get all data for the context */
  ba=AB_Job_GetAccount(j);
  assert(ba);
  country=AB_Account_GetCountry(ba);
  bankId=AB_Account_GetBankCode(ba);
  accountId=AB_Account_GetAccountNumber(ba);
  b=AO_Provider_FindMyBank(pro, country, bankId);
  if (!b) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Bank \"%s\" not found", bankId);
    return AB_ERROR_INVALID;
  }
  a=AO_Bank_FindAccount(b, accountId);
  if (!a) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Account \"%s/%s\" not found", bankId, accountId);
    return AB_ERROR_INVALID;
  }
  userId=AO_Account_GetUserId(a);
  if (!userId || !*userId) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "No user id in account \"%s/%s\"", bankId, accountId);
    return AB_ERROR_INVALID;
  }
  u=AO_Bank_FindUser(b, userId);
  if (!u) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "User \"%s\" not found", userId);
    return AB_ERROR_INVALID;
  }

  /* get from time */
  if (AB_Job_GetType(j)==AB_Job_TypeGetTransactions) {
    const GWEN_TIME *ti;

    ti=AB_JobGetTransactions_GetFromTime(j);
    if (ti)
      t=GWEN_Time_toTime_t(ti);
  }

  /* create and setup context */
  ctx=AO_Context_new(b, u, j, ictx);
  assert(ctx);
  rv=AO_Context_Update(ctx);
  if (rv) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Error updating context");
    AO_Context_free(ctx);
    return rv;
  }

  /* create request data */
  msg=libofx_request_statement(AO_Context_GetFi(ctx),
                               AO_Context_GetAi(ctx),
                               t);
  if (!msg) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Could not generate getStatements-request");
    AO_Context_free(ctx);
    return AB_ERROR_GENERIC;
  }

  //fprintf(stderr, "Sending this: %s\n", msg);
  //AO_Context_free(ctx);
  //AB_Banking_ProgressEnd(AB_Provider_GetBanking(pro), pid);
  //return 0;

  rbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=AO_Provider_SendAndReceive(pro, u, msg, strlen(msg), rbuf);
  if (rv) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Error exchanging getStatements-request (%d)", rv);
    GWEN_Buffer_free(rbuf);
    AO_Context_free(ctx);
    return rv;
  }

  /* parse response */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                         0,
                         AB_Banking_LogLevelInfo,
                         I18N("Parsing response"));

  rv=libofx_proc_buffer(AO_Context_GetOfxContext(ctx),
                        GWEN_Buffer_GetStart(rbuf),
                        GWEN_Buffer_GetUsedBytes(rbuf));
  if (rv) {
    DBG_ERROR(0, "Error parsing data: %d", rv);
    rv=AB_ERROR_BAD_DATA;
  }
  GWEN_Buffer_free(rbuf);

  if (!rv) {
    if (AO_Context_GetAbort(ctx))
      rv=AB_ERROR_ABORTED;
  }

  if (!rv) {
    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                           0,
                           AB_Banking_LogLevelInfo,
                           I18N("Processing response"));
    rv=AO_Context_ProcessImporterContext(ctx);
    if (rv) {
      DBG_ERROR(0, "Error pprocessing data: %d", rv);
      rv=AB_ERROR_BAD_DATA;
    }
  }

  AO_Context_free(ctx);
  return rv;
}



int AO_Provider_DistributeContext(AB_PROVIDER *pro,
				  AB_JOB *refJob,
				  AB_IMEXPORTER_CONTEXT *ictx) {
  AB_JOB_LIST2_ITERATOR *jit;
  AO_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  jit=AB_Job_List2_First(dp->bankingJobs);
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while(uj) {
      DBG_ERROR(0, "Checking job \"%s\"",
		AB_Job_Type2Char(AB_Job_GetType(uj)));
      if (AB_Job_GetAccount(refJob)==AB_Job_GetAccount(uj)) {
	AB_JOB_TYPE jt;

	jt=AB_Job_GetType(uj);
	if (jt==AB_Job_TypeGetBalance) {
	  AB_IMEXPORTER_ACCOUNTINFO *ai;

	  AB_Job_SetStatus(uj, AB_Job_StatusFinished);
	  ai=AB_ImExporterContext_GetFirstAccountInfo(ictx);
	  while(ai) {
	    AB_ACCOUNT_STATUS *ast;

	    /* last account info received wins */
	    ast=AB_ImExporterAccountInfo_GetFirstAccountStatus(ai);
	    while(ast) {
	      /* last received wins */
	      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
			"Saving account status to job %08x",
			AB_Job_GetJobId(uj));
	      AB_JobGetBalance_SetAccountStatus(uj, ast);
	      ast=AB_ImExporterAccountInfo_GetNextAccountStatus(ai);
	    }
	    ai=AB_ImExporterContext_GetNextAccountInfo(ictx);
	  }
	}
	else if (jt==AB_Job_TypeGetTransactions) {
	  AB_IMEXPORTER_ACCOUNTINFO *ai;
	  AB_TRANSACTION_LIST2 *tl;

	  tl=AB_Transaction_List2_new();
	  ai=AB_ImExporterContext_GetFirstAccountInfo(ictx);
	  while(ai) {
	    const AB_TRANSACTION *t;

	    t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
	    AB_Job_SetStatus(uj, AB_Job_StatusFinished);
	    while(t) {
	      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
			"Adding transaction to job %08x",
			AB_Job_GetJobId(uj));
              AB_Transaction_List2_PushBack(tl, AB_Transaction_dup(t));
	      t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
	    } /* while t */
	    ai=AB_ImExporterContext_GetNextAccountInfo(ictx);
	  } /* while ai */
	  if (AB_Transaction_List2_GetSize(tl)==0)
	    AB_Transaction_List2_free(tl);
	  else
	    AB_JobGetTransactions_SetTransactions(uj, tl);
	} /* if JobGetTransactions */
      } /* if same account */
      else {
        DBG_ERROR(0, "Account does not match");
      }
      uj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  return 0;
}



int AO_Provider_ExecUserQueue(AB_PROVIDER *pro, AO_USERQUEUE *uq){
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

      jt=AB_Job_GetType(uj);
      if (jt==AB_Job_TypeGetBalance || jt==AB_Job_TypeGetTransactions) {
	AB_IMEXPORTER_CONTEXT *ictx;
	int rv;

	/* start new context */
	ictx=AB_ImExporterContext_new();

	rv=AO_Provider_RequestStatements(pro, uj, ictx);
	if (rv==AB_ERROR_USER_ABORT) {
	  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "User aborted");
	  AB_Job_List2Iterator_free(jit);
	  return rv;
	}
	else if (rv==AB_ERROR_ABORTED) {
	  AB_ImExporterContext_free(ictx);
	  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Aborted");
	  break;
	}
	if (!rv && AB_Job_GetStatus(uj)!=AB_Job_StatusError) {
	  /* Store data from context to all matching jobs */
	  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Distributing context");
	  rv=AO_Provider_DistributeContext(pro, uj, ictx);
	}
	else {
	  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
		    "Not distributing context (%d)", rv);
	}

	AB_ImExporterContext_free(ictx);
	if (!rv)
	  oks++;
	else
	  errors++;
	rv=AB_Banking_ProgressAdvance(AB_Provider_GetBanking(pro),
				      0,
				      AO_Provider_CountDoneJobs(dp->bankingJobs));
	if (rv==AB_ERROR_USER_ABORT) {
	  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "User aborted");
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

  if (!oks && errors) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Not a single job succeeded");
    return AB_ERROR_GENERIC;
  }

  return 0;
}



int AO_Provider_ExecBankQueue(AB_PROVIDER *pro, AO_BANKQUEUE *bq) {
  AO_USERQUEUE *uq;
  AO_PROVIDER *dp;
  int errors=0;
  int oks=0;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  uq=AO_UserQueue_List_First(AO_BankQueue_GetUserQueues(bq));
  while(uq) {
    rv=AO_Provider_ExecUserQueue(pro, uq);
    if (rv)
      errors++;
    else
      oks++;
    if (rv==AB_ERROR_USER_ABORT) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "User aborted");
      return rv;
    }
    uq=AO_UserQueue_List_Next(uq);
  } /* while */

  if (!oks && errors) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Not a single job succeeded");
    return AB_ERROR_GENERIC;
  }

  return 0;
}



AB_ACCOUNT_LIST2 *AO_Provider_GetAccounts(AB_PROVIDER *pro) {
  AO_PROVIDER *dp;
  AB_ACCOUNT_LIST2 *nl;
  AO_BANK *b;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  if (AO_Bank_List_GetCount(dp->banks)==0)
    return 0;

  nl=AB_Account_List2_new();
  b=AO_Bank_List_First(dp->banks);
  while(b) {
    AB_ACCOUNT *a;

    a=AB_Account_List_First(AO_Bank_GetAccounts(b));
    while(a) {
      AB_Account_List2_PushBack(nl, a);
      a=AB_Account_List_Next(a);
    }
    b=AO_Bank_List_Next(b);
  }

  if (AB_Account_List2_GetSize(nl)==0) {
    AB_Account_List2_free(nl);
    return 0;
  }

  return nl;
}



int AO_Provider_AddBank(AB_PROVIDER *pro, AO_BANK *b) {
  AO_PROVIDER *dp;

  assert(b);
  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  AO_Bank_List_Add(b, dp->banks);
  return 0;
}



AO_BANK_LIST *AO_Provider_GetBanks(const AB_PROVIDER *pro) {
  AO_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  return dp->banks;
}




