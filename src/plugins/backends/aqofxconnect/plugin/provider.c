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
#include <gwenhywfar/net2.h>
#include <gwenhywfar/nl_ssl.h>
#include <gwenhywfar/nl_socket.h>
#include <gwenhywfar/nl_http.h>
#include <gwenhywfar/nl_packets.h>
#include <gwenhywfar/waitcallback.h>

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



void AO_Provider_FreeData(void *bp, void *p) {
  AO_PROVIDER *dp;

  dp=(AO_PROVIDER*)p;
  assert(dp);

  AO_Queue_free(dp->queue);
  AB_Job_List2_free(dp->bankingJobs);

  GWEN_FREE_OBJECT(dp);
}



int AO_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData) {
  AO_PROVIDER *dp;
#ifdef HAVE_I18N
  const char *s;
#endif
  const char *logLevelName;

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

  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Deinit done");

  if (errors)
    return AB_ERROR_GENERIC;

  return 0;
}



int AO_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j){
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
    return AB_ERROR_NOT_SUPPORTED;
  } /* switch */
}



int AO_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j){
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
    return AB_ERROR_NOT_SUPPORTED;
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



int AO_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx){
  AO_PROVIDER *dp;
  int oks=0;
  int errors=0;
  AB_JOB_LIST2_ITERATOR *jit;
  GWEN_TYPE_UINT32 pid;
  int rv;

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


  rv=AO_Provider_ExecQueue(pro, ctx);
  if (!rv)
    oks++;
  else {
    errors++;
    if (rv==AB_ERROR_USER_ABORT) {
      AO_Queue_Clear(dp->queue);
      AB_Job_List2_Clear(dp->bankingJobs);
      AB_Banking_ProgressEnd(AB_Provider_GetBanking(pro), pid);
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

  rv=AB_Banking_ExecutionProgress(AB_Provider_GetBanking(pro), 0);
  if (rv==AB_ERROR_USER_ABORT) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "User aborted");
    return rv;
  }

  AO_Queue_Clear(dp->queue);
  AB_Job_List2_Clear(dp->bankingJobs);
  AB_Banking_ProgressEnd(AB_Provider_GetBanking(pro), pid);

  if (!oks && errors) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Not a single job succeeded");
    return AB_ERROR_GENERIC;
  }

  return 0;
}


#if 0
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
#endif



void AO_Provider_AddBankCertFolder(AB_PROVIDER *pro,
                                   const AB_USER *u,
                                   GWEN_BUFFER *nbuf) {
  const char *s;

  AB_Provider_GetUserDataDir(pro, nbuf);
  GWEN_Buffer_AppendString(nbuf, "/banks/");
  s=AB_User_GetCountry(u);
  if (!s || !*s)
    s="us";
  GWEN_Buffer_AppendString(nbuf, s);
  GWEN_Buffer_AppendByte(nbuf, '/');
  s=AB_User_GetBankCode(u);
  assert(s);
  GWEN_Buffer_AppendString(nbuf, s);
  GWEN_Buffer_AppendByte(nbuf, '/');
  GWEN_Buffer_AppendString(nbuf, "/certs");
}




GWEN_NETLAYER *AO_Provider_CreateConnection(AB_PROVIDER *pro,
                                            AB_USER *u) {
  AO_PROVIDER *dp;
  GWEN_NETLAYER *nlBase;
  GWEN_NETLAYER *nl=0;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  const char *bankAddr;
  int bankPort;
  AO_USER_SERVERTYPE addrType;
  GWEN_ERRORCODE err;
  GWEN_BUFFER *nbuf;
  GWEN_URL *url;
  GWEN_DB_NODE *dbHeader;
  const char *s;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  /* create socket */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);

  /* create transport layer */
  nlBase=GWEN_NetLayerSocket_new(sk, 1);

  addrType=AO_User_GetServerType(u);

  /* get address */
  bankAddr=AO_User_GetServerAddr(u);
  if (!bankAddr) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "User has no valid address settings");
    GWEN_NetLayer_free(nlBase);
    return 0;
  }

  bankPort=AO_User_GetServerPort(u);
  if (bankPort<1) {
    /* set default port if none given */
    switch(addrType) {
    case AO_User_ServerTypeHTTP:  bankPort=80; break;
    case AO_User_ServerTypeHTTPS: bankPort=443; break;
    default:
      DBG_WARN(AQOFXCONNECT_LOGDOMAIN,
               "Unknown address type (%d), assuming HTTPS",
               addrType);
      bankPort=443;
      break;
    } /* switch */
  }

  /* extract address and local path from URL */
  url=GWEN_Url_fromString(bankAddr);
  if (!url) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Bad URL");
    GWEN_NetLayer_free(nlBase);
    return 0;
  }
  if (GWEN_Url_GetPort(url)!=0)
    bankPort=GWEN_Url_GetPort(url);

  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  err=GWEN_InetAddr_SetAddress(addr, GWEN_Url_GetServer(url));
  /* resolve address */
  if (!GWEN_Error_IsOk(err)) {
    char dbgbuf[256];

    snprintf(dbgbuf, sizeof(dbgbuf)-1,
	     I18N("Resolving hostname \"%s\" ..."),
	     GWEN_Url_GetServer(url));
    dbgbuf[sizeof(dbgbuf)-1]=0;
    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                           AB_Banking_LogLevelNotice,
                           dbgbuf);
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Resolving hostname \"%s\"",
	     GWEN_Url_GetServer(url));
    err=GWEN_InetAddr_SetName(addr, GWEN_Url_GetServer(url));
    if (!GWEN_Error_IsOk(err)) {
      snprintf(dbgbuf, sizeof(dbgbuf)-1,
	       I18N("Unknown hostname \"%s\""),
               GWEN_Url_GetServer(url));
      dbgbuf[sizeof(dbgbuf)-1]=0;
      AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                             AB_Banking_LogLevelError,
                             dbgbuf);
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
		"Error resolving hostname \"%s\":",
                GWEN_Url_GetServer(url));
      DBG_ERROR_ERR(AQOFXCONNECT_LOGDOMAIN, err);
      GWEN_Url_free(url);
      GWEN_NetLayer_free(nlBase);
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
  GWEN_NetLayer_SetPeerAddr(nlBase, addr);
  GWEN_InetAddr_free(addr);

  switch(addrType) {
  case AO_User_ServerTypeHTTP:
    break;

  case AO_User_ServerTypeHTTPS:
    nbuf=GWEN_Buffer_new(0, 64, 0, 1);
    AO_Provider_AddBankCertFolder(pro, u, nbuf);

    AB_Banking_ProgressLog(AB_Provider_GetBanking(pro), 0,
                           AB_Banking_LogLevelNotice,
			   I18N("Creating HTTPS connection"));

    nl=GWEN_NetLayerSsl_new(nlBase,
			    GWEN_Buffer_GetStart(nbuf),
			    GWEN_Buffer_GetStart(nbuf),
			    0, 0, 1);
    GWEN_Buffer_free(nbuf);
    GWEN_NetLayer_free(nlBase);
    nlBase=nl;
    GWEN_NetLayerSsl_SetAskAddCertFn(nlBase,
                                     AB_Banking_AskAddCert,
                                     AB_Provider_GetBanking(pro));
    break;

  default:
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Unknown server type %d", addrType);
    return 0;
  } /* switch */

  /* add HTTP protocol to the chain, prepare command and header */
  nl=GWEN_NetLayerHttp_new(nlBase);
  GWEN_NetLayer_free(nlBase);
  GWEN_NetLayerHttp_SetOutCommand(nl, "POST", url);
  dbHeader=GWEN_NetLayerHttp_GetOutHeader(nl);
  assert(dbHeader);
  s=GWEN_Url_GetServer(url);
  if (s)
    GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "Host", s);
  GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "Pragma", "no-cache");
  GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "Cache-control", "no cache");
  GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Content-type",
		       "application/x-ofx");
  GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Accept",
		       "*/*, application/x-ofx");
  GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Connection",
                       "close");
  GWEN_Url_free(url);
  nlBase=nl;

  /* add packets protocol to the chain */
  nl=GWEN_NetLayerPackets_new(nlBase);
  GWEN_NetLayer_free(nlBase);

  GWEN_Net_AddConnectionToPool(nl);

  /* return connection */
  return nl;
}



int AO_Provider_SendMessage(AB_PROVIDER *pro,
                            AB_USER *u,
			    GWEN_NETLAYER *nl,
                            const char *p,
                            unsigned int plen) {
  GWEN_BUFFER *nbuf;
  GWEN_NETLAYER_STATUS nst;
  GWEN_NL_PACKET *pk;
  int rv;

  assert(u);

  nst=GWEN_NetLayer_GetStatus(nl);
  if (nst==GWEN_NetLayerStatus_Disconnected) {
    /* connection down */
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
	     "Connection is down");
    return AB_ERROR_NETWORK;
  }

  nbuf=GWEN_Buffer_new(0, plen, 0, 1);
  GWEN_Buffer_AppendBytes(nbuf, p, plen);
  pk=GWEN_NL_Packet_new();
  GWEN_Buffer_Rewind(nbuf);
  GWEN_NL_Packet_SetBuffer(pk, nbuf);
  rv=GWEN_NetLayerPackets_SendPacket(nl, pk);
  if (rv) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
	     "Network error");
    GWEN_NL_Packet_free(pk);
    return AB_ERROR_NETWORK;
  }

  GWEN_NL_Packet_free(pk);

  DBG_DEBUG(AQOFXCONNECT_LOGDOMAIN, "Message enqueued");
  return 0;
}



int AO_Provider_SendAndReceive(AB_PROVIDER *pro,
                               AB_USER *u,
                               const char *p,
                               unsigned int plen,
                               GWEN_BUFFER **rbuf) {
  AO_PROVIDER *dp;
  GWEN_NETLAYER *nl;
  GWEN_NL_PACKET *pk;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  if (getenv("AQOFX_LOG_COMM")) {
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
  nl=AO_Provider_CreateConnection(pro, u);
  if (!nl) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Could not create connection");
    return AB_ERROR_GENERIC;
  }

  /* connect */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                         0,
                         AB_Banking_LogLevelInfo,
                         I18N("Connecting..."));
  rv=GWEN_NetLayer_Connect_Wait(nl, dp->connectTimeout);
  if (rv) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not connect to bank (%d)", rv);
    GWEN_NetLayer_free(nl);
    return AB_ERROR_NETWORK;
  }

  /* send message */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                         0,
                         AB_Banking_LogLevelInfo,
                         I18N("Sending request..."));
  rv=AO_Provider_SendMessage(pro, u, nl, p, plen);
  if (rv) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "Error %d", rv);
    GWEN_NetLayer_Disconnect(nl);
    GWEN_NetLayer_free(nl);
    return rv;
  }

  /* make sure message gets sent */
  rv=GWEN_NetLayerPackets_Flush(nl, dp->sendTimeout);
  if (rv) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
	     "Network error");
    GWEN_NetLayer_Disconnect(nl);
    GWEN_NetLayer_free(nl);
    return AB_ERROR_NETWORK;
  }

  /* wait for response */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                         0,
                         AB_Banking_LogLevelInfo,
			 I18N("Waiting for response..."));
  pk=GWEN_NetLayerPackets_GetNextPacket_Wait(nl, dp->recvTimeout);
  if (!pk) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "No response received");
    GWEN_NetLayer_Disconnect(nl);
    GWEN_NetLayer_free(nl);
    return AB_ERROR_NETWORK;
  }

  /* found a response, transform it */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                         0,
                         AB_Banking_LogLevelInfo,
                         I18N("Parsing response..."));

  /* disconnect */
  AB_Banking_ProgressLog(AB_Provider_GetBanking(pro),
                         0,
                         AB_Banking_LogLevelInfo,
                         I18N("Disconnecting..."));
  if (GWEN_NetLayer_Disconnect(nl)) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
	     "Could not disconnect connection");
  }

  *rbuf=GWEN_NL_Packet_TakeBuffer(pk);
  GWEN_NL_Packet_free(pk);

  /* release connection ressources */
  GWEN_NetLayer_free(nl);

  if (getenv("AQOFX_LOG_COMM")) {
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
      if (fwrite(GWEN_Buffer_GetStart(*rbuf),
		 GWEN_Buffer_GetUsedBytes(*rbuf),
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
                                AB_USER *u) {
  AO_PROVIDER *dp;
  AO_CONTEXT *ctx;
  char *msg;
  GWEN_BUFFER *rbuf;
  int rv;
  GWEN_TYPE_UINT32 pid;
  AB_IMEXPORTER_CONTEXT *ictx;

  assert(u);
  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

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
  ctx=AO_Context_new(u, 0, ictx);
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

  fprintf(stderr, "Sending this: %s\n", msg);
  //AO_Context_free(ctx);
  //AB_Banking_ProgressEnd(AB_Provider_GetBanking(pro), pid);
  //return 0;

  rv=AO_Provider_SendAndReceive(pro, u, msg, strlen(msg), &rbuf);
  if (rv) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Error exchanging getAccounts-request (%d)", rv);
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

  /* create and setup context */
  ctx=AO_Context_new(u, j, ictx);
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

  rv=AO_Provider_SendAndReceive(pro, u, msg, strlen(msg), &rbuf);
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



int AO_Provider_ExecUserQueue(AB_PROVIDER *pro,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              AO_USERQUEUE *uq){
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
        int rv;

	/* start new context */
        rv=AO_Provider_RequestStatements(pro, uj, ctx);
	if (rv==AB_ERROR_USER_ABORT) {
	  DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "User aborted");
	  AB_Job_List2Iterator_free(jit);
	  return rv;
	}
	else if (rv==AB_ERROR_ABORTED) {
          DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Aborted");
          break;
        }

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



int AO_Provider_ExecQueue(AB_PROVIDER *pro,
                          AB_IMEXPORTER_CONTEXT *ctx) {
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
    rv=AO_Provider_ExecUserQueue(pro, ctx, uq);
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



