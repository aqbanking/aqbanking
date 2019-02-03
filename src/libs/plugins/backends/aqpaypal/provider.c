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
#include "control/control_l.h"

#include "dlg_newuser_l.h"
#include "dlg_edituser_l.h"

#include <aqbanking/httpsession.h>
#include <aqbanking/transaction.h>
#include <aqbanking/banking_be.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/i18n.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/smalltresor.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/gui.h>

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



AB_PROVIDER *APY_Provider_new(AB_BANKING *ab)
{
  AB_PROVIDER *pro;
  APY_PROVIDER *xp;

  pro=AB_Provider_new(ab, APY_PROVIDER_NAME);
  GWEN_NEW_OBJECT(APY_PROVIDER, xp);
  GWEN_INHERIT_SETDATA(AB_PROVIDER, APY_PROVIDER, pro, xp,
                       APY_Provider_FreeData);


  AB_Provider_SetInitFn(pro, APY_Provider_Init);
  AB_Provider_SetFiniFn(pro, APY_Provider_Fini);

  AB_Provider_SetCreateAccountObjectsFn(pro, APY_Provider_CreateAccountObject);
  AB_Provider_SetCreateUserObjectsFn(pro, APY_Provider_CreateUserObject);

  AB_Provider_SetControlFn(pro, APY_Control);

  AB_Provider_SetGetNewUserDialogFn(pro, APY_Provider_GetNewUserDialog);
  AB_Provider_SetGetEditUserDialogFn(pro, APY_Provider_GetEditUserDialog);

  AB_Provider_AddFlags(pro,
                       AB_PROVIDER_FLAGS_HAS_EDITUSER_DIALOG |
                       AB_PROVIDER_FLAGS_HAS_NEWUSER_DIALOG);

  return pro;
}



void GWENHYWFAR_CB APY_Provider_FreeData(void *bp, void *p)
{
  APY_PROVIDER *xp;

  xp=(APY_PROVIDER *) p;

  GWEN_FREE_OBJECT(xp);
}



int APY_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
{
  APY_PROVIDER *dp;
  const char *logLevelName;
  uint32_t currentVersion;
  uint32_t lastVersion;

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
      DBG_WARN(AQPAYPAL_LOGDOMAIN, "Overriding loglevel for AqPAYPAL with \"%s\"", logLevelName);
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Unknown loglevel \"%s\"", logLevelName);
    }
  }

  /* check whether we need to update */
  currentVersion=
    (AQBANKING_VERSION_MAJOR<<24) |
    (AQBANKING_VERSION_MINOR<<16) |
    (AQBANKING_VERSION_PATCHLEVEL<<8) |
    AQBANKING_VERSION_BUILD;
  lastVersion=GWEN_DB_GetIntValue(dbData, "lastVersion", 0, 0);

  if (lastVersion<currentVersion) {
    int rv;

    DBG_WARN(AQPAYPAL_LOGDOMAIN, "Updating configuration for AqPaypal (before init)");
    rv=APY_Provider_UpdatePreInit(pro, lastVersion, currentVersion);
    if (rv<0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* do some init (currently: none needed) */

  /* update post-init */
  if (lastVersion<currentVersion) {
    int rv;

    DBG_WARN(AQPAYPAL_LOGDOMAIN, "Updating configuration for AqPaypal (after init)");
    rv=APY_Provider_UpdatePostInit(pro, lastVersion, currentVersion);
    if (rv<0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }


  if (1) {
    GWEN_STRINGLIST *sl;
    const char *localedir;
    int rv;

    sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME, AB_PM_LOCALEDIR);
    localedir=GWEN_StringList_FirstString(sl);

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



int APY_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
{
  APY_PROVIDER *dp;
  uint32_t currentVersion;

  DBG_NOTICE(AQPAYPAL_LOGDOMAIN, "Deinitializing AqPaypal backend");

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, APY_PROVIDER, pro);
  assert(dp);

  currentVersion=
    (AQBANKING_VERSION_MAJOR<<24) |
    (AQBANKING_VERSION_MINOR<<16) |
    (AQBANKING_VERSION_PATCHLEVEL<<8) |
    AQBANKING_VERSION_BUILD;

  /* save configuration */
  DBG_NOTICE(AQPAYPAL_LOGDOMAIN, "Setting version %08x", currentVersion);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "lastVersion", currentVersion);

  DBG_INFO(AQPAYPAL_LOGDOMAIN, "Deinit done");

  return 0;
}



AB_ACCOUNT *APY_Provider_CreateAccountObject(AB_PROVIDER *pro)
{
  AB_ACCOUNT *a;

  a=AB_Account_new();
  assert(a);
  AB_Account_SetProvider(a, pro);
  AB_Account_SetBackendName(a, APY_PROVIDER_NAME);
  return a;
}



AB_USER *APY_Provider_CreateUserObject(AB_PROVIDER *pro)
{
  return APY_User_new(pro);
}









int APY_Provider_ParseResponse(AB_PROVIDER *pro, const char *s, GWEN_DB_NODE *db)
{
  /* read vars */
  while (*s) {
    GWEN_BUFFER *bName;
    GWEN_BUFFER *bValue;
    const char *p;
    GWEN_DB_NODE *dbT;

    bName=GWEN_Buffer_new(0, 256, 0, 1);
    bValue=GWEN_Buffer_new(0, 256, 0, 1);
    p=s;
    while (*p && *p!='&' && *p!='=')
      p++;
    if (p!=s)
      GWEN_Buffer_AppendBytes(bName, s, (p-s));
    s=p;
    if (*p=='=') {
      s++;
      p=s;
      while (*p && *p!='&')
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
        while (i && isdigit(*t)) {
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


#include "provider_accspec.c"
#include "provider_update.c"
#include "provider_credentials.c"
#include "provider_dialogs.c"
#include "provider_getbalance.c"
#include "provider_getstm.c"
#include "provider_sendcmd.c"



