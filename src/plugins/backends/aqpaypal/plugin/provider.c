/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_p.h"
#include "user_l.h"

#include "dlg_newuser_l.h"
#include "dlg_edituser_l.h"

#include <aqbanking/job_be.h>
#include <aqbanking/httpsession.h>
#include <aqbanking/jobgettransactions.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/i18n.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/smalltresor.h>
#include <gwenhywfar/directory.h>

#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


#define AQPAYPAL_PASSWORD_ITERATIONS 1467
#define AQPAYPAL_CRYPT_ITERATIONS    648

#define AQPAYPAL_API_VER    "56.0"


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)


/*#define DEBUG_PAYPAL */



GWEN_INHERIT(AB_PROVIDER, APY_PROVIDER)



AB_PROVIDER *APY_Provider_new(AB_BANKING *ab){
  AB_PROVIDER *pro;
  APY_PROVIDER *xp;

  pro=AB_Provider_new(ab, APY_PROVIDER_NAME);
  GWEN_NEW_OBJECT(APY_PROVIDER, xp);
  GWEN_INHERIT_SETDATA(AB_PROVIDER, APY_PROVIDER, pro, xp,
		       APY_Provider_FreeData);


  AB_Provider_SetInitFn(pro, APY_Provider_Init);
  AB_Provider_SetFiniFn(pro, APY_Provider_Fini);
  AB_Provider_SetUpdateJobFn(pro, APY_Provider_UpdateJob);
  AB_Provider_SetAddJobFn(pro, APY_Provider_AddJob);
  AB_Provider_SetExecuteFn(pro, APY_Provider_Execute);
  AB_Provider_SetResetQueueFn(pro, APY_Provider_ResetQueue);
  AB_Provider_SetExtendUserFn(pro, APY_Provider_ExtendUser);
  AB_Provider_SetExtendAccountFn(pro, APY_Provider_ExtendAccount);

  AB_Provider_SetGetNewUserDialogFn(pro, APY_Provider_GetNewUserDialog);
  AB_Provider_SetGetEditUserDialogFn(pro, APY_Provider_GetEditUserDialog);

  AB_Provider_AddFlags(pro,
		       AB_PROVIDER_FLAGS_HAS_EDITUSER_DIALOG |
		       AB_PROVIDER_FLAGS_HAS_NEWUSER_DIALOG);

  return pro;
}



void GWENHYWFAR_CB APY_Provider_FreeData(void *bp, void *p) {
  APY_PROVIDER *xp;

  xp=(APY_PROVIDER*) p;

  GWEN_FREE_OBJECT(xp);
}



int APY_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData) {
  APY_PROVIDER *dp;
  const char *logLevelName;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(dp);

  if (!GWEN_Logger_IsOpen(AQPAYPAL_LOGDOMAIN)) {
    GWEN_Logger_Open(AQPAYPAL_LOGDOMAIN,
		     "aqpaypal", 0,
		     GWEN_LoggerType_Console,
		     GWEN_LoggerFacility_User);
  }

  logLevelName=getenv("AQPAYPAL_LOGLEVEL");
  if (logLevelName) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(logLevelName);
    if (ll!=GWEN_LoggerLevel_Unknown) {
      GWEN_Logger_SetLevel(AQPAYPAL_LOGDOMAIN, ll);
      DBG_WARN(AQPAYPAL_LOGDOMAIN,
               "Overriding loglevel for AqPAYPAL with \"%s\"",
               logLevelName);
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Unknown loglevel \"%s\"",
                logLevelName);
    }
  }

  if (1) {
    GWEN_STRINGLIST *sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME,
						  AB_PM_LOCALEDIR);
    const char *localedir=GWEN_StringList_FirstString(sl);
    int rv;

    rv=GWEN_I18N_BindTextDomain_Dir(PACKAGE, localedir);
    if (rv) {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not bind textdomain (%d)", rv);
    }
    else {
      rv=GWEN_I18N_BindTextDomain_Codeset(PACKAGE, "UTF-8");
      if (rv) {
	DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not set codeset (%d)", rv);
      }
    }

    GWEN_StringList_free(sl);
  }

  DBG_NOTICE(AQPAYPAL_LOGDOMAIN, "Initializing AqPaypal backend");

  return 0;
}



int APY_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData){
  APY_PROVIDER *dp;
  uint32_t currentVersion;

  DBG_NOTICE(AQPAYPAL_LOGDOMAIN, "Deinitializing AqPaypal backend");

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(dp);

  currentVersion=
    (AQPAYPAL_VERSION_MAJOR<<24) |
    (AQPAYPAL_VERSION_MINOR<<16) |
    (AQPAYPAL_VERSION_PATCHLEVEL<<8) |
    AQPAYPAL_VERSION_BUILD;

  /* save configuration */
  DBG_NOTICE(AQPAYPAL_LOGDOMAIN, "Setting version %08x",
             currentVersion);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "lastVersion", currentVersion);

  return 0;
}



int APY_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j){
  APY_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(dp);

  switch(AB_Job_GetType(j)) {
  case AB_Job_TypeGetTransactions:
    break;
  case AB_Job_TypeGetBalance:
     break;
  case AB_Job_TypeTransfer:
  case AB_Job_TypeDebitNote:
  default:
    DBG_INFO(AQPAYPAL_LOGDOMAIN,
	     "Job not yet supported (%d)",
	     AB_Job_GetType(j));
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */

  return 0;
}



int APY_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j){
  APY_PROVIDER *xp;
  AB_ACCOUNT *a;
  AB_USER *u;
  int doAdd=1;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  switch(AB_Job_GetType(j)) {
  case AB_Job_TypeGetTransactions:
    break;
  case AB_Job_TypeGetBalance:
    break;
  case AB_Job_TypeTransfer:
  case AB_Job_TypeDebitNote:
  default:
    DBG_INFO(AQPAYPAL_LOGDOMAIN,
	     "Job not supported (%d)",
	     AB_Job_GetType(j));
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */

  a=AB_Job_GetAccount(j);
  if (a==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "No account for job");
    return GWEN_ERROR_GENERIC;
  }

  u=AB_Account_GetFirstUser(a);
  if (u==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "No user for account");
    return GWEN_ERROR_GENERIC;
  }

  if (xp->queue==NULL)
    xp->queue=AB_Queue_new();

  if (AB_Job_GetType(j)==AB_Job_TypeGetTransactions) {
    AB_JOB *firstJob;

    firstJob=AB_Queue_FindFirstJobLikeThis(xp->queue, u, j);
    if (firstJob) {
      GWEN_DB_NODE *dbCurrJob;

      /* this job is just a copy of the firstJob, reference it */
      dbCurrJob=AB_Job_GetProviderData(j, pro);
      assert(dbCurrJob);

      GWEN_DB_SetIntValue(dbCurrJob,
			  GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "refJob",
			  AB_Job_GetJobId(firstJob));
      /* don't add to queues */
      doAdd=0;
    }
  }

  if (doAdd)
    AB_Queue_AddJob(xp->queue, u, j);

  return 0;
}



int APY_Provider_ResetQueue(AB_PROVIDER *pro){
  APY_PROVIDER *xp;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  if (xp->queue) {
    AB_Queue_free(xp->queue);
    xp->queue=NULL;
  }

  return 0;
}



int APY_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
			    AB_PROVIDER_EXTEND_MODE em,
			    GWEN_DB_NODE *dbBackend) {
  APY_User_Extend(u, pro, em, dbBackend);
  return 0;
}



int APY_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
			       AB_PROVIDER_EXTEND_MODE em,
			       GWEN_DB_NODE *dbBackend){
  return 0;
}





int APY_Provider_ParseResponse(AB_PROVIDER *pro, const char *s, GWEN_DB_NODE *db) {
  /* read vars */
  while(*s) {
    GWEN_BUFFER *bName;
    GWEN_BUFFER *bValue;
    const char *p;
    GWEN_DB_NODE *dbT;

    bName=GWEN_Buffer_new(0, 256, 0, 1);
    bValue=GWEN_Buffer_new(0, 256, 0, 1);
    p=s;
    while(*p && *p!='&' && *p!='=')
      p++;
    if (p!=s)
      GWEN_Buffer_AppendBytes(bName, s, (p-s));
    s=p;
    if (*p=='=') {
      s++;
      p=s;
      while(*p && *p!='&')
        p++;
      if (p!=s)
        GWEN_Buffer_AppendBytes(bValue, s, (p-s));
      s=p;
    }

    dbT=db;
    if (strncasecmp(GWEN_Buffer_GetStart(bName), "L_ERRORCODE", 11)!=0 &&
	strncasecmp(GWEN_Buffer_GetStart(bName), "L_SHORTMESSAGE", 14)!=0 &&
	strncasecmp(GWEN_Buffer_GetStart(bName), "L_LONGMESSAGE", 13)!=0 &&
	strncasecmp(GWEN_Buffer_GetStart(bName), "L_SEVERITYCODE", 14)!=0 &&
	strncasecmp(GWEN_Buffer_GetStart(bName), "SHIPTOSTREET2", 13)!=0) {
      int i;

      i=GWEN_Buffer_GetUsedBytes(bName)-1;
      if (i>0) {
	char *t;
  
	t=GWEN_Buffer_GetStart(bName)+i;
	while(i && isdigit(*t)) {
	  i--;
	  t--;
	}
	if (i>0) {
	  t++;
	  if (*t) {
	    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, t);
	    *t=0;
	  }
	}
      }
    }

    /* store variable/value pair */
    if (strlen(GWEN_Buffer_GetStart(bName))) {
      GWEN_BUFFER *xbuf;

      xbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Text_UnescapeToBufferTolerant(GWEN_Buffer_GetStart(bValue), xbuf);
      GWEN_DB_SetCharValue(dbT,
			   GWEN_DB_FLAGS_DEFAULT,
			   GWEN_Buffer_GetStart(bName),
			   GWEN_Buffer_GetStart(xbuf));
      GWEN_Buffer_free(xbuf);
    }

    GWEN_Buffer_free(bValue);
    GWEN_Buffer_free(bName);
    if (*s!='&')
      break;
    s++;
  }

  return 0;
}



int APY_Provider_UpdateTrans(AB_PROVIDER *pro,
                             AB_USER *u,
			     AB_TRANSACTION *t) {
  GWEN_HTTP_SESSION *sess;
  GWEN_BUFFER *tbuf;
  const char *s;
  int vmajor;
  int vminor;
  int rv;
  GWEN_DB_NODE *dbResponse;
  GWEN_DB_NODE *dbT;

  sess=AB_HttpSession_new(pro, u,
			  APY_User_GetServerUrl(u),
			  "https",
			  443);
  if (sess==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not create http session for user [%s]",
	      AB_User_GetUserId(u));
    return GWEN_ERROR_GENERIC;
  }

  vmajor=APY_User_GetHttpVMajor(u);
  vminor=APY_User_GetHttpVMinor(u);
  if (vmajor==0 && vminor==0) {
    vmajor=1;
    vminor=0;
  }
  GWEN_HttpSession_SetHttpVMajor(sess, vmajor);
  GWEN_HttpSession_SetHttpVMinor(sess, vminor);
  GWEN_HttpSession_SetHttpContentType(sess, "application/x-www-form-urlencoded");

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(tbuf, "user=");
  s=APY_User_GetApiUserId(u);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing user id");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&pwd=");
  s=APY_User_GetApiPassword(u);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing API password");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&signature=");
  s=APY_User_GetApiSignature(u);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing API signature");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&version=");
  GWEN_Text_EscapeToBuffer(AQPAYPAL_API_VER, tbuf);
  GWEN_Buffer_AppendString(tbuf, "&method=getTransactionDetails");

  GWEN_Buffer_AppendString(tbuf, "&transactionId=");
  s=AB_Transaction_GetFiId(t);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing transaction id");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    return GWEN_ERROR_INVALID;
  }

  /* init session */
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  if (getenv("AQPAYPAL_LOG_COMM")) {
    int len;
    FILE *f;

    len=GWEN_Buffer_GetUsedBytes(tbuf);

    f=fopen("paypal.log", "a+");
    if (f) {
      fprintf(f, "\n============================================\n");
      fprintf(f, "Sending (UpdateTrans):\n");
      if (len>0) {
	if (1!=fwrite(GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(tbuf), 1, f)) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	  fclose(f);
	}
	else {
	  if (fclose(f)) {
	    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	  }
	}
      }
      else {
	fprintf(f, "Empty data.\n");
	if (fclose(f)) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	}
      }
    }
  }

  /* send request */
  rv=GWEN_HttpSession_SendPacket(sess, "POST",
				 (const uint8_t*) GWEN_Buffer_GetStart(tbuf),
				 GWEN_Buffer_GetUsedBytes(tbuf));
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_Fini(sess);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* get response */
  GWEN_Buffer_Reset(tbuf);
  rv=GWEN_HttpSession_RecvPacket(sess, tbuf);
  if (rv<0 || rv<200 || rv>299) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_Fini(sess);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  if (getenv("AQPAYPAL_LOG_COMM")) {
    int len;
    FILE *f;

    len=GWEN_Buffer_GetUsedBytes(tbuf);

    f=fopen("paypal.log", "a+");
    if (f) {
      fprintf(f, "\n============================================\n");
      fprintf(f, "Received (UpdateTrans):\n");
      if (len>0) {
	if (1!=fwrite(GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(tbuf), 1, f)) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	  fclose(f);
	}
	else {
	  if (fclose(f)) {
	    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	  }
	}
      }
      else {
	fprintf(f, "Empty data.\n");
	if (fclose(f)) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	}
      }
    }
  }

  /* deinit (ignore result because it isn't important) */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  /* parse response */
  dbResponse=GWEN_DB_Group_new("response");
  rv=APY_Provider_ParseResponse(pro, GWEN_Buffer_GetStart(tbuf), dbResponse);

#if 0
  if (getenv("AQPAYPAL_LOG_COMM")) {
    static int debugCounter=0;
    char namebuf[64];

    snprintf(namebuf, sizeof(namebuf)-1, "paypal-%02x.db", debugCounter++);
    GWEN_DB_WriteFile(dbResponse, namebuf, GWEN_DB_FLAGS_DEFAULT);
  }
#endif

  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbResponse);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* check result */
  s=GWEN_DB_GetCharValue(dbResponse, "ACK", 0, NULL);
  if (s && *s) {
    if (strcasecmp(s, "Success")==0 ||
	strcasecmp(s, "SuccessWithWarning")==0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "Success");
    }
    else {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "No positive response from server");
      GWEN_DB_Group_free(dbResponse);
      GWEN_Buffer_free(tbuf);
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "No ACK response from server");
    GWEN_DB_Group_free(dbResponse);
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_BAD_DATA;
  }


  /* parse response */
  s=GWEN_DB_GetCharValue(dbResponse, "TRANSACTIONTYPE", 0, NULL);
  if (s && *s)
    AB_Transaction_SetTransactionText(t, s);
  /* address */
  s=GWEN_DB_GetCharValue(dbResponse, "SHIPTOSTREET", 0, NULL);
  if (s && *s)
    AB_Transaction_SetRemoteAddrStreet(t, s);
  s=GWEN_DB_GetCharValue(dbResponse, "SHIPTOCITY", 0, NULL);
  if (s && *s)
    AB_Transaction_SetRemoteAddrCity(t, s);
  s=GWEN_DB_GetCharValue(dbResponse, "SHIPTOZIP", 0, NULL);
  if (s && *s)
    AB_Transaction_SetRemoteAddrZipcode(t, s);

  s=GWEN_DB_GetCharValue(dbResponse, "PAYMENTSTATUS", 0, NULL);
  if (s && *s) {
    if (strcasecmp(s, "Completed")==0)
      AB_Transaction_SetStatus(t, AB_Transaction_StatusAccepted);
    else if (strcasecmp(s, "Denied")==0 ||
	     strcasecmp(s, "Failed")==0 ||
	     strcasecmp(s, "Expired")==0 ||
	     strcasecmp(s, "Voided")==0)
      AB_Transaction_SetStatus(t, AB_Transaction_StatusRejected);
    else if (strcasecmp(s, "Pending")==0 ||
	     strcasecmp(s, "Processed")==0)
      AB_Transaction_SetStatus(t, AB_Transaction_StatusPending);
    else if (strcasecmp(s, "Refunded")==0 ||
	     strcasecmp(s, "Reversed")==0)
      AB_Transaction_SetStatus(t, AB_Transaction_StatusRevoked);
    else {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "Unknown payment status (%s)", s);
    }
  }

  s=GWEN_DB_GetCharValue(dbResponse, "BUYERID", 0, NULL);
  if (s && *s)
    AB_Transaction_SetBankReference(t, s);

  dbT=GWEN_DB_GetFirstGroup(dbResponse);
  while(dbT) {
    GWEN_BUFFER *pbuf;

    pbuf=GWEN_Buffer_new(0, 256, 0, 1);
    s=GWEN_DB_GetCharValue(dbT, "L_QTY", 0, NULL);
    if (s && *s) {
      GWEN_Buffer_AppendString(pbuf, s);
      GWEN_Buffer_AppendString(pbuf, "x");
    }
    s=GWEN_DB_GetCharValue(dbT, "L_NAME", 0, NULL);
    if (s && *s) {
      GWEN_Buffer_AppendString(pbuf, s);
      s=GWEN_DB_GetCharValue(dbT, "L_NUMBER", 0, NULL);
      if (s && *s) {
	GWEN_Buffer_AppendString(pbuf, "(");
	GWEN_Buffer_AppendString(pbuf, s);
	GWEN_Buffer_AppendString(pbuf, ")");
      }
    }
    else {
      s=GWEN_DB_GetCharValue(dbT, "L_NUMBER", 0, NULL);
      if (s && *s)
	GWEN_Buffer_AppendString(pbuf, s);
    }

    s=GWEN_DB_GetCharValue(dbT, "L_AMT", 0, NULL);
    if (s && *s) {
      GWEN_Buffer_AppendString(pbuf, "[");
      GWEN_Buffer_AppendString(pbuf, s);
      s=GWEN_DB_GetCharValue(dbT, "L_CURRENCYCODE", 0, NULL);
      if (s && *s) {
	GWEN_Buffer_AppendString(pbuf, " ");
	GWEN_Buffer_AppendString(pbuf, s);
      }
      GWEN_Buffer_AppendString(pbuf, "]");

    }

    AB_Transaction_AddPurposeLine(t, GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);

    dbT=GWEN_DB_GetNextGroup(dbT);
  }

  GWEN_DB_Group_free(dbResponse);
  GWEN_Buffer_free(tbuf);


  return 0;
}



int APY_Provider_ExecGetTrans(AB_PROVIDER *pro,
			      AB_IMEXPORTER_ACCOUNTINFO *ai,
			      AB_USER *u,
			      AB_JOB *j) {
  GWEN_HTTP_SESSION *sess;
  GWEN_BUFFER *tbuf;
  const char *s;
  const GWEN_TIME *ti;
  int vmajor;
  int vminor;
  int rv;
  GWEN_DB_NODE *dbResponse;
  GWEN_DB_NODE *dbT;

  sess=AB_HttpSession_new(pro, u,
			  APY_User_GetServerUrl(u),
			  "https",
			  443);
  if (sess==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not create http session for user [%s]",
	      AB_User_GetUserId(u));
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return GWEN_ERROR_GENERIC;
  }

  vmajor=APY_User_GetHttpVMajor(u);
  vminor=APY_User_GetHttpVMinor(u);
  if (vmajor==0 && vminor==0) {
    vmajor=1;
    vminor=0;
  }
  GWEN_HttpSession_SetHttpVMajor(sess, vmajor);
  GWEN_HttpSession_SetHttpVMinor(sess, vminor);
  GWEN_HttpSession_SetHttpContentType(sess, "application/x-www-form-urlencoded");

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(tbuf, "user=");
  s=APY_User_GetApiUserId(u);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing user id");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&pwd=");
  s=APY_User_GetApiPassword(u);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing API password");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&signature=");
  s=APY_User_GetApiSignature(u);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing API signature");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&version=");
  GWEN_Text_EscapeToBuffer(AQPAYPAL_API_VER, tbuf);

  GWEN_Buffer_AppendString(tbuf, "&method=transactionSearch");

  ti=AB_JobGetTransactions_GetFromTime(j);
  if (ti==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing start date");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&startdate=");
  GWEN_Time_toUtcString(ti, "YYYY-MM-DDThh:mm:ssZ", tbuf);
  //testing: GWEN_Buffer_AppendString(tbuf, "&enddate=2016-01-01T00:00:00Z");

  /* init session */
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return rv;
  }

  if (getenv("AQPAYPAL_LOG_COMM")) {
    int len;
    FILE *f;

    len=GWEN_Buffer_GetUsedBytes(tbuf);

    f=fopen("paypal.log", "a+");
    if (f) {
      fprintf(f, "\n============================================\n");
      fprintf(f, "Sending (GetTrans):\n");
      if (len>0) {
	if (1!=fwrite(GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(tbuf), 1, f)) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	  fclose(f);
	}
	else {
	  if (fclose(f)) {
	    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	  }
	}
      }
      else {
	fprintf(f, "Empty data.\n");
	if (fclose(f)) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	}
      }
    }
  }


#if 0
  DBG_ERROR(0, "Would send this: [%s]", GWEN_Buffer_GetStart(tbuf));
  GWEN_HttpSession_Fini(sess);
  GWEN_Buffer_free(tbuf);
  GWEN_HttpSession_free(sess);
  AB_Job_SetStatus(j, AB_Job_StatusError);
  return GWEN_ERROR_INTERNAL;
#endif

  /* send request */
  rv=GWEN_HttpSession_SendPacket(sess, "POST",
				 (const uint8_t*) GWEN_Buffer_GetStart(tbuf),
				 GWEN_Buffer_GetUsedBytes(tbuf));
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_Fini(sess);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return rv;
  }
  AB_Job_SetStatus(j, AB_Job_StatusSent);

  /* get response */
  GWEN_Buffer_Reset(tbuf);
  rv=GWEN_HttpSession_RecvPacket(sess, tbuf);
  if (rv<0 || rv<200 || rv>299) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_Fini(sess);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return rv;
  }

  if (getenv("AQPAYPAL_LOG_COMM")) {
    int len;
    FILE *f;

    len=GWEN_Buffer_GetUsedBytes(tbuf);
    f=fopen("paypal.log", "a+");
    if (f) {
      fprintf(f, "\n============================================\n");
      fprintf(f, "Received (GetTrans):\n");
      if (len>0) {
	if (1!=fwrite(GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(tbuf), 1, f)) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	  fclose(f);
	}
	else {
	  if (fclose(f)) {
	    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	  }
	}
      }
      else {
	fprintf(f, "Empty data.\n");
	if (fclose(f)) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	}
      }
    }
  }


  /* deinit (ignore result because it isn't important) */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  /* parse response */
  dbResponse=GWEN_DB_Group_new("response");
  rv=APY_Provider_ParseResponse(pro, GWEN_Buffer_GetStart(tbuf), dbResponse);
#ifdef DEBUG_PAYPAL
  GWEN_DB_WriteFile(dbResponse, "paypal.db", GWEN_DB_FLAGS_DEFAULT);
#endif
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbResponse);
    GWEN_Buffer_free(tbuf);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return rv;
  }

  /* check result */
  s=GWEN_DB_GetCharValue(dbResponse, "ACK", 0, NULL);
  if (s && *s) {
    if (strcasecmp(s, "Success")==0 ||
	strcasecmp(s, "SuccessWithWarning")==0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "Success");
    }
    else {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "No positive response from server");
      GWEN_DB_Group_free(dbResponse);
      GWEN_Buffer_free(tbuf);
      AB_Job_SetStatus(j, AB_Job_StatusError);
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "No ACK response from server");
    GWEN_DB_Group_free(dbResponse);
    GWEN_Buffer_free(tbuf);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return GWEN_ERROR_BAD_DATA;
  }

  /* now get the transactions */
  dbT=GWEN_DB_GetFirstGroup(dbResponse);
  while(dbT) {
    AB_TRANSACTION *t;
    int dontKeep;

    dontKeep=0;

    t=AB_Transaction_new();
    s=GWEN_DB_GetCharValue(dbT, "L_TIMESTAMP", 0, NULL);
    if (s && *s) {
      GWEN_DATE *da;

      da=GWEN_Date_fromStringWithTemplate(s, "YYYY-MM-DD");
      if (da) {
        AB_Transaction_SetDate(t, da);
        AB_Transaction_SetValutaDate(t, da);
        GWEN_Date_free(da);
      }
      else {
	DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Invalid timespec [%s]", s);
      }
    }

    s=GWEN_DB_GetCharValue(dbT, "L_TYPE", 0, NULL);
    if (s && *s) {
      // reverse engineered rule which transactions to keep and which not
      if (strcasecmp(s, "Authorization")==0 || strcasecmp(s, "Order")==0 )
	dontKeep++;

      /* TODO: maybe handle those types differently? */
      if (strcasecmp(s, "Transfer")==0) {
	AB_Transaction_SetType(t, AB_Transaction_TypeTransaction);
	AB_Transaction_SetSubType(t, AB_Transaction_SubTypeStandard);
      }
      else if (strcasecmp(s, "Payment")==0) {
	AB_Transaction_SetType(t, AB_Transaction_TypeTransaction);
	AB_Transaction_SetSubType(t, AB_Transaction_SubTypeStandard);
      }
      else {
	AB_Transaction_SetType(t, AB_Transaction_TypeTransaction);
	AB_Transaction_SetSubType(t, AB_Transaction_SubTypeStandard);
      }
    }

    s=GWEN_DB_GetCharValue(dbT, "L_NAME", 0, NULL);
    if (s && *s)
      AB_Transaction_SetRemoteName(t, s);

    s=GWEN_DB_GetCharValue(dbT, "L_TRANSACTIONID", 0, NULL);
    if (s && *s)
      AB_Transaction_SetFiId(t, s);

    s=GWEN_DB_GetCharValue(dbT, "L_AMT", 0, NULL);
    if (s && *s) {
      AB_VALUE *v;

      v=AB_Value_fromString(s);
      if (v==NULL) {
	DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Invalid amount [%s]", s);
      }
      else {
	s=GWEN_DB_GetCharValue(dbT, "L_CURRENCYCODE", 0, NULL);
	if (s && *s)
	  AB_Value_SetCurrency(v, s);
	AB_Transaction_SetValue(t, v);
	AB_Value_free(v);
      }
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing amount");
    }

    s=GWEN_DB_GetCharValue(dbT, "L_FEEAMT", 0, NULL);
    if (s && *s) {
      AB_VALUE *v;

      v=AB_Value_fromString(s);
      if (v==NULL) {
	DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Invalid fee amount [%s]", s);
      }
      else {
	s=GWEN_DB_GetCharValue(dbT, "L_CURRENCYCODE", 0, NULL);
	if (s && *s)
	  AB_Value_SetCurrency(v, s);
	AB_Transaction_SetFees(t, v);
	AB_Value_free(v);
      }
    }

    s=GWEN_DB_GetCharValue(dbT, "L_STATUS", 0, NULL);
    if (s && *s) {
      // reverse engineered rule which transactions to keep and which not
      if (strcasecmp(s, "Placed")==0 || strcasecmp(s, "Removed")==0 )
	dontKeep++;

      if (strcasecmp(s, "Completed")==0)
	AB_Transaction_SetStatus(t, AB_Transaction_StatusAccepted);
      else
	AB_Transaction_SetStatus(t, AB_Transaction_StatusPending);
    }

    /* get transaction details */
    s=AB_Transaction_GetFiId(t);
    if (s && *s) {
      const char *s2;

      s2=GWEN_DB_GetCharValue(dbT, "L_TYPE", 0, NULL);
      if (s2 && *s2) {
        /* only get details for payments (maybe add other types later) */
	if (strcasecmp(s2, "Payment")==0 ||
	    strcasecmp(s2, "Purchase")==0 ||
	    strcasecmp(s2, "Donation")==0) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "Getting details for transaction [%s]", s);
	  rv=APY_Provider_UpdateTrans(pro, u, t);
	  if (rv<0) {
	    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
	  }
	}
      }
    }

    /* add transaction */
    /* but only if L_TYPE neither Authorization nor Order */
    s=GWEN_DB_GetCharValue(dbT, "L_TYPE", 0, NULL);
    if (s && *s ) {
     if (!dontKeep) AB_ImExporterAccountInfo_AddTransaction(ai, t);
    }

    dbT=GWEN_DB_GetNextGroup(dbT);
  }

  GWEN_DB_Group_free(dbResponse);
  GWEN_Buffer_free(tbuf);
  AB_Job_SetStatus(j, AB_Job_StatusFinished);
  return 0;
}


int APY_Provider_ExecGetBal(AB_PROVIDER *pro,
			      AB_IMEXPORTER_ACCOUNTINFO *ai,
			      AB_USER *u,
			      AB_JOB *j) {
  GWEN_HTTP_SESSION *sess;
  GWEN_BUFFER *tbuf;
  const char *s;
  int vmajor;
  int vminor;
  int rv;
  GWEN_DB_NODE *dbResponse;
  GWEN_DB_NODE *dbCurr;

  sess=AB_HttpSession_new(pro, u,
			  APY_User_GetServerUrl(u),
			  "https",
			  443);
  if (sess==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not create http session for user [%s]",
	      AB_User_GetUserId(u));
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return GWEN_ERROR_GENERIC;
  }

  vmajor=APY_User_GetHttpVMajor(u);
  vminor=APY_User_GetHttpVMinor(u);
  if (vmajor==0 && vminor==0) {
    vmajor=1;
    vminor=0;
  }
  GWEN_HttpSession_SetHttpVMajor(sess, vmajor);
  GWEN_HttpSession_SetHttpVMinor(sess, vminor);
  GWEN_HttpSession_SetHttpContentType(sess, "application/x-www-form-urlencoded");

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(tbuf, "user=");
  s=APY_User_GetApiUserId(u);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing user id");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&pwd=");
  s=APY_User_GetApiPassword(u);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing API password");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&signature=");
  s=APY_User_GetApiSignature(u);
  if (s && *s)
    GWEN_Text_EscapeToBuffer(s, tbuf);
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing API signature");
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return GWEN_ERROR_INVALID;
  }

  GWEN_Buffer_AppendString(tbuf, "&version=");
  GWEN_Text_EscapeToBuffer(AQPAYPAL_API_VER, tbuf);

  GWEN_Buffer_AppendString(tbuf, "&method=GetBalance");

  /* init session */
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return rv;
  }

  if (getenv("AQPAYPAL_LOG_COMM")) {
    int len;
    FILE *f;

    len=GWEN_Buffer_GetUsedBytes(tbuf);

    f=fopen("paypal.log", "a+");
    if (f) {
      fprintf(f, "\n============================================\n");
      fprintf(f, "Sending (GetBal):\n");
      if (len>0) {
	if (1!=fwrite(GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(tbuf), 1, f)) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	  fclose(f);
	}
	else {
	  if (fclose(f)) {
	    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	  }
	}
      }
      else {
	fprintf(f, "Empty data.\n");
	if (fclose(f)) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	}
      }
    }
  }


  /* send request */
  rv=GWEN_HttpSession_SendPacket(sess, "POST",
				 (const uint8_t*) GWEN_Buffer_GetStart(tbuf),
				 GWEN_Buffer_GetUsedBytes(tbuf));
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_Fini(sess);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return rv;
  }
  AB_Job_SetStatus(j, AB_Job_StatusSent);

  /* get response */
  GWEN_Buffer_Reset(tbuf);
  rv=GWEN_HttpSession_RecvPacket(sess, tbuf);
  if (rv<0 || rv<200 || rv>299) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_Fini(sess);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return rv;
  }

  if (getenv("AQPAYPAL_LOG_COMM")) {
    int len;
    FILE *f;

    len=GWEN_Buffer_GetUsedBytes(tbuf);
    f=fopen("paypal.log", "a+");
    if (f) {
      fprintf(f, "\n============================================\n");
      fprintf(f, "Received (GetBal):\n");
      if (len>0) {
	if (1!=fwrite(GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(tbuf), 1, f)) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	  fclose(f);
	}
	else {
	  if (fclose(f)) {
	    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	  }
	}
      }
      else {
	fprintf(f, "Empty data.\n");
	if (fclose(f)) {
	  DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
	}
      }
    }
  }


  /* deinit (ignore result because it isn't important) */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  /* parse response */
  dbResponse=GWEN_DB_Group_new("response");
  rv=APY_Provider_ParseResponse(pro, GWEN_Buffer_GetStart(tbuf), dbResponse);
#ifdef DEBUG_PAYPAL
  GWEN_DB_WriteFile(dbResponse, "paypal.db", GWEN_DB_FLAGS_DEFAULT);
#endif
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbResponse);
    GWEN_Buffer_free(tbuf);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return rv;
  }

  /* check result */
  s=GWEN_DB_GetCharValue(dbResponse, "ACK", 0, NULL);
  if (s && *s) {
    if (strcasecmp(s, "Success")==0 ||
	strcasecmp(s, "SuccessWithWarning")==0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "Success");
    }
    else {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "No positive response from server");
      GWEN_DB_Group_free(dbResponse);
      GWEN_Buffer_free(tbuf);
      AB_Job_SetStatus(j, AB_Job_StatusError);
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "No ACK response from server");
    GWEN_DB_Group_free(dbResponse);
    GWEN_Buffer_free(tbuf);
    AB_Job_SetStatus(j, AB_Job_StatusError);
    return GWEN_ERROR_BAD_DATA;
  }

  /* now get the transactions */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponse);
  while(dbCurr) {
    AB_ACCOUNT_STATUS *acst;
    AB_ACCOUNT *a;
    AB_BALANCE *bal;
    GWEN_TIME *t=NULL;
    AB_VALUE *vc;
    const char *p;
    
    DBG_NOTICE(AQPAYPAL_LOGDOMAIN, "Got a balance");
    
    acst=AB_AccountStatus_new();
    
    /* read and parse value */
    p=GWEN_DB_GetCharValue(dbCurr, "L_AMT", 0, 0);
    if (!p)
      return GWEN_ERROR_BAD_DATA;
    vc=AB_Value_fromString(p);
    if (vc==NULL)
      return GWEN_ERROR_BAD_DATA;
    
    /* read currency (if any) */
    p=GWEN_DB_GetCharValue(dbCurr, "L_CURRENCYCODE", 0, "EUR");
    if (p)
      AB_Value_SetCurrency(vc, p);
    
    p=GWEN_DB_GetCharValue(dbResponse, "TIMESTAMP", 0, NULL);
    if (p && *p) {
      t=GWEN_Time_fromUtcString(p, "YYYY-MM-DDThh:mm:ssZ");
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Invalid timespec [%s]", p);
    }
    
    bal=AB_Balance_new(vc, t);
    
    AB_Value_free(vc);
    GWEN_Time_free(t);
    
    AB_AccountStatus_SetBookedBalance(acst, bal);
    AB_AccountStatus_SetTime(acst, AB_Balance_GetTime(bal));
    AB_Balance_free(bal);
    
    a=AB_Job_GetAccount(j);
    assert(a);

    /* add new account status */
    AB_ImExporterAccountInfo_AddAccountStatus(ai, acst);
    break; /* break loop, we found the balance */

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  GWEN_DB_Group_free(dbResponse);
  GWEN_Buffer_free(tbuf);
  AB_Job_SetStatus(j, AB_Job_StatusFinished);
  return 0;
}



int APY_Provider_ExecJobQueue(AB_PROVIDER *pro,
                              AB_IMEXPORTER_ACCOUNTINFO *ai,
			      AB_USER *u,
			      AB_ACCOUNT *a,
			      AB_JOBQUEUE *jq) {
  APY_PROVIDER *xp;
  AB_JOB_LIST2_ITERATOR *it;
  int errors=0;
  int oks=0;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  it=AB_Job_List2_First(AB_JobQueue_GetJobList(jq));
  if (it) {
    AB_JOB *j;

    j=AB_Job_List2Iterator_Data(it);
    while(j) {
      int rv=0;

      switch(AB_Job_GetType(j)) {
      case AB_Job_TypeGetTransactions:
	rv=APY_Provider_ExecGetTrans(pro, ai, u, j);
	break;
      case AB_Job_TypeGetBalance:
	rv=APY_Provider_ExecGetBal(pro, ai, u, j);
	break;
      case AB_Job_TypeTransfer:
      case AB_Job_TypeDebitNote:
      default:
	DBG_INFO(AQPAYPAL_LOGDOMAIN,
		 "Job not supported (%d)",
		 AB_Job_GetType(j));
	rv=GWEN_ERROR_NOT_SUPPORTED;
      } /* switch */

      if (rv<0) {
	errors++;
      }
      else
	oks++;

      j=AB_Job_List2Iterator_Next(it);
    }

    AB_Job_List2Iterator_free(it);
  }

  if (errors) {
    if (oks) {
      DBG_WARN(AQPAYPAL_LOGDOMAIN, "Some jobs failed");
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "All jobs failed");
      return GWEN_ERROR_GENERIC;
    }
  }

  return 0;
}




int APY_Provider_ExecAccountQueue(AB_PROVIDER *pro,
				  AB_IMEXPORTER_CONTEXT *ctx,
				  AB_USER *u,
				  AB_ACCOUNTQUEUE *aq) {
  APY_PROVIDER *xp;
  AB_JOBQUEUE *jq;
  AB_ACCOUNT *a;
  int errors=0;
  int oks=0;
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  a=AB_AccountQueue_GetAccount(aq);
  assert(a);

  ai=AB_ImExporterContext_GetAccountInfo(ctx,
                                         AB_Account_GetBankCode(a),
					 AB_Account_GetAccountNumber(a));
  if (ai==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not create account info");
    return GWEN_ERROR_GENERIC;
  }

  jq=AB_JobQueue_List_First(AB_AccountQueue_GetJobQueueList(aq));
  while(jq) {
    int rv;

    rv=APY_Provider_ExecJobQueue(pro, ai, u, a, jq);
    if (rv<0)
      errors++;
    else
      oks++;
    jq=AB_JobQueue_List_Next(jq);
  }

  if (errors) {
    if (oks) {
      DBG_WARN(AQPAYPAL_LOGDOMAIN, "Some jobs failed");
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "All jobs failed");
      return GWEN_ERROR_GENERIC;
    }
  }

  return 0;
}



int APY_Provider_ExecUserQueue(AB_PROVIDER *pro,
			       AB_IMEXPORTER_CONTEXT *ctx,
			       AB_USERQUEUE *uq) {
  APY_PROVIDER *xp;
  AB_ACCOUNTQUEUE *aq;
  AB_USER *u;
  int errors=0;
  int oks=0;
  GWEN_BUFFER *xbuf;
  int rv;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  u=AB_UserQueue_GetUser(uq);
  assert(u);

  /* lock user */
  rv=AB_Banking_BeginExclUseUser(AB_Provider_GetBanking(pro), u);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* read secrets */
  xbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=APY_Provider_ReadUserApiSecrets(pro, u, xbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  else {
    char *t;
    char *t2=NULL;
    GWEN_BUFFER *sbuf1;
    GWEN_BUFFER *sbuf2;
    GWEN_BUFFER *sbuf3;

    t=strchr(GWEN_Buffer_GetStart(xbuf), ':');
    if (t) {
      *(t++)=0;
      t2=strchr(t, ':');
      if (t2) {
	*(t2++)=0;
      }
    }

    sbuf1=GWEN_Buffer_new(0, 256, 0, 1);
    sbuf2=GWEN_Buffer_new(0, 256, 0, 1);
    sbuf3=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Text_UnescapeToBufferTolerant(GWEN_Buffer_GetStart(xbuf), sbuf1);
    if (t) {
      GWEN_Text_UnescapeToBufferTolerant(t, sbuf2);
      t=GWEN_Buffer_GetStart(sbuf2);
      if (t2) {
	GWEN_Text_UnescapeToBufferTolerant(t2, sbuf3);
      }
    }
    APY_User_SetApiSecrets_l(u, GWEN_Buffer_GetStart(sbuf1), GWEN_Buffer_GetStart(sbuf2), GWEN_Buffer_GetStart(sbuf3));
    GWEN_Buffer_free(xbuf);
    GWEN_Buffer_free(sbuf3);
    GWEN_Buffer_free(sbuf2);
    GWEN_Buffer_free(sbuf1);
  }

  aq=AB_AccountQueue_List_First(AB_UserQueue_GetAccountQueueList(uq));
  while(aq) {
    int rv;

    rv=APY_Provider_ExecAccountQueue(pro, ctx, u, aq);
    if (rv<0)
      errors++;
    else
      oks++;
    aq=AB_AccountQueue_List_Next(aq);
  }

  /* erase secrets */
  APY_User_SetApiSecrets_l(u, NULL, NULL, NULL);

  /* unlock user */
  rv=AB_Banking_EndExclUseUser(AB_Provider_GetBanking(pro), u, 0);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_EndExclUseUser(AB_Provider_GetBanking(pro), u, 1);
    return rv;
  }

  if (errors) {
    if (oks) {
      DBG_WARN(AQPAYPAL_LOGDOMAIN, "Some jobs failed");
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "All jobs failed");
      return GWEN_ERROR_GENERIC;
    }
  }

  return 0;
}



int APY_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx){
  APY_PROVIDER *xp;
  AB_USERQUEUE *uq;
  int errors=0;
  int oks=0;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  if (xp->queue==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Empty queue");
    return GWEN_ERROR_NOT_FOUND;
  }

  uq=AB_UserQueue_List_First(AB_Queue_GetUserQueueList(xp->queue));
  while(uq) {
    int rv;

    rv=APY_Provider_ExecUserQueue(pro, ctx, uq);
    if (rv<0)
      errors++;
    else
      oks++;
    uq=AB_UserQueue_List_Next(uq);
  }

  if (errors) {
    if (oks) {
      DBG_WARN(AQPAYPAL_LOGDOMAIN, "Some jobs failed");
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "All jobs failed");
      return GWEN_ERROR_GENERIC;
    }
  }

  return 0;
}



static int readFile(const char *fname, GWEN_BUFFER *dbuf) {
  FILE *f;

  f=fopen(fname, "rb");
  if (f) {
    while(!feof(f)) {
      uint32_t l;
      ssize_t s;
      char *p;

      GWEN_Buffer_AllocRoom(dbuf, 1024);
      l=GWEN_Buffer_GetMaxUnsegmentedWrite(dbuf);
      p=GWEN_Buffer_GetPosPointer(dbuf);
      s=fread(p, 1, l, f);
      if (s==0)
	break;
      if (s==(ssize_t)-1) {
	DBG_ERROR(AQPAYPAL_LOGDOMAIN,
		  "fread(%s): %s",
		  fname, strerror(errno));
	fclose(f);
	return GWEN_ERROR_IO;
      }

      GWEN_Buffer_IncrementPos(dbuf, s);
      GWEN_Buffer_AdjustUsedBytes(dbuf);
    }

    fclose(f);
    return 0;
  }
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN,
	     "fopen(%s): %s",
	     fname, strerror(errno));
    return GWEN_ERROR_IO;
  }
}



static int writeToFile(FILE *f, const char *p, int len) {
  while(len>0) {
    ssize_t l;
    ssize_t s;

    l=1024;
    if (l>len)
      l=len;
    s=fwrite(p, 1, l, f);
    if (s==(ssize_t)-1 || s==0) {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN,
	       "fwrite: %s",
	       strerror(errno));
      return GWEN_ERROR_IO;
    }
    p+=s;
    len-=s;
  }

  return 0;
}



static int writeFile(const char *fname, const char *p, int len) {
  FILE *f;

  f=fopen(fname, "wb");
  if (f) {
    int rv;

    rv=writeToFile(f, p, len);
    if (rv<0) {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
      fclose(f);
      return rv;
    }
    if (fclose(f)) {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "fopen(%s): %s",
	      fname, strerror(errno));
    return GWEN_ERROR_IO;
  }

  return 0;
}




int APY_Provider_ReadUserApiSecrets(AB_PROVIDER *pro, const AB_USER *u, GWEN_BUFFER *secbuf) {
  APY_PROVIDER *xp;
  int rv;
  GWEN_BUFFER *pbuf;
  GWEN_BUFFER *sbuf;
  GWEN_BUFFER *tbuf;
  const char *uid;
  char text[512];
  char pw[129];

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  uid=AB_User_GetUserId(u);
  if (!(uid && *uid)) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "No user id");
    return GWEN_ERROR_INVALID;
  }

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AB_Provider_GetUserDataDir(pro, pbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  GWEN_Buffer_AppendString(pbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Text_UnescapeToBufferTolerant(uid, pbuf);
  GWEN_Buffer_AppendString(pbuf, ".sec");

  sbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readFile(GWEN_Buffer_GetStart(pbuf), sbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(sbuf);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  snprintf(text, sizeof(text)-1,
	   I18N("Please enter the password for \n"
		"Paypal user %s\n"
		"<html>"
		"Please enter the password for Paypal user <i>%s</i></br>"
		"</html>"),
	   uid, uid);
  text[sizeof(text)-1]=0;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, "PASSWORD_");
  GWEN_Text_UnescapeToBufferTolerant(GWEN_Buffer_GetStart(pbuf), tbuf);

  rv=GWEN_Gui_GetPassword(0,
			  GWEN_Buffer_GetStart(tbuf),
			  I18N("Enter Password"),
			  text,
			  pw,
			  4,
                          sizeof(pw)-1,
                          GWEN_Gui_PasswordMethod_Text, NULL,
			  0);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(sbuf);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  rv=GWEN_SmallTresor_Decrypt((const uint8_t*) GWEN_Buffer_GetStart(sbuf),
			      GWEN_Buffer_GetUsedBytes(sbuf),
			      pw,
			      secbuf,
			      AQPAYPAL_PASSWORD_ITERATIONS,
			      AQPAYPAL_CRYPT_ITERATIONS);
  /* overwrite password ASAP */
  memset(pw, 0, sizeof(pw));
  GWEN_Buffer_free(tbuf);
  GWEN_Buffer_free(sbuf);
  GWEN_Buffer_free(pbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int APY_Provider_WriteUserApiSecrets(AB_PROVIDER *pro, const AB_USER *u, const char *sec) {
  APY_PROVIDER *xp;
  int rv;
  GWEN_BUFFER *pbuf;
  GWEN_BUFFER *sbuf;
  GWEN_BUFFER *tbuf;
  const char *uid;
  char text[512];
  char pw[129];

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  uid=AB_User_GetUserId(u);
  if (!(uid && *uid)) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "No user id");
    return GWEN_ERROR_INVALID;
  }

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AB_Provider_GetUserDataDir(pro, pbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  /* make sure the data dir exists */
  DBG_INFO(0, "Looking for [%s]", GWEN_Buffer_GetStart(pbuf));
  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(pbuf), 0);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  GWEN_Buffer_AppendString(pbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Text_UnescapeToBufferTolerant(uid, pbuf);
  GWEN_Buffer_AppendString(pbuf, ".sec");

  snprintf(text, sizeof(text)-1,
	   I18N("Please enter the password for \n"
		"Paypal user %s\n"
		"<html>"
		"Please enter the password for Paypal user <i>%s</i></br>"
		"</html>"),
	   uid, uid);
  text[sizeof(text)-1]=0;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, "PASSWORD_");
  GWEN_Text_UnescapeToBufferTolerant(GWEN_Buffer_GetStart(pbuf), tbuf);

  rv=GWEN_Gui_GetPassword(GWEN_GUI_INPUT_FLAGS_CONFIRM,
			  GWEN_Buffer_GetStart(tbuf),
			  I18N("Enter Password"),
			  text,
			  pw,
			  4,
			  sizeof(pw)-1,
                          GWEN_Gui_PasswordMethod_Text, NULL,
                          0);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  GWEN_Buffer_free(tbuf);

  sbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_SmallTresor_Encrypt((const uint8_t*) sec,
			      strlen(sec),
			      pw,
			      sbuf,
			      AQPAYPAL_PASSWORD_ITERATIONS,
			      AQPAYPAL_CRYPT_ITERATIONS);
  /* overwrite password ASAP */
  memset(pw, 0, sizeof(pw));
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(sbuf);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  /* write file */
  rv=writeFile(GWEN_Buffer_GetStart(pbuf),
	       GWEN_Buffer_GetStart(sbuf),
	       GWEN_Buffer_GetUsedBytes(sbuf));
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(sbuf);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  GWEN_Buffer_free(sbuf);
  GWEN_Buffer_free(pbuf);

  return 0;
}



GWEN_DIALOG *APY_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i) {
  APY_PROVIDER *xp;
  GWEN_DIALOG *dlg;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  dlg=APY_NewUserDialog_new(AB_Provider_GetBanking(pro));
  if (dlg==NULL) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



GWEN_DIALOG *APY_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u) {
  APY_PROVIDER *xp;
  GWEN_DIALOG *dlg;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(xp);

  dlg=APY_EditUserDialog_new(AB_Provider_GetBanking(pro), u, 1);
  if (dlg==NULL) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}
