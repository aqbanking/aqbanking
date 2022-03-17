/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <aqbanking/backendsupport/providerqueue.h>
#include "aqpaypal/provider_accspec.h"
#include "aqpaypal/provider_credentials.h"
#include "aqpaypal/provider_request.h"
#include "aqpaypal/provider_getstm.h"
#include "aqpaypal/provider_getbalance.h"
#include "aqpaypal/provider_sendcmd.h"
#include "aqpaypal/provider_update.h"
#include "aqpaypal/provider_p.h"
#include "aqpaypal/user_l.h"
#include "aqpaypal/control/control_l.h"

#include "aqpaypal/dlg_newuser_l.h"
#include "aqpaypal/dlg_edituser_l.h"

#include <aqbanking/backendsupport/httpsession.h>
#include <aqbanking/types/transaction.h>
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
#include <errno.h>


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)


/*#define DEBUG_PAYPAL */



GWEN_INHERIT(AB_PROVIDER, APY_PROVIDER)



static void GWENHYWFAR_CB _providerFreeData(void *bp, void *p);
static int _providerInit(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int _providerFini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static AB_USER *_createUserObject(AB_PROVIDER *pro);
static GWEN_DIALOG *_getNewUserDialog(AB_PROVIDER *pro, int i);
static GWEN_DIALOG *_getEditUserDialog(AB_PROVIDER *pro, AB_USER *u);



AB_PROVIDER *APY_Provider_new(AB_BANKING *ab)
{
  AB_PROVIDER *pro;
  APY_PROVIDER *xp;

  pro=AB_Provider_new(ab, APY_PROVIDER_NAME);
  GWEN_NEW_OBJECT(APY_PROVIDER, xp);
  GWEN_INHERIT_SETDATA(AB_PROVIDER, APY_PROVIDER, pro, xp, _providerFreeData);


  AB_Provider_SetInitFn(pro, _providerInit);
  AB_Provider_SetFiniFn(pro, _providerFini);

  AB_Provider_SetCreateUserObjectsFn(pro, _createUserObject);

  AB_Provider_SetUpdateAccountSpecFn(pro, APY_Provider_UpdateAccountSpec);
  AB_Provider_SetControlFn(pro, APY_Control);
  AB_Provider_SetSendCommandsFn(pro, APY_Provider_SendCommands);

  AB_Provider_SetGetNewUserDialogFn(pro, _getNewUserDialog);
  AB_Provider_SetGetEditUserDialogFn(pro, _getEditUserDialog);

  AB_Provider_AddFlags(pro,
                       AB_PROVIDER_FLAGS_HAS_EDITUSER_DIALOG |
                       AB_PROVIDER_FLAGS_HAS_NEWUSER_DIALOG);

  return pro;
}



void GWENHYWFAR_CB _providerFreeData(void *bp, void *p)
{
  APY_PROVIDER *xp;

  xp=(APY_PROVIDER *) p;
  GWEN_FREE_OBJECT(xp);
}



int _providerInit(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
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



int _providerFini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
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



AB_USER *_createUserObject(AB_PROVIDER *pro)
{
  return APY_User_new(pro);
}



GWEN_DIALOG *_getNewUserDialog(AB_PROVIDER *pro, int i)
{
  GWEN_DIALOG *dlg;

  dlg=APY_NewUserDialog_new(pro);
  if (dlg==NULL) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



GWEN_DIALOG *_getEditUserDialog(AB_PROVIDER *pro, AB_USER *u)
{
  GWEN_DIALOG *dlg;

  dlg=APY_EditUserDialog_new(pro, u, 1);
  if (dlg==NULL) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}





