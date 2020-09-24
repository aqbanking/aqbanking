/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_p.h"

#include "provider_accspec.h"
#include "provider_dialogs.h"
#include "provider_sendcmd.h"
#include "provider_update.h"
#include "aqhbci_l.h"
#include "account_l.h"
#include "hbci_l.h"
#include "dialog_l.h"
#include "outbox_l.h"
#include "user_l.h"
#include "control_l.h"

#include "message_l.h" /* for test4 */

/*
#include "dlg_newuser_l.h"
#include "dlg_pintan_l.h"
#include "dlg_ddvcard_l.h"
#include "dlg_zkacard_l.h"
#include "dlg_newkeyfile_l.h"
#include "dlg_importkeyfile_l.h"
#include "dlg_edituserpintan_l.h"
#include "dlg_edituserddv_l.h"
#include "dlg_edituserrdh_l.h"
#include "dlg_choose_usertype_l.h"
#include "dlg_editaccount_l.h"
*/

#include "adminjobs_l.h"
#include "aqhbci/banking/user.h"

#include <aqbanking/banking_be.h>
#include <aqbanking/backendsupport/account.h>
#include <aqbanking/backendsupport/provider_be.h>
#include <aqbanking/backendsupport/userqueue.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/ctplugin.h>



#ifdef OS_WIN32
# define AH_PATH_SEP "\\"
#else
# define AH_PATH_SEP "/"
#endif



GWEN_INHERIT(AB_PROVIDER, AH_PROVIDER);


AB_PROVIDER *AH_Provider_new(AB_BANKING *ab, const char *name)
{
  AB_PROVIDER *pro;
  AH_PROVIDER *hp;
  GWEN_BUFFER *pbuf;

  pbuf=0;
  pro=AB_Provider_new(ab, name);
  assert(pro);

  AB_Provider_SetInitFn(pro, AH_Provider_Init);
  AB_Provider_SetFiniFn(pro, AH_Provider_Fini);

  AB_Provider_SetGetNewUserDialogFn(pro, AH_Provider_GetNewUserDialog);
  AB_Provider_SetGetEditUserDialogFn(pro, AH_Provider_GetEditUserDialog);
  AB_Provider_SetGetUserTypeDialogFn(pro, AH_Provider_GetUserTypeDialog);
  AB_Provider_SetGetEditAccountDialogFn(pro, AH_Provider_GetEditAccountDialog);

  AB_Provider_SetSendCommandsFn(pro, AH_Provider_SendCommands);
  AB_Provider_SetCreateAccountObjectsFn(pro, AH_Provider_CreateAccountObject);
  AB_Provider_SetCreateUserObjectsFn(pro, AH_Provider_CreateUserObject);

  AB_Provider_SetUpdateAccountSpecFn(pro, AH_Provider_UpdateAccountSpec);
  AB_Provider_SetControlFn(pro, AH_Control);

  AB_Provider_AddFlags(pro,
                       AB_PROVIDER_FLAGS_HAS_NEWUSER_DIALOG |
                       AB_PROVIDER_FLAGS_HAS_EDITUSER_DIALOG |
                       AB_PROVIDER_FLAGS_HAS_EDITACCOUNT_DIALOG |
                       AB_PROVIDER_FLAGS_HAS_USERTYPE_DIALOG);

  GWEN_NEW_OBJECT(AH_PROVIDER, hp);
  GWEN_INHERIT_SETDATA(AB_PROVIDER, AH_PROVIDER, pro, hp,
                       AH_Provider_FreeData);

  hp->hbci=AH_HBCI_new(pro);
  assert(hp->hbci);
  GWEN_Buffer_free(pbuf);

  hp->dbTempConfig=GWEN_DB_Group_new("tmpConfig");

  return pro;
}



void GWENHYWFAR_CB AH_Provider_FreeData(void *bp, void *p)
{
  AH_PROVIDER *hp;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Destroying AH_PROVIDER");
  hp=(AH_PROVIDER *)p;

  GWEN_DB_Group_free(hp->dbTempConfig);
  AH_HBCI_free(hp->hbci);

  GWEN_FREE_OBJECT(hp);
}



int AH_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
{
  AH_PROVIDER *hp;
  int rv;
  const char *logLevelName;
  uint32_t currentVersion;
  uint32_t lastVersion;

  if (!GWEN_Logger_IsOpen(AQHBCI_LOGDOMAIN)) {
    GWEN_Logger_Open(AQHBCI_LOGDOMAIN, "aqhbci", 0, GWEN_LoggerType_Console, GWEN_LoggerFacility_User);
  }

  logLevelName=getenv("AQHBCI_LOGLEVEL");
  if (logLevelName) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(logLevelName);
    if (ll!=GWEN_LoggerLevel_Unknown) {
      GWEN_Logger_SetLevel(AQHBCI_LOGDOMAIN, ll);
      DBG_WARN(AQHBCI_LOGDOMAIN, "Overriding loglevel for AqHBCI with \"%s\"", logLevelName);
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown loglevel \"%s\"", logLevelName);
    }
  }

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Initializing AqHBCI backend");
  assert(pro);

  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  GWEN_DB_ClearGroup(hp->dbTempConfig, 0);

  /* check whether we need to update */
  currentVersion=
    (AQBANKING_VERSION_MAJOR<<24) |
    (AQBANKING_VERSION_MINOR<<16) |
    (AQBANKING_VERSION_PATCHLEVEL<<8) |
    AQBANKING_VERSION_BUILD;
  lastVersion=GWEN_DB_GetIntValue(dbData, "lastVersion", 0, 0);

  if (lastVersion<currentVersion) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Updating configuration for AqHBCI (before init)");
    rv=AH_Provider_UpdatePreInit(pro, lastVersion, currentVersion);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* HBCI init */
  rv=AH_HBCI_Init(hp->hbci, dbData);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
  }

  if (lastVersion<currentVersion) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Updating configuration for AqHBCI (after init)");
    rv=AH_Provider_UpdatePostInit(pro, lastVersion, currentVersion);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return rv;
}



int AH_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
{
  AH_PROVIDER *hp;
  uint32_t currentVersion;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Deinitializing AqHBCI backend");

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  currentVersion=
    (AQBANKING_VERSION_MAJOR<<24) |
    (AQBANKING_VERSION_MINOR<<16) |
    (AQBANKING_VERSION_PATCHLEVEL<<8) |
    AQBANKING_VERSION_BUILD;

  /* save version */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Setting version %08x", currentVersion);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS, "lastVersion", currentVersion);

  /* save configuration */
  rv=AH_HBCI_Fini(hp->hbci, dbData);
  GWEN_DB_ClearGroup(hp->dbTempConfig, 0);

  return rv;
}



const char *AH_Provider_GetProductName(const AB_PROVIDER *pro)
{
  AH_HBCI *h;

  assert(pro);
  h=AH_Provider_GetHbci(pro);
  assert(h);
  return AH_HBCI_GetProductName(h);
}



const char *AH_Provider_GetProductVersion(const AB_PROVIDER *pro)
{
  AH_HBCI *h;

  assert(pro);
  h=AH_Provider_GetHbci(pro);
  assert(h);
  return AH_HBCI_GetProductVersion(h);
}






AH_HBCI *AH_Provider_GetHbci(const AB_PROVIDER *pro)
{
  AH_PROVIDER *hp;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  return hp->hbci;
}



AB_ACCOUNT *AH_Provider_CreateAccountObject(AB_PROVIDER *pro)
{
  return AH_Account_new(pro);
}



AB_USER *AH_Provider_CreateUserObject(AB_PROVIDER *pro)
{
  return AH_User_new(pro);
}



int AH_Provider_CheckCryptToken(AB_PROVIDER *pro,
                                GWEN_CRYPT_TOKEN_DEVICE devt,
                                GWEN_BUFFER *typeName,
                                GWEN_BUFFER *tokenName)
{
  GWEN_PLUGIN_MANAGER *pm;
  int rv;

  /* get crypt token */
  pm=GWEN_PluginManager_FindPluginManager(GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
  if (pm==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "CryptToken plugin manager not found");
    return GWEN_ERROR_NOT_FOUND;
  }

  /* try to determine the type and name */
  rv=GWEN_Crypt_Token_PluginManager_CheckToken(pm, devt, typeName, tokenName, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}









int AH_Provider_WriteValueToDb(const AB_VALUE *v, GWEN_DB_NODE *dbV)
{
  if (v) {
    GWEN_BUFFER *nbuf;
    char *p;
    const char *s;
    int l;

    nbuf=GWEN_Buffer_new(0, 32, 0, 1);
    AH_Job_ValueToChallengeString(v, nbuf);
    l=GWEN_Buffer_GetUsedBytes(nbuf);
    if (!l) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error in conversion");
      GWEN_Buffer_free(nbuf);
      abort();
    }

    /* replace "C" comma with "DE" comma, remove thousand's comma */
    p=GWEN_Buffer_GetStart(nbuf);
    s=p;
    while (*s) {
      if (*s=='.') {
        *p=',';
        p++;
      }
      else if (*s!=',') {
        *p=*s;
        p++;
      }
      s++;
    } /* while */
    *p=0;

    if (strchr(GWEN_Buffer_GetStart(nbuf), ',')) {
      /* kill all trailing '0' behind the comma */
      p=GWEN_Buffer_GetStart(nbuf)+l;
      while (l--) {
        --p;
        if (*p=='0')
          *p=0;
        else
          break;
      }
    }
    else
      GWEN_Buffer_AppendString(nbuf, ",");

    /* store value */
    GWEN_DB_SetCharValue(dbV, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "value",
                         GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);

    s=AB_Value_GetCurrency(v);
    if (!s)
      s="EUR";
    GWEN_DB_SetCharValue(dbV, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "currency", s);

    return 0;
  } /* if value */
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No value");
    return GWEN_ERROR_NO_DATA;
  }
}



static int AH_Provider_Test4(AB_PROVIDER *pro)
{
#if 0
  AB_BANKING *ab;
  AB_USER *u;
  AH_DIALOG *dlg;
  AH_MSG *msg;
  GWEN_BUFFER *tbuf;

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  tbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  GWEN_Buffer_AppendString(tbuf, "(Test-String)");
  GWEN_Buffer_Rewind(tbuf);

  u=AB_Banking_FindUser(ab, "aqhbci", "de", "20090500", "*", "*");
  assert(u);
  dlg=AH_Dialog_new(u);
  assert(dlg);
  AH_Dialog_AddFlags(dlg, AH_DIALOG_FLAGS_INITIATOR);
  msg=AH_Msg_new(dlg);
  assert(msg);
  AH_Msg_SetBuffer(msg, tbuf);
  AH_Msg_SetHbciVersion(msg, 220);

  AH_Msg_AddSignerId(msg, AB_User_GetUserId(u));
  AH_Msg_SetCrypterId(msg, AB_User_GetUserId(u));

  if (AH_Msg_EncodeMsg(msg)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not decode this message:");
    AH_Msg_Dump(msg, 2);
    return -1;
  }

  fprintf(stderr, "Message is:\n");
  AH_Msg_Dump(msg, 2);
#endif
  return 0;
}



int AH_Provider_Test(AB_PROVIDER *pro)
{
  return AH_Provider_Test4(pro);
}





#include "provider_dtazv.c"



