/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* don't warn about our own deprecated functions */
//#define AQBANKING_NOWARN_DEPRECATED


#include "banking_p.h"
#include "backendsupport/provider_l.h"
#include "backendsupport/imexporter_l.h"
#include "backendsupport/bankinfoplugin_l.h"
#include "i18n_l.h"
#include "banking_dialogs.h"


#include <gwenhywfar/version.h>
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/ctplugin.h>
#include <gwenhywfar/configmgr.h>
#include <gwenhywfar/syncio_file.h>

#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>


#ifdef OS_WIN32
# define ftruncate chsize
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif

GWEN_INHERIT_FUNCTIONS(AB_BANKING)

#include <aqbanking/error.h>


#include "banking_init.c"
#include "banking_cfg.c"
#include "banking_update.c"

#include "banking_accspec.c"
#include "banking_account.c"
#include "banking_user.c"
#include "banking_transaction.c"

#include "banking_online.c"
#include "banking_imex.c"
#include "banking_bankinfo.c"
#include "banking_dialogs.c"
#include "banking_compat.c"




AB_BANKING *AB_Banking_new(const char *appName,
                           const char *dname,
                           uint32_t extensions)
{
  AB_BANKING *ab;
  GWEN_BUFFER *nbuf;
  char buffer[256];
  int err;

  assert(appName);
  err=GWEN_Init();
  if (err) {
    DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
    abort();
  }
  DBG_INFO(AQBANKING_LOGDOMAIN,
           "Application \"%s\" compiled with extensions %08x",
           appName, extensions);

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (GWEN_Text_EscapeToBufferTolerant(appName, nbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Bad application name, aborting.");
    GWEN_Buffer_free(nbuf);
    abort();
  }
  else {
    char *s;

    s=GWEN_Buffer_GetStart(nbuf);
    while (*s) {
      *s=tolower(*s);
      s++;
    }
  }

  GWEN_NEW_OBJECT(AB_BANKING, ab);
  GWEN_INHERIT_INIT(AB_BANKING, ab);
  ab->appEscName=strdup(GWEN_Buffer_GetStart(nbuf));
  ab->appName=strdup(appName);
  ab->cryptTokenList=GWEN_Crypt_Token_List2_new();
  ab->dbRuntimeConfig=GWEN_DB_Group_new("runtimeConfig");

  GWEN_Buffer_free(nbuf);

  AB_Banking__GetConfigManager(ab, dname);

  ab->appExtensions=extensions;

  if (getcwd(buffer, sizeof(buffer)-1)==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "getcwd: %s", strerror(errno));
  }
  else {
    struct stat st;

    if (stat(buffer, &st)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "stat(%s): %s", buffer, strerror(errno));
    }
    else {
      ab->startFolder=strdup(buffer);
    }
  }

  return ab;
}



void AB_Banking_free(AB_BANKING *ab)
{
  if (ab) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Destroying AB_BANKING");

    GWEN_INHERIT_FINI(AB_BANKING, ab);

    GWEN_DB_Group_free(ab->dbRuntimeConfig);
    AB_Banking_ClearCryptTokenList(ab);
    GWEN_Crypt_Token_List2_free(ab->cryptTokenList);
    GWEN_ConfigMgr_free(ab->configMgr);
    free(ab->startFolder);
    free(ab->appName);
    free(ab->appEscName);
    free(ab->dataDir);
    GWEN_FREE_OBJECT(ab);
    GWEN_Fini();
  }
}



int AB_Banking_GetNamedUniqueId(AB_BANKING *ab, const char *idName, int startAtStdUniqueId)
{
  int rv;
  int uid=0;
  GWEN_DB_NODE *dbConfig=NULL;

  rv=GWEN_ConfigMgr_LockGroup(ab->configMgr,
                              AB_CFG_GROUP_MAIN,
                              "uniqueId");
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to lock main config (%d)", rv);
    return rv;
  }

  rv=GWEN_ConfigMgr_GetGroup(ab->configMgr,
                             AB_CFG_GROUP_MAIN,
                             "uniqueId",
                             &dbConfig);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to read main config (%d)", rv);
    return rv;
  }

  if (idName && *idName) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(tbuf, "uniqueid-");
    GWEN_Buffer_AppendString(tbuf, idName);

    uid=GWEN_DB_GetIntValue(dbConfig, GWEN_Buffer_GetStart(tbuf), 0, 0);
    if (uid==0 && startAtStdUniqueId) {
      /* not set yet, start with a unique id from standard source */
      uid=GWEN_DB_GetIntValue(dbConfig, "uniqueId", 0, 0);
      uid++;
      GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_OVERWRITE_VARS, "uniqueId", uid);
      GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_OVERWRITE_VARS, GWEN_Buffer_GetStart(tbuf), uid);
    }
    else {
      uid++;
      GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_OVERWRITE_VARS, GWEN_Buffer_GetStart(tbuf), uid);
    }
    GWEN_Buffer_free(tbuf);
  }
  else {
    uid=GWEN_DB_GetIntValue(dbConfig, "uniqueId", 0, 0);
    uid++;
    GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_OVERWRITE_VARS, "uniqueId", uid);
  }

  rv=GWEN_ConfigMgr_SetGroup(ab->configMgr,
                             AB_CFG_GROUP_MAIN,
                             "uniqueId",
                             dbConfig);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to write main config (%d)", rv);
    GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
                               AB_CFG_GROUP_MAIN,
                               "uniqueId");
    GWEN_DB_Group_free(dbConfig);
    return rv;
  }
  GWEN_DB_Group_free(dbConfig);

  /* unlock */
  rv=GWEN_ConfigMgr_UnlockGroup(ab->configMgr,
                                AB_CFG_GROUP_MAIN,
                                "uniqueId");
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unable to unlock main config (%d)", rv);
    return rv;
  }

  return uid;
}



GWEN_CONFIGMGR *AB_Banking_GetConfigMgr(AB_BANKING *ab)
{
  assert(ab);
  return ab->configMgr;
}



const char *AB_Banking_GetAppName(const AB_BANKING *ab)
{
  assert(ab);
  return ab->appName;
}



const char *AB_Banking_GetEscapedAppName(const AB_BANKING *ab)
{
  assert(ab);
  return ab->appEscName;
}



void AB_Banking_GetVersion(int *major,
                           int *minor,
                           int *patchlevel,
                           int *build)
{
  if (major)
    *major=AQBANKING_VERSION_MAJOR;
  if (minor)
    *minor=AQBANKING_VERSION_MINOR;
  if (patchlevel)
    *patchlevel=AQBANKING_VERSION_PATCHLEVEL;
  if (build)
    *build=AQBANKING_VERSION_BUILD;
}



void AB_Banking_RuntimeConfig_SetCharValue(AB_BANKING *ab, const char *varName, const char *value)
{
  assert(ab);
  assert(ab->dbRuntimeConfig);
  GWEN_DB_SetCharValue(ab->dbRuntimeConfig, GWEN_DB_FLAGS_OVERWRITE_VARS, varName, value);
}



const char *AB_Banking_RuntimeConfig_GetCharValue(const AB_BANKING *ab, const char *varName, const char *defaultValue)
{
  assert(ab);
  assert(ab->dbRuntimeConfig);
  return GWEN_DB_GetCharValue(ab->dbRuntimeConfig, varName, 0, defaultValue);
}



void AB_Banking_RuntimeConfig_SetIntValue(AB_BANKING *ab, const char *varName, int value)
{
  assert(ab);
  assert(ab->dbRuntimeConfig);
  GWEN_DB_SetIntValue(ab->dbRuntimeConfig, GWEN_DB_FLAGS_OVERWRITE_VARS, varName, value);
}



int AB_Banking_RuntimeConfig_GetIntValue(const AB_BANKING *ab, const char *varName, int defaultValue)
{
  assert(ab);
  assert(ab->dbRuntimeConfig);
  return GWEN_DB_GetIntValue(ab->dbRuntimeConfig, varName, 0, defaultValue);
}




GWEN_DIALOG *AB_Banking_GetNewUserDialog(AB_BANKING *ab,
                                         const char *backend,
                                         int mode)
{
  /*
    AB_PROVIDER *pro;
    GWEN_DIALOG *dlg;

    assert(ab);
    pro=AB_Banking_GetProvider(ab, backend);
    if (!pro) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Backend \"%s\" not found", backend);
      return NULL;
    }

    dlg=AB_Provider_GetNewUserDialog(pro, mode);
    if (dlg==NULL) {
      DBG_INFO(AQBANKING_LOGDOMAIN,
         "Provider did not return a NewUser dialog (backend=%s, mode=%d)",
         backend, mode);
      return NULL;
    }

    return dlg;
  */
  return NULL;
}






