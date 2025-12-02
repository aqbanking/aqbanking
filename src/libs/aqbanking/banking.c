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
#include <stdarg.h>


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



static void _logMsgForJobId(const AB_BANKING *ab, uint32_t jobId, const char *msg);




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

  {
    int rv;

    rv=AB_Banking_CopyOldSettingsFolderIfNeeded(ab);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not copy old settings folder (%d), ignoring", rv);
    }
  }

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


#if 0
GWEN_CONFIGMGR *AB_Banking_GetConfigMgr(AB_BANKING *ab)
{
  assert(ab);
  return ab->configMgr;
}
#endif


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



void AB_Banking_LogMsgForJobId(const AB_BANKING *ab, uint32_t jobId, const char *fmt, ...)
{
  if (jobId>0) {
    GWEN_BUFFER *bf;
    va_list list;
    char *p;
    int maxUnsegmentedWrite;
    int rv;

    bf=GWEN_Buffer_new(0, 256, 0, 1);

    maxUnsegmentedWrite=GWEN_Buffer_GetMaxUnsegmentedWrite(bf);
    p=GWEN_Buffer_GetStart(bf)+GWEN_Buffer_GetPos(bf);

    /* prepare list for va_arg */
    va_start(list, fmt);
    rv=vsnprintf(p, maxUnsegmentedWrite, fmt, list);
    if (rv<0) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error on vnsprintf (%d)", rv);
      GWEN_Buffer_free(bf);
      va_end(list);
      return;
    }
    else if (rv>=maxUnsegmentedWrite) {
      GWEN_Buffer_AllocRoom(bf, rv+1);
      maxUnsegmentedWrite=GWEN_Buffer_GetMaxUnsegmentedWrite(bf);
      p=GWEN_Buffer_GetStart(bf)+GWEN_Buffer_GetPos(bf);
      rv=vsnprintf(p, maxUnsegmentedWrite, fmt, list);
      if (rv<0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Error on vnsprintf (%d)", rv);
        GWEN_Buffer_free(bf);
        va_end(list);
        return;
      }
    }
    if (rv>0) {
      GWEN_Buffer_IncrementPos(bf, rv);
      GWEN_Buffer_AdjustUsedBytes(bf);
      _logMsgForJobId(ab, jobId, GWEN_Buffer_GetStart(bf));
    }
    GWEN_Buffer_free(bf);
    va_end(list);
  }
}



void AB_Banking_LogCmdInfoMsgForJob(const AB_BANKING *ab, const AB_TRANSACTION *t, uint32_t jid, const char *msg)
{
  if (jid>0) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    GWEN_Buffer_AppendString(tbuf, msg);
    AB_Banking_AddJobInfoToBuffer(t, tbuf);
    AB_Banking_LogMsgForJobId(ab, jid, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
}



void _logMsgForJobId(const AB_BANKING *ab, uint32_t jobId, const char *msg)
{
  GWEN_BUFFER *pathBuffer;
  int rv;
  FILE *f;

  pathBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(pathBuffer, ab->dataDir);
  GWEN_Buffer_AppendString(pathBuffer, GWEN_DIR_SEPARATOR_S "jobs" GWEN_DIR_SEPARATOR_S);
  GWEN_Buffer_AppendArgs(pathBuffer, "%02x/", (unsigned int)(jobId>>24) & 0xff);
  GWEN_Buffer_AppendArgs(pathBuffer, "%02x/", (unsigned int)(jobId>>16) & 0xff);
  GWEN_Buffer_AppendArgs(pathBuffer, "%02x/", (unsigned int)(jobId>>8) & 0xff);
  GWEN_Buffer_AppendArgs(pathBuffer, "%02x.log", (unsigned int)(jobId & 0xff));
  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(pathBuffer), GWEN_PATH_FLAGS_VARIABLE | GWEN_PATH_FLAGS_CHECKROOT);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error preparing path for logfile \"%s\": %d",
              GWEN_Buffer_GetStart(pathBuffer),
              rv);
    GWEN_Buffer_free(pathBuffer);
    return;
  }

  f=fopen(GWEN_Buffer_GetStart(pathBuffer), "a");
  if (f==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error opening/creating logfile \"%s\": %s",
              GWEN_Buffer_GetStart(pathBuffer),
              strerror(errno));
    GWEN_Buffer_free(pathBuffer);
    return;
  }
  else {
    GWEN_TIME *ti;
    GWEN_BUFFER *tiBuffer;

    ti=GWEN_CurrentTime();
    tiBuffer=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Time_toString(ti, "YYYY/MM/DD-hh:mm:ss", tiBuffer);

    fprintf(f, "%s %s\n", GWEN_Buffer_GetStart(tiBuffer), msg?msg:"<empty>");
    GWEN_Buffer_free(tiBuffer);
    fclose(f);
    GWEN_Buffer_free(pathBuffer);
    GWEN_Time_free(ti);
  }
}



void AB_Banking_Iso8859_1ToUtf8(const char *p, int size, GWEN_BUFFER *buf)
{
  while (*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    if (c!=10) { // keep linefeed as is
      if (c<32 || c==127)
        c=32;
      else {
        /* Dirty hack to support Unicode code points */
        /* U+00A0..U+00FF already in UTF-8 encoding. */
        /* E.g. German Umlaute from Consorsbank      */
        unsigned int c2 = (unsigned char)(*p);
        if ((c & ~0x01)==0xC2 && (c2 & ~0x3F)==0x80) {
          GWEN_Buffer_AppendByte(buf, c);
          c=(unsigned char)(*(p++));
        }
        else if (c & 0x80) {
          GWEN_Buffer_AppendByte(buf, 0xc0 | c>>6);
          c &= ~0x40;
        }
      }
    }
    GWEN_Buffer_AppendByte(buf, c);
    if (size!=-1)
      size--;
  } /* while */
}



