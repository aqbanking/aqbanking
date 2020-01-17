/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define AO_PROVIDER_HEAVY_DEBUG

#include "provider_p.h"

#include "aqofxconnect/account.h"
#include "aqofxconnect/user.h"
#include "aqofxconnect/dialogs/dlg_edituser_l.h"
#include "aqofxconnect/dialogs/dlg_newuser_l.h"
#include "aqofxconnect/control/control.h"
#include "aqofxconnect/v2/r_statements.h"
#include "aqofxconnect/v2/r_accounts.h"

#include <aqbanking/backendsupport/account.h>
#include <aqbanking/types/transaction.h>
#include <aqbanking/types/value.h>
#include <aqbanking/backendsupport/httpsession.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/process.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/i18n.h>
#include <gwenhywfar/url.h>

#include <errno.h>


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18S(msg) msg




GWEN_INHERIT(AB_PROVIDER, AO_PROVIDER)




static AO_APPINFO _appInfos[]= {
  /* got this list from https://microsoftmoneyoffline.wordpress.com/appid-appver/ */
  { I18S("Intuit Quicken Windows 2013"),    "QWIN",       "2200"},
  { I18S("Intuit Quicken Windows 2012"),    "QWIN",       "2100"},
  { I18S("Intuit Quicken Windows 2011"),    "QWIN",       "2000"},
  { I18S("Intuit Quicken Windows 2010"),    "QWIN",       "1900"},
  { I18S("Intuit Quicken Windows 2009"),    "QWIN",       "1800"},
  { I18S("Intuit Quicken Windows 2008"),    "QWIN",       "1700"},
  { I18S("Intuit Quicken Windows 2007"),    "QWIN",       "1600"},
  { I18S("Intuit Quicken Windows 2006"),    "QWIN",       "1500"},
  { I18S("Intuit Quicken Windows 2005"),    "QWIN",       "1400"},

  { I18S("Intuit Quicken Mac 2008"),        "QMOFX",      "1700"},
  { I18S("Intuit Quicken Mac 2007"),        "QMOFX",      "1600"},
  { I18S("Intuit Quicken Mac 2006"),        "QMOFX",      "1500"},
  { I18S("Intuit Quicken Mac 2005"),        "QMOFX",      "1400"},

  { I18S("Intuit QuickBooks Windows 2008"), "QBW",        "1800"},
  { I18S("Intuit QuickBooks Windows 2007"), "QBW",        "1700"},
  { I18S("Intuit QuickBooks Windows 2006"), "QBW",        "1600"},
  { I18S("Intuit QuickBooks Windows 2005"), "QBW",        "1500"},

  { I18S("Microsoft Money Plus"),           "Money Plus", "1700"},
  { I18S("Microsoft Money 2007"),           "Money",      "1600"},
  { I18S("Microsoft Money 2006"),           "Money",      "1500"},
  { I18S("Microsoft Money 2005"),           "Money",      "1400"},
  { I18S("Microsoft Money 2004"),           "Money",      "1200"},
  { I18S("Microsoft Money 2003"),           "Money",      "1100"},

  { I18S("ProSaldo Money 2013"),            "PROSALDO",   "11005"},

  { NULL, NULL, NULL}
};





AB_PROVIDER *AO_Provider_new(AB_BANKING *ab)
{
  AB_PROVIDER *pro;
  AO_PROVIDER *dp;

  pro=AB_Provider_new(ab, "aqofxconnect");
  GWEN_NEW_OBJECT(AO_PROVIDER, dp);
  GWEN_INHERIT_SETDATA(AB_PROVIDER, AO_PROVIDER, pro, dp,
                       AO_Provider_FreeData);

  AB_Provider_SetInitFn(pro, AO_Provider_Init);
  AB_Provider_SetFiniFn(pro, AO_Provider_Fini);

  AB_Provider_SetSendCommandsFn(pro, AO_Provider_SendCommands);
  AB_Provider_SetCreateAccountObjectsFn(pro, AO_Provider_CreateAccountObject);
  AB_Provider_SetCreateUserObjectsFn(pro, AO_Provider_CreateUserObject);

  AB_Provider_SetGetEditUserDialogFn(pro, AO_Provider_GetEditUserDialog);
  AB_Provider_AddFlags(pro, AB_PROVIDER_FLAGS_HAS_EDITUSER_DIALOG);

  AB_Provider_SetCreateAccountObjectsFn(pro, AO_Provider_CreateAccountObject);
  AB_Provider_SetCreateUserObjectsFn(pro, AO_Provider_CreateUserObject);

  AB_Provider_SetUpdateAccountSpecFn(pro, AO_Provider_UpdateAccountSpec);

  AB_Provider_SetControlFn(pro, AO_Control);

  AB_Provider_SetGetNewUserDialogFn(pro, AO_Provider_GetNewUserDialog);
  AB_Provider_AddFlags(pro, AB_PROVIDER_FLAGS_HAS_NEWUSER_DIALOG);

  return pro;
}



void GWENHYWFAR_CB AO_Provider_FreeData(void *bp, void *p)
{
  AO_PROVIDER *dp;

  dp=(AO_PROVIDER *)p;
  assert(dp);

  GWEN_FREE_OBJECT(dp);
}



int AO_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
{
  AO_PROVIDER *dp;
  const char *logLevelName;
  uint32_t currentVersion;
  uint32_t lastVersion;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  /* setup logging */
  if (!GWEN_Logger_IsOpen(AQOFXCONNECT_LOGDOMAIN)) {
    GWEN_Logger_Open(AQOFXCONNECT_LOGDOMAIN, "aqofxconnect", 0, GWEN_LoggerType_Console, GWEN_LoggerFacility_User);
  }

  logLevelName=getenv("AQOFXCONNECT_LOGLEVEL");
  if (logLevelName) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(logLevelName);
    if (ll!=GWEN_LoggerLevel_Unknown) {
      GWEN_Logger_SetLevel(AQOFXCONNECT_LOGDOMAIN, ll);
      DBG_WARN(AQOFXCONNECT_LOGDOMAIN, "Overriding loglevel for AqOFXConnect with \"%s\"", logLevelName);
    }
    else {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Unknown loglevel \"%s\"", logLevelName);
    }
  }

  DBG_NOTICE(AQOFXCONNECT_LOGDOMAIN, "Initializing AqOfxConnect backend");

  dp->dbConfig=dbData;


  /* check whether we need to update */
  currentVersion=
    (AQBANKING_VERSION_MAJOR<<24) |
    (AQBANKING_VERSION_MINOR<<16) |
    (AQBANKING_VERSION_PATCHLEVEL<<8) |
    AQBANKING_VERSION_BUILD;
  lastVersion=GWEN_DB_GetIntValue(dbData, "lastVersion", 0, 0);

  if (lastVersion<currentVersion) {
    int rv;

    DBG_WARN(AQOFXCONNECT_LOGDOMAIN, "Updating configuration for AqOfxConnect (before init)");
    rv=AO_Provider_UpdatePreInit(pro, lastVersion, currentVersion);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* init */
  dp->lastJobId=GWEN_DB_GetIntValue(dp->dbConfig, "lastJobId", 0, 0);
  dp->connectTimeout=GWEN_DB_GetIntValue(dp->dbConfig, "connectTimeout", 0, AO_PROVIDER_CONNECT_TIMEOUT);
  dp->sendTimeout=GWEN_DB_GetIntValue(dp->dbConfig, "sendTimeout", 0, AO_PROVIDER_SEND_TIMEOUT);
  dp->recvTimeout=GWEN_DB_GetIntValue(dp->dbConfig, "recvTimeout", 0, AO_PROVIDER_RECV_TIMEOUT);

  /* update post-init */
  if (lastVersion<currentVersion) {
    int rv;

    DBG_WARN(AQOFXCONNECT_LOGDOMAIN, "Updating configuration for AqOfxConnect (after init)");
    rv=AO_Provider_UpdatePostInit(pro, lastVersion, currentVersion);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }


  return 0;
}



int AO_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
{
  AO_PROVIDER *dp;
  uint32_t currentVersion;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Deinitializing AqOFXDC backend");

  currentVersion=
    (AQBANKING_VERSION_MAJOR<<24) |
    (AQBANKING_VERSION_MINOR<<16) |
    (AQBANKING_VERSION_PATCHLEVEL<<8) |
    AQBANKING_VERSION_BUILD;

  /* save version */
  DBG_NOTICE(AQOFXCONNECT_LOGDOMAIN, "Setting version %08x", currentVersion);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "lastVersion", currentVersion);

  /* save vars */
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "lastJobId", dp->lastJobId);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "connectTimeout", dp->connectTimeout);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "sendTimeout", dp->sendTimeout);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "recvTimeout", dp->recvTimeout);

  dp->dbConfig=0;

  DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Deinit done");

  return 0;
}



AB_ACCOUNT *AO_Provider_CreateAccountObject(AB_PROVIDER *pro)
{
  return AO_Account_new(pro);
}



AB_USER *AO_Provider_CreateUserObject(AB_PROVIDER *pro)
{
  return AO_User_new(pro);
}




GWEN_DIALOG *AO_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u)
{
  AO_PROVIDER *xp;
  GWEN_DIALOG *dlg;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(xp);

  dlg=AO_EditUserDialog_new(pro, u, 1);
  if (dlg==NULL) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



GWEN_DIALOG *AO_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i)
{
  AO_PROVIDER *xp;
  GWEN_DIALOG *dlg;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(xp);

  dlg=AO_NewUserDialog_new(pro);
  if (dlg==NULL) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



const AO_APPINFO *AO_Provider_GetAppInfos(AB_PROVIDER *pro)
{
  return _appInfos;
}



int AO_Provider_GetCert(AB_PROVIDER *pro, AB_USER *u)
{
  AO_PROVIDER *xp;
  int rv;
  const char *url;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(xp);

  url=AO_User_GetServerAddr(u);
  if (url && *url) {
    uint32_t uFlags;
    uint32_t hFlags=0;
    uint32_t pid;

    uFlags=AO_User_GetFlags(u);
    if (uFlags & AO_USER_FLAGS_FORCE_SSL3)
      hFlags|=GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3;

    pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_EMBED |
                               GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                               GWEN_GUI_PROGRESS_SHOW_ABORT,
                               I18N("Getting Certificate"),
                               I18N("We are now asking the server for its "
                                    "SSL certificate"),
                               GWEN_GUI_PROGRESS_NONE,
                               0);

    rv=AB_Banking_GetCert(AB_Provider_GetBanking(pro),
                          url,
                          "https", 443, &hFlags, pid);
    if (rv<0) {
      GWEN_Gui_ProgressEnd(pid);
      return rv;
    }

    if (hFlags & GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Setting ForceSSLv3 flag");
      uFlags|=AO_USER_FLAGS_FORCE_SSL3;
    }
    else {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Clearing ForceSSLv3 flag");
      uFlags&=~AO_USER_FLAGS_FORCE_SSL3;
    }
    AO_User_SetFlags(u, uFlags);
    GWEN_Gui_ProgressEnd(pid);
    return 0;
  }
  else {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "No url");
    return GWEN_ERROR_INVALID;
  }
}



int AO_Provider_RequestStatements(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j,
                                  AB_IMEXPORTER_CONTEXT *ictx)
{
  int rv;

  rv=AO_V2_RequestStatements(pro, u, a, j, ictx);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Error adding request element (%d)", rv);
    return rv;
  }

  return 0;
}



int AO_Provider_RequestAccounts(AB_PROVIDER *pro, AB_USER *u, int keepOpen)
{
  AO_PROVIDER *dp;
  int rv;
  uint32_t pid;
  AB_IMEXPORTER_CONTEXT *ictx;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS |
                             GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                             GWEN_GUI_PROGRESS_SHOW_LOG |
                             GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
                             (keepOpen?GWEN_GUI_PROGRESS_KEEP_OPEN:0) |
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
                             0);

  ictx=AB_ImExporterContext_new();
  rv=AO_V2_RequestAccounts(pro, u, ictx);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Gui_ProgressEnd(pid);
    AB_ImExporterContext_free(ictx);
    return rv;
  }

  /* create accounts */
  rv=AO_Provider__ProcessImporterContext(pro, u, ictx);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Error importing accounts (%d)", rv);
    GWEN_Gui_ProgressLog(pid,
                         GWEN_LoggerLevel_Error,
                         I18N("Error importing accounts"));
    AB_ImExporterContext_free(ictx);
    GWEN_Gui_ProgressEnd(pid);
    return rv;
  }


  AB_ImExporterContext_free(ictx);

  GWEN_Gui_ProgressEnd(pid);
  return 0;
}





#include "provider_accspec.c"
#include "provider_sendcmd.c"
#include "provider_update.c"


