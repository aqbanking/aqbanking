/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2018 by Martin Preuss
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




