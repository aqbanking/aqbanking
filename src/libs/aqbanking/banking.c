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

#include "banking_p.h"
#include "provider_l.h"
#include "imexporter_l.h"
#include "bankinfoplugin_l.h"
#include "cryptmanager_l.h"
#include "i18n_l.h"
#include "country_l.h"
#include "wcb_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/bio_file.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/md.h>
#include <gwenhywfar/nettransportssl.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

/* includes for high level API */
#include "jobgetbalance.h"
#include "jobgettransactions.h"
#include "jobgetstandingorders.h"
#include "jobsingletransfer.h"
#include "jobsingledebitnote.h"
#include "jobeutransfer.h"
#include "jobgetdatedtransfers.h"


#ifdef OS_WIN32
# define ftruncate chsize
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif


#undef AB_Banking_new


GWEN_INHERIT_FUNCTIONS(AB_BANKING)

#include <aqbanking/error.h>


AB_BANKING *AB_Banking_new(const char *appName, const char *fname){
  return AB_Banking_newExtended(appName, fname, 0);
}



AB_BANKING *AB_Banking_newExtended(const char *appName, const char *fname,
                                   GWEN_TYPE_UINT32 extensions){
  AB_BANKING *ab;
  GWEN_BUFFER *buf;
  GWEN_BUFFER *nbuf;

  assert(appName);

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
    while(*s) {
      *s=tolower(*s);
      s++;
    }
  }

  buf=0;
  if (!fname) {
    char home[256];

    if (GWEN_Directory_GetHomeDirectory(home, sizeof(home))) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not determine home directory, aborting.");
      GWEN_Buffer_free(nbuf);
      abort();
    }
    buf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(buf, home);
    GWEN_Buffer_AppendString(buf, DIRSEP AB_BANKING_CONFIGFILE);
    fname=GWEN_Buffer_GetStart(buf);
  }

  GWEN_NEW_OBJECT(AB_BANKING, ab);
  GWEN_INHERIT_INIT(AB_BANKING, ab);
  ab->providers=AB_Provider_List_new();
  ab->imexporters=AB_ImExporter_List_new();
  ab->bankInfoPlugins=AB_BankInfoPlugin_List_new();
  ab->accounts=AB_Account_List_new();
  ab->enqueuedJobs=AB_Job_List_new();
  ab->appEscName=strdup(GWEN_Buffer_GetStart(nbuf));
  ab->appName=strdup(appName);
  ab->activeProviders=GWEN_StringList_new();
  GWEN_StringList_SetSenseCase(ab->activeProviders, 0);
  ab->data=GWEN_DB_Group_new("BankingData");
  ab->configFile=strdup(fname);
  ab->pinList=AB_Pin_List_new();
  ab->pinCacheEnabled=0;
  GWEN_Buffer_free(buf);
  GWEN_Buffer_free(nbuf);

  ab->dbTempConfig=GWEN_DB_Group_new("tmpConfig");

  GWEN_NetTransportSSL_SetAskAddCertFn2(AB_Banking_AskAddCert, ab);

  DBG_NOTICE(AQBANKING_LOGDOMAIN, "Registering callbacks");
  ab->waitCallback=AB_WaitCallback_new(ab, AB_BANKING_WCB_GENERIC);
  if (GWEN_WaitCallback_Register(ab->waitCallback)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Internal error: Could not register callback.");
    abort();
  }

  ab->appExtensions=extensions;

  return ab;
}



void AB_Banking_free(AB_BANKING *ab){
  if (ab) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Destroying AB_BANKING");

#if 0
    /* This might be enabled to help application programmers to keep
       track of the callbacks that they might have forgotten. */
    if (!ab->messageBoxFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_MESSAGEBOX_FN messageBoxFn was not set during this usage of AB_BANKING.");
    if (!ab->inputBoxFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_INPUTBOX_FN inputBoxFn was not set during this usage of AB_BANKING.");
    if (!ab->showBoxFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_SHOWBOX_FN showBoxFn was not set during this usage of AB_BANKING.");
    if (!ab->hideBoxFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_HIDEBOX_FN hideBoxFn was not set during this usage of AB_BANKING.");
    if (!ab->progressStartFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_PROGRESS_START_FN progressStartFn was not set during this usage of AB_BANKING.");
    if (!ab->progressAdvanceFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_PROGRESS_ADVANCE_FN progressAdvanceFn was not set during this usage of AB_BANKING.");
    if (!ab->progressLogFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_PROGRESS_LOG_FN progressLogFn was not set during this usage of AB_BANKING.");
    if (!ab->progressEndFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_PROGRESS_END_FN progressEndFn was not set during this usage of AB_BANKING.");
    if (!ab->printFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_PRINT_FN printFn was not set during this usage of AB_BANKING.");
    if (!ab->getPinFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_GETPIN_FN getPinFn was not set during this usage of AB_BANKING.");
    if (!ab->setPinStatusFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_SETPINSTATUS_FN setPinStatusFn was not set during this usage of AB_BANKING.");
    if (!ab->getTanFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_GETTAN_FN getTanFn was not set during this usage of AB_BANKING.");
    if (!ab->setTanStatusFn) DBG_WARN(AQBANKING_LOGDOMAIN, "AB_BANKING_SETTANSTATUS_FN setTanStatusFn was not set during this usage of AB_BANKING.");
#endif

    GWEN_INHERIT_FINI(AB_BANKING, ab);

    GWEN_WaitCallback_Unregister(ab->waitCallback);
    GWEN_WaitCallback_free(ab->waitCallback);

    AB_Job_List_free(ab->enqueuedJobs);
    AB_Account_List_free(ab->accounts);
    AB_Provider_List_free(ab->providers);
    AB_BankInfoPlugin_List_free(ab->bankInfoPlugins);
    AB_ImExporter_List_free(ab->imexporters);
    GWEN_StringList_free(ab->activeProviders);
    GWEN_DB_Group_free(ab->data);
    AB_Pin_List_free(ab->pinList);
    GWEN_DB_Group_free(ab->dbTempConfig);
    free(ab->appName);
    free(ab->appEscName);
    free(ab->configFile);
    free(ab->dataDir);
    GWEN_FREE_OBJECT(ab);
  }
}



AB_JOB_LIST2 *AB_Banking_GetEnqueuedJobs(const AB_BANKING *ab){
  AB_JOB_LIST2 *jl;
  AB_JOB *j;

  assert(ab);
  if (AB_Job_List_GetCount(ab->enqueuedJobs)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No jobs");
    return 0;
  }

  jl=AB_Job_List2_new();
  j=AB_Job_List_First(ab->enqueuedJobs);
  assert(j);
  while(j) {
    AB_Job_List2_PushBack(jl, j);
    j=AB_Job_List_Next(j);
  } /* while */

  return jl;
}



GWEN_TYPE_UINT32 AB_Banking_GetUniqueId(AB_BANKING *ab){
  GWEN_BUFFER *nbuf;
  GWEN_TYPE_UINT32 uniqueId;
  int fd;

  assert(ab);
  uniqueId=0;
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (AB_Banking_GetUserDataDir(ab, nbuf)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    GWEN_Buffer_free(nbuf);
    return 0;
  }
  GWEN_Buffer_AppendString(nbuf, DIRSEP "uniqueid");

  fd=AB_Banking__OpenFile(GWEN_Buffer_GetStart(nbuf), 1);
  if (fd!=-1) {
    GWEN_BUFFEREDIO *bio;
    char buffer[256];
    GWEN_ERRORCODE err;
    unsigned long int i;

    buffer[0]=0;
    bio=GWEN_BufferedIO_File_new(fd);
    GWEN_BufferedIO_SubFlags(bio, GWEN_BUFFEREDIO_FLAGS_CLOSE);
    GWEN_BufferedIO_SetReadBuffer(bio, 0, 256);
    if (!GWEN_BufferedIO_CheckEOF(bio)) {
      err=GWEN_BufferedIO_ReadLine(bio, buffer, sizeof(buffer)-1);
      if (!GWEN_Error_IsOk(err)) {
        DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
        GWEN_BufferedIO_free(bio);
        AB_Banking__CloseFile(fd);
        GWEN_Buffer_free(nbuf);
        return 0;
      }
      if (strlen(buffer)) {
        if (1!=sscanf(buffer, "%lu", &i)) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Bad value in file (%s)",
                    buffer);
          GWEN_BufferedIO_free(bio);
          AB_Banking__CloseFile(fd);
          GWEN_Buffer_free(nbuf);
          return 0;
        }
      }
      else
        i=0;
    }
    else {
      DBG_INFO(AQBANKING_LOGDOMAIN, "File is empty");
      i=0;
    }
    GWEN_BufferedIO_free(bio);

    uniqueId=++i;
    buffer[0]=0;
    snprintf(buffer, sizeof(buffer)-1, "%lu", i);
    if (ftruncate(fd, 0)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"ftruncate(%s, 0): %s",
		GWEN_Buffer_GetStart(nbuf), strerror(errno));
      GWEN_BufferedIO_free(bio);
      return 0;
    }
    if (lseek(fd, 0, SEEK_SET)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"lseek(%s, 0): %s",
		GWEN_Buffer_GetStart(nbuf), strerror(errno));
      GWEN_BufferedIO_free(bio);
      return 0;
    }

    bio=GWEN_BufferedIO_File_new(fd);
    GWEN_BufferedIO_SubFlags(bio, GWEN_BUFFEREDIO_FLAGS_CLOSE);
    GWEN_BufferedIO_SetWriteBuffer(bio, 0, 256);
    err=GWEN_BufferedIO_WriteLine(bio, buffer);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
      GWEN_BufferedIO_free(bio);
      AB_Banking__CloseFile(fd);
      GWEN_Buffer_free(nbuf);
      return 0;
    }
    err=GWEN_BufferedIO_Flush(bio);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
      GWEN_BufferedIO_free(bio);
      AB_Banking__CloseFile(fd);
      GWEN_Buffer_free(nbuf);
      return 0;
    }
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Could not open file.");
    uniqueId=1;
  }

  if (AB_Banking__CloseFile(fd)) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Error closing file \"%s\"",
	     GWEN_Buffer_GetStart(nbuf));
    uniqueId=0;
  }

  GWEN_Buffer_free(nbuf);
  return uniqueId;
}



const char *AB_Banking_GetAppName(const AB_BANKING *ab){
  assert(ab);
  return ab->appName;
}



const char *AB_Banking_GetEscapedAppName(const AB_BANKING *ab){
  assert(ab);
  return ab->appEscName;
}


int AB_Banking__GetAppConfigFileName(AB_BANKING *ab, GWEN_BUFFER *buf) {
  if (AB_Banking_GetUserDataDir(ab, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not get user data dir");
    return AB_ERROR_GENERIC;
  }
  GWEN_Buffer_AppendString(buf, DIRSEP "apps" DIRSEP);
  GWEN_Buffer_AppendString(buf, ab->appEscName);
  GWEN_Buffer_AppendString(buf, DIRSEP "settings.conf");
  return 0;
}



int AB_Banking__LoadAppData(AB_BANKING *ab) {
  GWEN_BUFFER *pbuf;
  GWEN_DB_NODE *db;

  assert(ab);
  assert(ab->appEscName);
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (AB_Banking__GetAppConfigFileName(ab, pbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not get config file name");
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }

  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "static/apps");
  assert(db);
  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
                      ab->appEscName);
  assert(db);
  DBG_NOTICE(AQBANKING_LOGDOMAIN,
             "Reading file \"%s\"", GWEN_Buffer_GetStart(pbuf));
  if (GWEN_DB_ReadFile(db, GWEN_Buffer_GetStart(pbuf),
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_CREATE_GROUP |
                       GWEN_DB_FLAGS_LOCKFILE)) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Could not load config file \"%s\", creating it later",
	     GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);
    return 0;
  }
  GWEN_Buffer_free(pbuf);

  /* sucessfully read */
  return 0;
}



int AB_Banking__SaveAppData(AB_BANKING *ab) {
  GWEN_BUFFER *pbuf;
  GWEN_BUFFER *rpbuf;
  GWEN_DB_NODE *db;

  assert(ab);
  assert(ab->appEscName);

  db=GWEN_DB_GetGroup(ab->data, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "static/apps");
  if (!db)
    return 0;
  db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      ab->appEscName);
  if (!db)
    return 0;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (AB_Banking__GetAppConfigFileName(ab, pbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not get config file name");
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }

  DBG_NOTICE(AQBANKING_LOGDOMAIN,
             "Writing file \"%s\"", GWEN_Buffer_GetStart(pbuf));
  if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(pbuf),
			     GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Path \"%s\" is not available",
              GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }
  rpbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(pbuf)+4, 0, 1);
  GWEN_Buffer_AppendBuffer(rpbuf, pbuf);
  GWEN_Buffer_AppendString(pbuf, ".tmp");
  if (GWEN_DB_WriteFile(db, GWEN_Buffer_GetStart(pbuf),
                        GWEN_DB_FLAGS_DEFAULT |
                        GWEN_DB_FLAGS_LOCKFILE)) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Could not save app config file \"%s\"",
	     GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(rpbuf);
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }
#ifdef OS_WIN32
  if (unlink(GWEN_Buffer_GetStart(rpbuf))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not delete old file \"%s\": %s",
              GWEN_Buffer_GetStart(rpbuf),
              strerror(errno));
    GWEN_Buffer_free(rpbuf);
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }
#endif
  if (rename(GWEN_Buffer_GetStart(pbuf),
             GWEN_Buffer_GetStart(rpbuf))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not rename file to \"%s\": %s",
              GWEN_Buffer_GetStart(rpbuf),
              strerror(errno));
    GWEN_Buffer_free(rpbuf);
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }
  GWEN_Buffer_free(rpbuf);
  GWEN_Buffer_free(pbuf);

  /* sucessfully written */
  return 0;
}



GWEN_DB_NODE *AB_Banking_GetAppData(AB_BANKING *ab) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbT;

  assert(ab);
  assert(ab->appEscName);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "static/apps");
  assert(db);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      ab->appEscName);
  if (!dbT) {
    if (AB_Banking__LoadAppData(ab)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load app data file");
      return 0;
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
		       ab->appEscName);
  assert(dbT);
  return dbT;
}



int AB_Banking__GetProviderConfigFileName(AB_BANKING *ab,
					  const char *name,
					  GWEN_BUFFER *buf) {
  if (AB_Banking_GetUserDataDir(ab, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not get user data dir");
    return AB_ERROR_GENERIC;
  }
  GWEN_Buffer_AppendString(buf, DIRSEP "backends" DIRSEP);
  GWEN_Buffer_AppendString(buf, name);
  GWEN_Buffer_AppendString(buf, DIRSEP "settings.conf");
  return 0;
}



int AB_Banking__LoadProviderData(AB_BANKING *ab,
				 const char *name){
  GWEN_BUFFER *pbuf;
  GWEN_DB_NODE *db;

  assert(ab);
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (AB_Banking__GetProviderConfigFileName(ab, name, pbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not get config file name");
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }

  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
		      "static/providers");
  assert(db);
  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, name);
  assert(db);
  DBG_NOTICE(AQBANKING_LOGDOMAIN,
             "Reading file \"%s\"", GWEN_Buffer_GetStart(pbuf));
  if (GWEN_DB_ReadFile(db, GWEN_Buffer_GetStart(pbuf),
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_CREATE_GROUP |
                       GWEN_DB_FLAGS_LOCKFILE)) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Could not load config file \"%s\", creating it later",
	     GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);
    return 0;
  }
  GWEN_Buffer_free(pbuf);

  /* sucessfully read */
  return 0;
}



int AB_Banking__SaveProviderData(AB_BANKING *ab,
                                 const char *name,
				 int del){
  GWEN_BUFFER *pbuf;
  GWEN_BUFFER *rpbuf;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbBackends;

  assert(ab);

  dbBackends=GWEN_DB_GetGroup(ab->data, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			      "static/providers");
  if (!dbBackends)
    return 0;
  db=GWEN_DB_GetGroup(dbBackends, GWEN_PATH_FLAGS_NAMEMUSTEXIST, name);
  if (!db)
    return 0;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (AB_Banking__GetProviderConfigFileName(ab, name, pbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not get config file name");
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }

  DBG_NOTICE(AQBANKING_LOGDOMAIN,
             "Saving file \"%s\"", GWEN_Buffer_GetStart(pbuf));
  if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(pbuf),
			     GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Path \"%s\" is not available",
              GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }

  rpbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(pbuf)+4, 0, 1);
  GWEN_Buffer_AppendBuffer(rpbuf, pbuf);
  GWEN_Buffer_AppendString(pbuf, ".tmp");
  if (GWEN_DB_WriteFile(db, GWEN_Buffer_GetStart(pbuf),
                        GWEN_DB_FLAGS_DEFAULT|GWEN_DB_FLAGS_LOCKFILE)) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Could not save backend config file \"%s\"",
	     GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(rpbuf);
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }
#ifdef OS_WIN32
  if (unlink(GWEN_Buffer_GetStart(rpbuf))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not delete old file \"%s\": %s",
              GWEN_Buffer_GetStart(rpbuf),
              strerror(errno));
    GWEN_Buffer_free(rpbuf);
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }
#endif
  if (rename(GWEN_Buffer_GetStart(pbuf),
             GWEN_Buffer_GetStart(rpbuf))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not rename file to \"%s\": %s",
              GWEN_Buffer_GetStart(rpbuf),
              strerror(errno));
    GWEN_Buffer_free(rpbuf);
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }
  GWEN_Buffer_free(rpbuf);
  GWEN_Buffer_free(pbuf);

  if (del)
    GWEN_DB_DeleteGroup(dbBackends, name);

  /* sucessfully written */
  return 0;
}



GWEN_DB_NODE *AB_Banking_GetProviderData(AB_BANKING *ab,
					 const char *name) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbT;

  assert(ab);
  assert(name);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
		      "static/providers");
  assert(db);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, name);
  if (!dbT) {
    if (AB_Banking__LoadProviderData(ab, name)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load provider data file");
      return 0;
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, name);
  assert(dbT);
  return dbT;
}



void AB_Banking_SetMessageBoxFn(AB_BANKING *ab,
                                AB_BANKING_MESSAGEBOX_FN f){
  assert(ab);
  ab->messageBoxFn=f;
}



void AB_Banking_SetInputBoxFn(AB_BANKING *ab,
                              AB_BANKING_INPUTBOX_FN f){
  assert(ab);
  ab->inputBoxFn=f;
}



void AB_Banking_SetShowBoxFn(AB_BANKING *ab,
                             AB_BANKING_SHOWBOX_FN f){
  assert(ab);
  ab->showBoxFn=f;
}



void AB_Banking_SetHideBoxFn(AB_BANKING *ab,
                             AB_BANKING_HIDEBOX_FN f){
  assert(ab);
  ab->hideBoxFn=f;
}



void AB_Banking_SetProgressStartFn(AB_BANKING *ab,
                                   AB_BANKING_PROGRESS_START_FN f){
  assert(ab);
  ab->progressStartFn=f;
}



void AB_Banking_SetProgressAdvanceFn(AB_BANKING *ab,
                                     AB_BANKING_PROGRESS_ADVANCE_FN f){
  assert(ab);
  ab->progressAdvanceFn=f;
}



void AB_Banking_SetProgressLogFn(AB_BANKING *ab,
                                 AB_BANKING_PROGRESS_LOG_FN f){
  assert(ab);
  ab->progressLogFn=f;
}



void AB_Banking_SetProgressEndFn(AB_BANKING *ab,
                                 AB_BANKING_PROGRESS_END_FN f){
  assert(ab);
  ab->progressEndFn=f;
}



void AB_Banking_SetPrintFn(AB_BANKING *ab,
                           AB_BANKING_PRINT_FN f){
  assert(ab);
  ab->printFn=f;
}








int AB_Banking_MessageBox(AB_BANKING *ab,
                          GWEN_TYPE_UINT32 flags,
                          const char *title,
                          const char *text,
                          const char *b1,
                          const char *b2,
                          const char *b3){
  assert(ab);
  if (ab->messageBoxFn) {
    return ab->messageBoxFn(ab, flags, title, text, b1, b2, b3);
  }
  DBG_INFO(AQBANKING_LOGDOMAIN, "No messageBox function set");
  return 0;
}



int AB_Banking_InputBox(AB_BANKING *ab,
                        GWEN_TYPE_UINT32 flags,
                        const char *title,
                        const char *text,
                        char *buffer,
                        int minLen,
                        int maxLen){
  assert(ab);
  if (ab->inputBoxFn) {
    return ab->inputBoxFn(ab, flags, title, text, buffer, minLen, maxLen);
  }
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No inputBox function set");
  return AB_ERROR_NOFN;
}



GWEN_TYPE_UINT32 AB_Banking_ShowBox(AB_BANKING *ab,
                                    GWEN_TYPE_UINT32 flags,
                                    const char *title,
                                    const char *text){
  assert(ab);
  if (ab->showBoxFn) {
    return ab->showBoxFn(ab, flags, title, text);
  }
  DBG_INFO(AQBANKING_LOGDOMAIN, "No showBox function set");
  return 0;
}



void AB_Banking_HideBox(AB_BANKING *ab, GWEN_TYPE_UINT32 id){
  assert(ab);
  if (ab->hideBoxFn) {
    ab->hideBoxFn(ab, id);
    return;
  }
  DBG_INFO(AQBANKING_LOGDOMAIN, "No hideBox function set");
}



GWEN_TYPE_UINT32 AB_Banking_ProgressStart(AB_BANKING *ab,
                                          const char *title,
                                          const char *text,
                                          GWEN_TYPE_UINT32 total){
  assert(ab);
  if (ab->progressStartFn) {
    GWEN_TYPE_UINT32 pid;

    if (ab->progressNestingLevel>0) {
      /* nesting, check whether the application supports it */
      if (!(ab->appExtensions & AB_BANKING_EXTENSION_NESTING_PROGRESS)) {
        /* nope, app doesn't support nesting, return currently active
         * progress id */
        ab->progressNestingLevel++;
        return ab->lastProgressId;
      }
    }

    pid=ab->progressStartFn(ab, title, text, total);
    if (pid) {
      ab->progressNestingLevel++;
      ab->lastProgressId=pid;
    }
    else {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      ab->lastProgressId=0;
    }
    return pid;
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "No progressStart function set");
    return 0;
  }
}



int AB_Banking_ProgressAdvance(AB_BANKING *ab,
                               GWEN_TYPE_UINT32 id,
                               GWEN_TYPE_UINT32 progress){
  assert(ab);
  if (ab->progressAdvanceFn) {
    return ab->progressAdvanceFn(ab, id, progress);
  }
  DBG_INFO(AQBANKING_LOGDOMAIN, "No progressAdvance function set");
  return 0;
}



int AB_Banking_ProgressLog(AB_BANKING *ab,
                           GWEN_TYPE_UINT32 id,
                           AB_BANKING_LOGLEVEL level,
                           const char *text){
  assert(ab);
  if (ab->progressLogFn)
    return ab->progressLogFn(ab, id, level, text);
  DBG_INFO(AQBANKING_LOGDOMAIN, "No progressLog function set");
  return 0;
}



int AB_Banking_ProgressEnd(AB_BANKING *ab, GWEN_TYPE_UINT32 id){
  assert(ab);
  if (ab->progressEndFn) {
    if (ab->progressNestingLevel<1) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No progress context open");
      return AB_ERROR_INVALID;
    }
    else {
      int rv;

      if (!(ab->appExtensions & AB_BANKING_EXTENSION_NESTING_PROGRESS)) {
        /* app does not support nesting progress */
        if ((ab->progressNestingLevel)>1) {
          /* just count down the nesting level and return */
          ab->progressNestingLevel--;
          return 0;
        }
      }
      rv=ab->progressEndFn(ab, id);
      if (rv==0) {
        ab->progressNestingLevel--;
      }
      ab->lastProgressId=0;
      return rv;
    }
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No progressEnd function set");
    return 0;
  }
}



int AB_Banking_Print(AB_BANKING *ab,
                     const char *docTitle,
                     const char *docType,
                     const char *descr,
                     const char *text){
  assert(ab);
  if (ab->printFn) {
    return ab->printFn(ab, docTitle, docType, descr, text);
  }
  DBG_INFO(AQBANKING_LOGDOMAIN, "No print function set");
  return AB_ERROR_NOT_SUPPORTED;
}




int AB_Banking_InitProvider(AB_BANKING *ab, AB_PROVIDER *pro) {
  return AB_Provider_Init(pro);
}



int AB_Banking_FiniProvider(AB_BANKING *ab, AB_PROVIDER *pro) {
  int rv1;
  int rv2;

  rv1=AB_Provider_Fini(pro);
  rv2=AB_Banking__SaveProviderData(ab,
				   AB_Provider_GetEscapedName(pro),
				   (rv1==0));
  if (rv2) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error saving backend data (%d)", rv2);
    return rv2;
  }
  if (rv1) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error deinit backend (%d)", rv1);
  }
  return rv1;
}



AB_PROVIDER *AB_Banking_FindProvider(AB_BANKING *ab, const char *name) {
  AB_PROVIDER *pro;

  assert(ab);
  assert(name);
  pro=AB_Provider_List_First(ab->providers);
  while(pro) {
    if (strcasecmp(AB_Provider_GetName(pro), name)==0)
      break;
    pro=AB_Provider_List_Next(pro);
  } /* while */

  return pro;
}



AB_PROVIDER *AB_Banking_GetProvider(AB_BANKING *ab, const char *name) {
  AB_PROVIDER *pro;

  assert(ab);
  assert(name);
  pro=AB_Banking_FindProvider(ab, name);
  if (pro)
    return pro;
  pro=AB_Banking_LoadProviderPlugin(ab, name);
  if (pro) {
    if (AB_Banking_InitProvider(ab, pro)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not init provider \"%s\"", name);
      AB_Provider_free(pro);
      return 0;
    }
    AB_Provider_List_Add(pro, ab->providers);
  }

  return pro;
}



AB_ACCOUNT_LIST2 *AB_Banking_GetAccounts(const AB_BANKING *ab){
  AB_ACCOUNT_LIST2 *al;
  AB_ACCOUNT *a;

  assert(ab);
  if (AB_Account_List_GetCount(ab->accounts)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No accounts");
    return 0;
  }
  al=AB_Account_List2_new();
  a=AB_Account_List_First(ab->accounts);
  assert(a);
  while(a) {
    AB_Account_List2_PushBack(al, a);
    a=AB_Account_List_Next(a);
  } /* while */

  return al;
}



AB_ACCOUNT *AB_Banking_GetAccount(const AB_BANKING *ab,
                                  GWEN_TYPE_UINT32 uniqueId){
  AB_ACCOUNT *a;

  assert(ab);
  if (AB_Account_List_GetCount(ab->accounts)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No accounts");
    return 0;
  }
  a=AB_Account_List_First(ab->accounts);
  assert(a);
  while(a) {
    if (AB_Account_GetUniqueId(a)==uniqueId)
      break;
    a=AB_Account_List_Next(a);
  } /* while */

  return a;
}



AB_ACCOUNT *AB_Banking_GetAccountByCodeAndNumber(const AB_BANKING *ab,
                                                 const char *bankCode,
                                                 const char *accountId){
  AB_ACCOUNT *a;

  if ((bankCode == NULL) || (accountId == NULL))
    return NULL;
  assert(ab);
  if (AB_Account_List_GetCount(ab->accounts)==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No accounts");
    return 0;
  }
  a=AB_Account_List_First(ab->accounts);
  assert(a);
  while(a) {
    if (bankCode) {
      if (strcasecmp(AB_Account_GetBankCode(a), bankCode)==0 &&
          strcasecmp(AB_Account_GetAccountNumber(a), accountId)==0)
        break;
    }
    else {
      if (strcasecmp(AB_Account_GetAccountNumber(a), accountId)==0)
        break;
    }
    a=AB_Account_List_Next(a);
  } /* while */

  return a;
}



int AB_Banking_Init(AB_BANKING *ab) {
  GWEN_DB_NODE *dbT;
  GWEN_DB_NODE *dbTsrc;
  AB_JOB_LIST2 *jl;
  int i;
  const char *s;
  GWEN_PLUGIN_MANAGER *pm;

  assert(ab);

  if (!GWEN_Logger_IsOpen(AQBANKING_LOGDOMAIN)) {
    GWEN_Logger_Open(AQBANKING_LOGDOMAIN,
                     "aqbanking", 0,
                     GWEN_LoggerTypeConsole,
                     GWEN_LoggerFacilityUser);
  }

#ifdef HAVE_I18N
  setlocale(LC_ALL,"");
  s=bindtextdomain(PACKAGE,  LOCALEDIR);
  if (s) {
    DBG_NOTICE(AQBANKING_LOGDOMAIN, "Locale bound.");
    bind_textdomain_codeset(PACKAGE, "UTF-8");
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error binding locale");
  }
#endif

  /* create bankinfo plugin manager */
  DBG_INFO(AQBANKING_LOGDOMAIN, "Registering bankinfo plugin manager");
  pm=GWEN_PluginManager_new("bankinfo");
  GWEN_PluginManager_AddPathFromWinReg(pm,
				       "Software\\Aqbanking\\Paths",
				       "bankinfodir");
  GWEN_PluginManager_AddPath(pm,
                             AQBANKING_PLUGINS
                             DIRSEP
                             AB_BANKINFO_PLUGIN_FOLDER);
  if (GWEN_PluginManager_Register(pm)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not register bankinfo plugin manager");
    return AB_ERROR_GENERIC;
  }
  ab->pluginManagerBankInfo=pm;

  /* create provider plugin manager */
  DBG_INFO(AQBANKING_LOGDOMAIN, "Registering provider plugin manager");
  pm=GWEN_PluginManager_new("provider");
  GWEN_PluginManager_AddPathFromWinReg(pm,
				       "Software\\Aqbanking\\Paths",
				       "providerdir");
  GWEN_PluginManager_AddPath(pm,
			     AQBANKING_PLUGINS
			     DIRSEP
			     AB_PROVIDER_FOLDER);
  if (GWEN_PluginManager_Register(pm)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not register provider plugin manager");
    return AB_ERROR_GENERIC;
  }
  ab->pluginManagerProvider=pm;

  /* create imexporters plugin manager */
  DBG_INFO(AQBANKING_LOGDOMAIN, "Registering imexporters plugin manager");
  pm=GWEN_PluginManager_new("imexporters");
  GWEN_PluginManager_AddPathFromWinReg(pm,
				       "Software\\Aqbanking\\Paths",
				       "imexporterdir");
  GWEN_PluginManager_AddPath(pm,
			     AQBANKING_PLUGINS
			     DIRSEP
			     AB_IMEXPORTER_FOLDER);
  if (GWEN_PluginManager_Register(pm)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not register imexporters plugin manager");
    return AB_ERROR_GENERIC;
  }
  ab->pluginManagerImExporter=pm;

  /* create imexporters plugin manager */
  DBG_INFO(AQBANKING_LOGDOMAIN, "Registering pkgdatadir plugin manager");
  pm=GWEN_PluginManager_new("pkgdatadir");
  GWEN_PluginManager_AddPathFromWinReg(pm,
				       "Software\\Aqbanking\\Paths",
				       "pkgdatadir");
  GWEN_PluginManager_AddPath(pm,
			     PKGDATADIR);
  if (GWEN_PluginManager_Register(pm)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not register pkgdatadir plugin manager");
    return AB_ERROR_GENERIC;
  }
  ab->pluginManagerPkgdatadir=pm;

  /* create crypt plugin manager */
  DBG_INFO(AQBANKING_LOGDOMAIN, "Registering crypttoken plugin manager");
  pm=AB_CryptManager_new(ab);
  if (pm) {
    GWEN_BUFFER *ctbuf;

    /* add path from gwen since all crypt token plugins are installed there */
    ctbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_GetPluginPath(ctbuf);
    GWEN_Buffer_AppendString(ctbuf, DIRSEP);
    GWEN_Buffer_AppendString(ctbuf, "crypttoken");
    GWEN_PluginManager_AddPath(pm,
                               GWEN_Buffer_GetStart(ctbuf));
    GWEN_Buffer_free(ctbuf);

    if (GWEN_PluginManager_Register(pm)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Could not register crypttoken plugin manager");
      return AB_ERROR_GENERIC;
    }
    ab->pluginManagerCryptToken=pm;
  }

  /* read config file */
  if (access(ab->configFile, F_OK)) {
    DBG_NOTICE(AQBANKING_LOGDOMAIN,
               "Configuration file \"%s\" does not exist, "
               "will create it later.", ab->configFile);
    return 0;
  }

  dbT=GWEN_DB_Group_new("config");
  assert(dbT);
  if (GWEN_DB_ReadFile(dbT, ab->configFile,
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_CREATE_GROUP |
                       GWEN_DB_FLAGS_LOCKFILE)) {
    GWEN_DB_Group_free(dbT);
    return AB_ERROR_BAD_CONFIG_FILE;
  }

  /* read active providers */
  for (i=0; ; i++) {
    const char *p;

    p=GWEN_DB_GetCharValue(dbT, "activeProviders", i, 0);
    if (!p)
      break;
    GWEN_StringList_AppendString(ab->activeProviders, p, 0, 1);
  }

  s=GWEN_DB_GetCharValue(dbT, "datadir", 0, 0);
  free(ab->dataDir);
  if (s) ab->dataDir=strdup(s);
  else ab->dataDir=0;

  /* read data */
  dbTsrc=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "banking");
  if (dbTsrc) {
    GWEN_DB_NODE *dbTdst;

    dbTdst=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT, "static");
    GWEN_DB_AddGroupChildren(dbTdst, dbTsrc);
  }

  ab->alwaysAskForCert=GWEN_DB_GetIntValue(ab->data,
                                           "static/alwaysAskForCert", 0, 0);
  ab->pinCacheEnabled=0;

  /* init active providers */
  if (GWEN_StringList_Count(ab->activeProviders)) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(ab->activeProviders);
    assert(se);
    while(se) {
      const char *p;
      AB_PROVIDER *pro;

      p=GWEN_StringListEntry_Data(se);
      assert(p);

      pro=AB_Banking_GetProvider(ab, p);
      if (!pro) {
        DBG_WARN(AQBANKING_LOGDOMAIN,
                 "Error loading/initializing backend \"%s\"", p);
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* read accounts */
  dbTsrc=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "accounts");
  if (dbTsrc) {
    GWEN_DB_NODE *dbA;

    dbA=GWEN_DB_FindFirstGroup(dbTsrc, "account");
    while(dbA) {
      AB_ACCOUNT *a;

      a=AB_Account_fromDbWithProvider(ab, dbA);
      if (a) {
        int rv;

        rv=AB_Account_Update(a);
        if (rv) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "here");
        }
        else {
          DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding account");
          AB_Account_List_Add(a, ab->accounts);
        }
      }
      dbA=GWEN_DB_FindNextGroup(dbA, "account");
    } /* while */
  }

  /* ask active providers for account lists */
  if (GWEN_StringList_Count(ab->activeProviders)) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(ab->activeProviders);
    assert(se);
    while(se) {
      const char *p;
      int rv;

      p=GWEN_StringListEntry_Data(se);
      assert(p);
      rv=AB_Banking_ImportProviderAccounts(ab, p);
      if (rv) {
        DBG_WARN(AQBANKING_LOGDOMAIN, "Error importing accounts from backend \"%s\"", p);
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* read jobs */
  jl=AB_Banking__LoadJobsAs(ab, "todo");
  if (jl) {
    AB_JOB_LIST2_ITERATOR *it;
    AB_JOB *j;

    AB_Job_List_free(ab->enqueuedJobs);
    ab->enqueuedJobs=AB_Job_List_new();

    it=AB_Job_List2_First(jl);
    assert(it);
    j=AB_Job_List2Iterator_Data(it);
    assert(j);
    while(j) {
      if (AB_Job_CheckAvailability(j)) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "Job not available, ignoring");
      }
      else
	AB_Job_List_Add(j, ab->enqueuedJobs);
      j=AB_Job_List2Iterator_Next(it);
    } /* while */
    AB_Job_List2Iterator_free(it);
    AB_Job_List2_free(jl);
  }
  GWEN_DB_Group_free(dbT);

  GWEN_WaitCallback_Enter(AB_BANKING_WCB_GENERIC);

  return 0;
}




int AB_Banking_Fini(AB_BANKING *ab) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbT;
  AB_ACCOUNT *a;
  AB_JOB *j;
  int rv;
  AB_PROVIDER *pro;
  GWEN_BUFFER *rpbuf;

  assert(ab);

  /* deinit all providers */
  pro=AB_Provider_List_First(ab->providers);
  while(pro) {
    while (AB_Provider_IsInit(pro)) {
      rv=AB_Banking_FiniProvider(ab, pro);
      if (rv) {
	DBG_WARN(AQBANKING_LOGDOMAIN,
		 "Error deinitializing backend \"%s\"",
		 AB_Provider_GetName(pro));
        break;
      }
    }
    pro=AB_Provider_List_Next(pro);
  } /* while */

  GWEN_DB_SetIntValue(ab->data, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "static/alwaysAskForCert",
                      ab->alwaysAskForCert);

  db=GWEN_DB_Group_new("config");
  assert(db);

  if (ab->dataDir)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "dataDir", ab->dataDir);

  /* save accounts */
  a=AB_Account_List_First(ab->accounts);
  while(a) {
    GWEN_DB_NODE *dbTdst;
    int rv;

    dbTdst=GWEN_DB_GetGroup(db,
                            GWEN_DB_FLAGS_DEFAULT |
                            GWEN_PATH_FLAGS_CREATE_GROUP,
                            "accounts/account");
    assert(dbTdst);
    rv=AB_Account_toDb(a, dbTdst);
    if (rv) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error saving account \"%08x\"",
                AB_Account_GetUniqueId(a));
      GWEN_DB_Group_free(db);
      GWEN_Logger_Close(AQBANKING_LOGDOMAIN);
      return rv;
    }
    a=AB_Account_List_Next(a);
  } /* while */

  /* save enqueued jobs */
  j=AB_Job_List_First(ab->enqueuedJobs);
  while(j) {
    if (AB_Banking__SaveJobAs(ab, j, "todo")) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Error saving job, ignoring");
    }
    j=AB_Job_List_Next(j);
  } /* while */

  /* save list of active backends */
  if (GWEN_StringList_Count(ab->activeProviders)) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(ab->activeProviders);
    assert(se);
    while(se) {
      const char *p;

      p=GWEN_StringListEntry_Data(se);
      assert(p);
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
                           "activeProviders", p);

      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* store appplication specific data */
  rv=AB_Banking__SaveAppData(ab);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not save configuration");
    GWEN_DB_Group_free(db);
    return rv;
  }
  GWEN_DB_DeleteGroup(ab->data, "static/apps");

  /* save bad pins */
  rv=AB_Banking__SaveBadPins(ab);
  if (rv) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Could not save bad pins");
  }

  /* store static config data as "banking" */
  dbT=GWEN_DB_GetGroup(ab->data, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "static");
  if (dbT) {
    GWEN_DB_NODE *dbTdst;

    dbTdst=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "banking");
    assert(dbTdst);
    GWEN_DB_AddGroupChildren(dbTdst, dbT);
  }

  /* write config file. TODO: make backups */

  rpbuf=GWEN_Buffer_new(0, strlen(ab->configFile)+4, 0, 1);
  GWEN_Buffer_AppendString(rpbuf, ab->configFile);
  GWEN_Buffer_AppendString(rpbuf, ".tmp");
  if (GWEN_DB_WriteFile(db, GWEN_Buffer_GetStart(rpbuf),
                        GWEN_DB_FLAGS_DEFAULT|GWEN_DB_FLAGS_LOCKFILE)) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Could not save app config file \"%s\"",
             GWEN_Buffer_GetStart(rpbuf));
    GWEN_Buffer_free(rpbuf);
    GWEN_DB_Group_free(db);
    return AB_ERROR_BAD_CONFIG_FILE;
  }
#ifdef OS_WIN32
  if (unlink(ab->configFile)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not delete old file \"%s\": %s",
	      ab->configFile,
              strerror(errno));
    GWEN_Buffer_free(rpbuf);
    GWEN_DB_Group_free(db);
    return AB_ERROR_GENERIC;
  }
#endif
  if (rename(GWEN_Buffer_GetStart(rpbuf), ab->configFile)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not rename file to \"%s\": %s",
              ab->configFile,
              strerror(errno));
    GWEN_Buffer_free(rpbuf);
    GWEN_DB_Group_free(db);
    return AB_ERROR_GENERIC;
  }
  GWEN_Buffer_free(rpbuf);

  GWEN_DB_Group_free(db);

  AB_Job_List_Clear(ab->enqueuedJobs);
  AB_Account_List_Clear(ab->accounts);
  AB_Provider_List_Clear(ab->providers);

  /* unregister and unload crypt token plugin manager */
  if (ab->pluginManagerCryptToken) {
      if (GWEN_PluginManager_Unregister(ab->pluginManagerCryptToken)) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Could not unregister crypt token plugin manager");
      }
      GWEN_PluginManager_free(ab->pluginManagerCryptToken);
      ab->pluginManagerCryptToken=0;
  }

  /* unregister and unload provider plugin manager */
  if (ab->pluginManagerProvider) {
      if (GWEN_PluginManager_Unregister(ab->pluginManagerProvider)) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Could not unregister provider plugin manager");
      }
      GWEN_PluginManager_free(ab->pluginManagerProvider);
      ab->pluginManagerProvider=0;
  }

  /* unregister and unload bankinfo plugin manager */
  if (ab->pluginManagerBankInfo) {
    if (GWEN_PluginManager_Unregister(ab->pluginManagerBankInfo)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not unregister bankinfo plugin manager");
    }
    GWEN_PluginManager_free(ab->pluginManagerBankInfo);
    ab->pluginManagerBankInfo=0;
  }

  /* unregister and unload imexporters plugin manager */
  if (ab->pluginManagerImExporter) {
    if (GWEN_PluginManager_Unregister(ab->pluginManagerImExporter)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not unregister imexporter plugin manager");
    }
    GWEN_PluginManager_free(ab->pluginManagerImExporter);
    ab->pluginManagerImExporter=0;
  }

  /* unregister and unload imexporters plugin manager */
  if (ab->pluginManagerPkgdatadir) {
    if (GWEN_PluginManager_Unregister(ab->pluginManagerPkgdatadir)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not unregister Pkgdatadir plugin manager");
    }
    GWEN_PluginManager_free(ab->pluginManagerPkgdatadir);
    ab->pluginManagerPkgdatadir=0;
  }

  GWEN_DB_ClearGroup(ab->data, 0);
  free(ab->dataDir);
  ab->dataDir=0;
  GWEN_Logger_Close(AQBANKING_LOGDOMAIN);
  GWEN_WaitCallback_Leave();
  return 0;
}



int AB_Banking_Save(AB_BANKING *ab) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbT;
  AB_ACCOUNT *a;
  AB_JOB *j;
  int rv;
  AB_PROVIDER *pro;
  GWEN_BUFFER *rpbuf;

  assert(ab);

  db=GWEN_DB_Group_new("config");
  assert(db);

  /* save accounts */
  a=AB_Account_List_First(ab->accounts);
  while(a) {
    GWEN_DB_NODE *dbTdst;
    int rv;

    dbTdst=GWEN_DB_GetGroup(db,
                            GWEN_DB_FLAGS_DEFAULT |
                            GWEN_PATH_FLAGS_CREATE_GROUP,
                            "accounts/account");
    assert(dbTdst);
    rv=AB_Account_toDb(a, dbTdst);
    if (rv) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error saving account \"%08x\"",
                AB_Account_GetUniqueId(a));
      GWEN_DB_Group_free(db);
      return rv;
    }
    a=AB_Account_List_Next(a);
  } /* while */

  /* save enqueued jobs */
  j=AB_Job_List_First(ab->enqueuedJobs);
  while(j) {
    if (AB_Banking__SaveJobAs(ab, j, "todo")) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Error saving job, ignoring");
    }
    j=AB_Job_List_Next(j);
  } /* while */

  /* save list of active backends */
  if (GWEN_StringList_Count(ab->activeProviders)) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(ab->activeProviders);
    assert(se);
    while(se) {
      const char *p;

      p=GWEN_StringListEntry_Data(se);
      assert(p);
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
                           "activeProviders", p);

      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* save all providers */
  pro=AB_Provider_List_First(ab->providers);
  while(pro) {
    rv=AB_Banking__SaveProviderData(ab,
				    AB_Provider_GetEscapedName(pro),
				    0);
    if (rv) {
      DBG_WARN(AQBANKING_LOGDOMAIN,
	       "Error saving backend \"%s\"",
	       AB_Provider_GetName(pro));
      break;
    }
    pro=AB_Provider_List_Next(pro);
  } /* while */

  /* store appplication specific data */
  rv=AB_Banking__SaveAppData(ab);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not save configuration");
    GWEN_DB_Group_free(db);
    return rv;
  }

  /* save bad pins */
  rv=AB_Banking__SaveBadPins(ab);
  if (rv) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Could not save bad pins");
  }

  /* store static config data as "banking" */
  dbT=GWEN_DB_GetGroup(ab->data, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "static");
  if (dbT) {
    GWEN_DB_NODE *dbTdst;

    dbTdst=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "banking");
    assert(dbTdst);
    GWEN_DB_AddGroupChildren(dbTdst, dbT);
  }

  /* write config file. TODO: make backups */
  rpbuf=GWEN_Buffer_new(0, strlen(ab->configFile)+4, 0, 1);
  GWEN_Buffer_AppendString(rpbuf, ab->configFile);
  GWEN_Buffer_AppendString(rpbuf, ".tmp");
  if (GWEN_DB_WriteFile(db, GWEN_Buffer_GetStart(rpbuf),
                        GWEN_DB_FLAGS_DEFAULT|GWEN_DB_FLAGS_LOCKFILE)) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "Could not save app config file \"%s\"",
             ab->configFile);
    GWEN_Buffer_free(rpbuf);
    GWEN_DB_Group_free(db);
    return AB_ERROR_GENERIC;
  }
#ifdef OS_WIN32
  if (unlink(ab->configFile)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not delete old file \"%s\": %s",
              ab->configFile,
              strerror(errno));
    GWEN_Buffer_free(rpbuf);
    GWEN_DB_Group_free(db);
    return AB_ERROR_GENERIC;
  }
#endif
  if (rename(GWEN_Buffer_GetStart(rpbuf), ab->configFile)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not rename file to \"%s\": %s",
              ab->configFile,
              strerror(errno));
    GWEN_Buffer_free(rpbuf);
    GWEN_DB_Group_free(db);
    return AB_ERROR_GENERIC;
  }
  GWEN_Buffer_free(rpbuf);
  GWEN_DB_Group_free(db);

  return 0;
}





GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetProviderDescrs(AB_BANKING *ab){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *l;

  l=GWEN_LoadPluginDescrs(AQBANKING_PLUGINS DIRSEP "providers");
  if (l) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;
    GWEN_PLUGIN_DESCRIPTION *pd;

    it=GWEN_PluginDescription_List2_First(l);
    assert(it);
    pd=GWEN_PluginDescription_List2Iterator_Data(it);
    assert(pd);
    while(pd) {
      if (GWEN_StringList_HasString(ab->activeProviders,
                                    GWEN_PluginDescription_GetName(pd)))
        GWEN_PluginDescription_SetIsActive(pd, 1);
      else
        GWEN_PluginDescription_SetIsActive(pd, 0);
      pd=GWEN_PluginDescription_List2Iterator_Next(it);
    }
    GWEN_PluginDescription_List2Iterator_free(it);
  }

  return l;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetWizardDescrs(AB_BANKING *ab,
                                                           const char *pn){
  GWEN_BUFFER *pbuf;
  GWEN_PLUGIN_DESCRIPTION_LIST2 *wdl;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(pbuf,
                           AQBANKING_PLUGINS
                           DIRSEP
                           AB_PROVIDER_WIZARD_FOLDER
                           DIRSEP);
  GWEN_Buffer_AppendString(pbuf, pn);

  wdl=GWEN_LoadPluginDescrs(GWEN_Buffer_GetStart(pbuf));

  GWEN_Buffer_free(pbuf);
  return wdl;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetDebuggerDescrs(AB_BANKING *ab,
                                                            const char *pn){
  GWEN_BUFFER *pbuf;
  GWEN_PLUGIN_DESCRIPTION_LIST2 *wdl;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(pbuf,
                           AQBANKING_PLUGINS
                           DIRSEP
			   AB_PROVIDER_DEBUGGER_FOLDER
                           DIRSEP);
  GWEN_Buffer_AppendString(pbuf, pn);

  wdl=GWEN_LoadPluginDescrs(GWEN_Buffer_GetStart(pbuf));

  GWEN_Buffer_free(pbuf);
  return wdl;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetImExporterDescrs(AB_BANKING *ab){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *l;

  l=GWEN_LoadPluginDescrs(AQBANKING_PLUGINS DIRSEP "imexporters");
  return l;
}



int AB_Banking__MergeInAccount(AB_BANKING *ab, AB_ACCOUNT *a) {
  AB_ACCOUNT *ta;
  const char *accountId;
  const char *bankCode;
  GWEN_DB_NODE *dbNew;
  GWEN_DB_NODE *dbOld;
  const char *p;

  accountId=AB_Account_GetAccountNumber(a);
  assert(accountId);
  bankCode=AB_Account_GetBankCode(a);
  assert(bankCode);

  ta=AB_Account_List_First(ab->accounts);
  if (!ta) {
    DBG_NOTICE(AQBANKING_LOGDOMAIN, "No accounts.");
  }
  while(ta) {
    if (AB_Account_GetProvider(a)==AB_Account_GetProvider(ta)) {
      const char *caccountId;
      const char *cbankCode;

      caccountId=AB_Account_GetAccountNumber(ta);
      assert(caccountId);
      cbankCode=AB_Account_GetBankCode(ta);
      assert(cbankCode);

      if ((strcasecmp(accountId, caccountId)==0) &&
          (strcasecmp(bankCode, cbankCode)==0)) {
        break;
      }
    }
    ta=AB_Account_List_Next(ta);
  } /* while */

  if (!ta) {
    /* account is new, simply add it */
    DBG_NOTICE(AQBANKING_LOGDOMAIN, "Adding account");
    AB_Account_SetUniqueId(a, AB_Banking_GetUniqueId(ab));
    AB_Account_List_Add(a, ab->accounts);
    return 0;
  }

  /* copy new provider data over old data */
  DBG_NOTICE(AQBANKING_LOGDOMAIN, "Updating account");
  dbNew=AB_Account_GetProviderData(ta);
  assert(dbNew);
  dbOld=AB_Account_GetProviderData(a);
  assert(dbOld);
  GWEN_DB_ClearGroup(dbOld, 0);
  GWEN_DB_AddGroupChildren(dbOld, dbNew);

  /* copy new data over old one */
  p=AB_Account_GetAccountName(a);
  if (p)
    AB_Account_SetAccountName(ta, p);
  p=AB_Account_GetOwnerName(a);
  if (p)
    AB_Account_SetOwnerName(ta, p);

  AB_Account_free(a);
  return 0;
}



int AB_Banking_ImportProviderAccounts(AB_BANKING *ab, const char *backend){
  AB_ACCOUNT_LIST2 *al;
  AB_ACCOUNT_LIST2_ITERATOR *ait;
  AB_ACCOUNT *a;
  AB_PROVIDER *pro;
  int successful;

  pro=AB_Banking_GetProvider(ab, backend);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Backend \"%s\" not found", backend);
    return AB_ERROR_NOT_FOUND;
  }

  al=AB_Provider_GetAccountList(pro);
  if (!al) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Backend \"%s\" has no accounts", backend);
    return AB_ERROR_EMPTY;
  }

  ait=AB_Account_List2_First(al);
  assert(ait);
  a=AB_Account_List2Iterator_Data(ait);
  successful=0;
  assert(a);
  while(a) {
    AB_Account_SetProvider(a, pro);
    if (AB_Banking__MergeInAccount(ab, a)) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Could not merge in account");
    }
    else
      successful++;
    a=AB_Account_List2Iterator_Next(ait);
  }
  AB_Account_List2Iterator_free(ait);
  AB_Account_List2_free(al);

  if (!successful) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No account imported");
    return AB_ERROR_EMPTY;
  }
  return 0;
}



int AB_Banking_UpdateAccountList(AB_BANKING *ab){
  assert(ab);
  if (GWEN_StringList_Count(ab->activeProviders)) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(ab->activeProviders);
    assert(se);
    while(se) {
      const char *p;
      int rv;

      p=GWEN_StringListEntry_Data(se);
      assert(p);
      rv=AB_Banking_ImportProviderAccounts(ab, p);
      if (rv) {
        DBG_WARN(AQBANKING_LOGDOMAIN, "Error importing accounts from backend \"%s\"", p);
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  return 0;
}



int AB_Banking_EnqueueJob(AB_BANKING *ab, AB_JOB *j){
  int rv;
  AB_JOB_STATUS jst;

  assert(ab);
  assert(j);
  rv=AB_Job_CheckAvailability(j);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Job is not available, refusing to enqueue.");
    return rv;
  }
  jst=AB_Job_GetStatus(j);
  if (jst==AB_Job_StatusFinished ||
      jst==AB_Job_StatusError ||
      jst==AB_Job_StatusEnqueued) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Job has already been enqueued or even finished, "
	      "not enqueueing it");
    return AB_ERROR_INVALID;
  }

  /* adapt status if necessary */
  if (jst!=AB_Job_StatusEnqueued &&
      jst!=AB_Job_StatusPending)
    AB_Job_SetStatus(j, AB_Job_StatusEnqueued);

  /* really enqueue the job */
  if (AB_Job_GetJobId(j)==0)
    AB_Job_SetUniqueId(j, AB_Banking_GetUniqueId(ab));
  AB_Job_Attach(j);
  AB_Job_List_Add(j, ab->enqueuedJobs);
  AB_Banking__SaveJobAs(ab, j, "todo");

  /* unlink it from whatever previous list it belonged to */
  switch(jst) {
  case AB_Job_StatusPending:
    AB_Banking__UnlinkJobAs(ab, j, "pending"); break;
  case AB_Job_StatusDeferred:
    AB_Banking__UnlinkJobAs(ab, j, "deferred"); break;
  default:
    break;
  }

  /* done */
  return 0;
}



int AB_Banking_DequeueJob(AB_BANKING *ab, AB_JOB *j){
  int rv;
  AB_JOB_STATUS jst;

  assert(ab);
  assert(j);
  jst=AB_Job_GetStatus(j);
  if (jst==AB_Job_StatusEnqueued) {
    AB_Job_SetStatus(j, AB_Job_StatusNew);
    AB_Job_List_Del(j);
  }
  rv=AB_Banking__UnlinkJobAs(ab, j, "todo");
  AB_Job_free(j);
  return rv;
}



int AB_Banking_DeferJob(AB_BANKING *ab, AB_JOB *j){
  int rv;
  AB_JOB_STATUS jst;

  assert(ab);
  assert(j);
  jst=AB_Job_GetStatus(j);
  if (jst!=AB_Job_StatusEnqueued) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "I can only defer jobs which haven't been executed.");
    return AB_ERROR_INVALID;
  }

  AB_Job_SetStatus(j, AB_Job_StatusDeferred);
  rv=AB_Banking__SaveJobAs(ab, j, "deferred");
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not save job as \"deferred\", not dequeueing it");
    AB_Job_SetStatus(j, jst);
    return rv;
  }

  AB_Job_List_Del(j);
  rv=AB_Banking__UnlinkJobAs(ab, j, "todo");
  AB_Job_free(j);
  return rv;
}



int AB_Banking_EnqueuePendingJobs(AB_BANKING *ab, int mineOnly){
  AB_JOB_LIST2 *jl;
  int errorCount;
  int successCount;

  errorCount=successCount=0;
  jl=AB_Banking_GetPendingJobs(ab);
  if (jl) {
    AB_JOB *j;
    AB_JOB_LIST2_ITERATOR *it;

    it=AB_Job_List2_First(jl);
    assert(it);

    j=AB_Job_List2Iterator_Data(it);
    assert(j);
    while(j) {
      int doit;

      if (!mineOnly)
        doit=1;
      else
        doit=(strcasecmp(AB_Job_GetCreatedBy(j), ab->appName)==0);
      if (doit) {
        if (AB_Banking_EnqueueJob(ab, j)) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Error enqueueing job %d",
                    AB_Job_GetJobId(j));
          errorCount++;
        }
        else
          successCount++;
      }
      j=AB_Job_List2Iterator_Next(it);
    } /* while */
    AB_Job_List2Iterator_free(it);
    AB_Job_List2_FreeAll(jl);
  } /* if pending jobs */

  if (!errorCount)
    return 0;
  if (errorCount)
    /* all attempts resulted in errors */
    return AB_ERROR_GENERIC;
  return 0;
}



int AB_Banking__ExecuteQueue(AB_BANKING *ab, AB_JOB_LIST *jl){
  AB_PROVIDER *pro;
  int succ;

  assert(ab);
  pro=AB_Provider_List_First(ab->providers);
  succ=0;

  while(pro) {
    AB_JOB *j;
    int jobs;
    int rv;

    j=AB_Job_List_First(jl);
    jobs=0;
    while(j) {
      AB_JOB *jnext;
      AB_JOB_STATUS jst;

      jnext=AB_Job_List_Next(j);
      jst=AB_Job_GetStatus(j);
      DBG_NOTICE(AQBANKING_LOGDOMAIN, "Checking job...");
      if (jst==AB_Job_StatusEnqueued ||
	  jst==AB_Job_StatusPending) {
        AB_ACCOUNT *a;

        a=AB_Job_GetAccount(j);
        assert(a);
	if (AB_Account_GetProvider(a)==pro) {
	  DBG_NOTICE(AQBANKING_LOGDOMAIN, "Same provider, adding job");
          /* same provider, add job */
          rv=AB_Provider_AddJob(pro, j);
          if (rv) {
            DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not add job (%d)", rv);
            AB_Job_SetStatus(j, AB_Job_StatusError);
            AB_Job_SetResultText(j, "Refused by backend");
          }
          else {
	    jobs++;
	    if (AB_Job_GetStatus(j)!=AB_Job_StatusPending) {
	      AB_Job_SetStatus(j, AB_Job_StatusSent);
	      AB_Banking__SaveJobAs(ab, j, "sent");
	      AB_Banking__UnlinkJobAs(ab, j, "todo");
            }
	    else {
	      AB_Banking__SaveJobAs(ab, j, "sent");
	      AB_Banking__UnlinkJobAs(ab, j, "todo");
            }
          }
        }
      } /* if job enqueued */
      else {
	DBG_WARN(AQBANKING_LOGDOMAIN, "Job in queue with status \"%s\"",
		 AB_Job_Status2Char(AB_Job_GetStatus(j)));
      }
      j=jnext;
    } /* while */
    if (jobs) {
      DBG_NOTICE(AQBANKING_LOGDOMAIN, "Letting backend \"%s\" work",
                 AB_Provider_GetName(pro));
      rv=AB_Provider_Execute(pro);
      if (rv) {
        int lrv;

        if (rv==AB_ERROR_USER_ABORT) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "Aborted by user");
          return rv;
        }
        DBG_NOTICE(AQBANKING_LOGDOMAIN, "Error executing backend's queue");
        lrv=AB_Banking_MessageBox(ab,
                                  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
                                  AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
                                  AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                                  I18N("Error"),
                                  I18N("Error executing backend's queue.\n"
				       "What shall we do?"),
                                  I18N("Continue"), I18N("Abort"), 0);
        if (lrv!=1) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "Aborted by user");
          return AB_ERROR_USER_ABORT;
        }
      }
      else
        succ++;
    } /* if jobs in backend's queue */

    pro=AB_Provider_List_Next(pro);
  } /* while */

  pro=AB_Provider_List_First(ab->providers);

  if (!succ) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Not a single job successfully executed");
    return AB_ERROR_GENERIC;
  }

  return 0;
}



int AB_Banking_ExecuteQueue(AB_BANKING *ab){
  int rv;
  AB_JOB *j;
  AB_PROVIDER *pro;

  assert(ab);

  /* clear temporarily accepted certificates */
  GWEN_DB_ClearGroup(ab->dbTempConfig, "certificates");

  rv=AB_Banking__ExecuteQueue(ab, ab->enqueuedJobs);

  /* clear temporarily accepted certificates again */
  GWEN_DB_ClearGroup(ab->dbTempConfig, "certificates");

  /* clear queue */
  j=AB_Job_List_First(ab->enqueuedJobs);
  while(j) {
    AB_JOB *nj;

    nj=AB_Job_List_Next(j);

    AB_Job_Attach(j);
    AB_Job_List_Del(j);

    switch(AB_Job_GetStatus(j)) {
    case AB_Job_StatusEnqueued:
      /* job still enqueued, so it has never been sent */
      AB_Job_SetStatus(j, AB_Job_StatusError);
      AB_Job_SetResultText(j, "Job has never been sent");
      if (AB_Banking__SaveJobAs(ab, j, "finished")) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not save job as \"finished\"");
      }
      AB_Banking__UnlinkJobAs(ab, j, "sent");
      break;

    case AB_Job_StatusPending:
      if (AB_Banking__SaveJobAs(ab, j, "pending")) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not save job as \"pending\"");
      }
      AB_Banking__UnlinkJobAs(ab, j, "sent");
      break;

    case AB_Job_StatusSent:
    case AB_Job_StatusFinished:
    case AB_Job_StatusError:
    default:
      if (AB_Banking__SaveJobAs(ab, j, "finished")) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not save job as \"finished\"");
      }
      AB_Banking__UnlinkJobAs(ab, j, "sent");
      break;
    }
    AB_Job_free(j);

    j=nj;
  } /* while */

  /* reset all provider queues, this makes sure no job remains in any queue */
  pro=AB_Provider_List_First(ab->providers);
  while(pro) {
    int lrv;

    lrv=AB_Provider_ResetQueue(pro);
    if (lrv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Error resetting providers queue (%d)",
               lrv);
    }
    pro=AB_Provider_List_Next(pro);
  } /* while */


  if (!AB_Banking_GetPinCacheEnabled(ab)) {
    /* If pin caching was disabled, then delete all PINs */
    AB_Pin_List_Clear(ab->pinList);
  }

  return rv;
}



int AB_Banking_ActivateProvider(AB_BANKING *ab, const char *pname) {
  int rv;
  AB_PROVIDER *pro;

  if (GWEN_StringList_HasString(ab->activeProviders, pname)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Provider already active");
    return AB_ERROR_FOUND;
  }

  pro=AB_Banking_GetProvider(ab, pname);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not load backend \"%s\"", pname);
    return AB_ERROR_NOT_FOUND;
  }

  rv=AB_Banking_ImportProviderAccounts(ab, pname);
  if (rv) {
    DBG_WARN(AQBANKING_LOGDOMAIN,
             "Could not import accounts from backend \"%s\"",
             pname);
    AB_Banking_FiniProvider(ab, pro);
    return rv;
  }

  GWEN_StringList_AppendString(ab->activeProviders, pname, 0, 1);
  return 0;
}



int AB_Banking_DeactivateProvider(AB_BANKING *ab, const char *pname) {
  AB_ACCOUNT *a;
  AB_PROVIDER *pro;

  if (!GWEN_StringList_HasString(ab->activeProviders, pname)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Provider not active");
    return AB_ERROR_INVALID;
  }

  pro=AB_Banking_FindProvider(ab, pname);
  if (pro)
    AB_Banking_FiniProvider(ab, pro);

  GWEN_StringList_RemoveString(ab->activeProviders, pname);

  /* delete accounts which use this backend */
  a=AB_Account_List_First(ab->accounts);
  while(a) {
    AB_ACCOUNT *na;

    na=AB_Account_List_Next(a);
    pro=AB_Account_GetProvider(a);
    assert(pro);
    if (strcasecmp(AB_Provider_GetName(pro), pname)==0) {
      AB_Account_List_Del(a);
      AB_Account_free(a);
    }
    a=na;
  }

  return 0;
}



const GWEN_STRINGLIST *AB_Banking_GetActiveProviders(const AB_BANKING *ab) {
  assert(ab);
  if (GWEN_StringList_Count(ab->activeProviders)==0)
    return 0;
  return ab->activeProviders;
}



int AB_Banking_GetUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf){
  char home[256];

  assert(ab);
  if (ab->dataDir) {
    GWEN_Buffer_AppendString(buf, ab->dataDir);
  }
  else {
    if (GWEN_Directory_GetHomeDirectory(home, sizeof(home))) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Could not determine home directory, aborting.");
      return -1;
    }
    GWEN_Buffer_AppendString(buf, home);
    GWEN_Buffer_AppendString(buf, DIRSEP ".banking");
  }
  return 0;
}



int AB_Banking_GetSharedDataDir(const AB_BANKING *ab,
                                const char *name,
                                GWEN_BUFFER *buf){
  char home[256];

  assert(ab);
  if (ab->dataDir) {
    GWEN_Buffer_AppendString(buf, ab->dataDir);
  }
  else {
    if (GWEN_Directory_GetHomeDirectory(home, sizeof(home))) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Could not determine home directory, aborting.");
      return -1;
    }
    GWEN_Buffer_AppendString(buf, home);
    GWEN_Buffer_AppendString(buf, DIRSEP ".banking");
  }

  if (GWEN_Text_EscapeToBufferTolerant(name, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Bad share name, aborting.");
    abort();
  }
  else {
    char *s;

    s=GWEN_Buffer_GetStart(buf);
    while(*s) {
      *s=tolower(*s);
      s++;
    }
  }

  return 0;
}



void AB_Banking_SetUserDataDir(AB_BANKING *ab, const char *s){
  assert(ab);

  free(ab->dataDir);
  if (s) ab->dataDir=strdup(s);
  else ab->dataDir=0;
}



int AB_Banking_GetAppUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf){
  int rv;

  assert(ab->appEscName);
  rv=AB_Banking_GetUserDataDir(ab, buf);
  if (rv)
    return rv;
  GWEN_Buffer_AppendString(buf, DIRSEP "apps" DIRSEP);
  GWEN_Buffer_AppendString(buf, ab->appEscName);
  GWEN_Buffer_AppendString(buf, DIRSEP "data");

  return 0;
}



int AB_Banking_GetProviderUserDataDir(const AB_BANKING *ab,
				      const char *name,
				      GWEN_BUFFER *buf){
  int rv;

  rv=AB_Banking_GetUserDataDir(ab, buf);
  if (rv)
    return rv;
  GWEN_Buffer_AppendString(buf, DIRSEP "backends" DIRSEP);
  GWEN_Buffer_AppendString(buf, name);
  GWEN_Buffer_AppendString(buf, DIRSEP "data");
  return 0;
}



int AB_Banking__ReadImExporterProfiles(AB_BANKING *ab,
                                       const char *path,
                                       GWEN_DB_NODE *db) {
  GWEN_DIRECTORYDATA *d;
  GWEN_BUFFER *nbuf;
  char nbuffer[64];
  unsigned int pathLen;

  if (!path)
    path="";

  /* create path */
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nbuf, path);
  pathLen=GWEN_Buffer_GetUsedBytes(nbuf);

  d=GWEN_Directory_new();
  if (GWEN_Directory_Open(d, GWEN_Buffer_GetStart(nbuf))) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "Path \"%s\" is not available",
	     GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);
    GWEN_Directory_free(d);
    return AB_ERROR_NOT_FOUND;
  }

  while(!GWEN_Directory_Read(d,
                             nbuffer,
                             sizeof(nbuffer))) {
    if (strcmp(nbuffer, ".") &&
        strcmp(nbuffer, "..")) {
      int nlen;

      nlen=strlen(nbuffer);
      if (nlen>4) {
        if (strcasecmp(nbuffer+nlen-5, ".conf")==0) {
          struct stat st;

	  GWEN_Buffer_Crop(nbuf, 0, pathLen);
	  GWEN_Buffer_SetPos(nbuf, pathLen);
	  GWEN_Buffer_AppendString(nbuf, DIRSEP);
	  GWEN_Buffer_AppendString(nbuf, nbuffer);

	  if (stat(GWEN_Buffer_GetStart(nbuf), &st)) {
	    DBG_ERROR(AQBANKING_LOGDOMAIN, "stat(%s): %s",
		      GWEN_Buffer_GetStart(nbuf),
		      strerror(errno));
	  }
	  else {
            if (!S_ISDIR(st.st_mode)) {
              GWEN_DB_NODE *dbT;

              dbT=GWEN_DB_Group_new("profile");
              if (GWEN_DB_ReadFile(dbT,
                                   GWEN_Buffer_GetStart(nbuf),
                                   GWEN_DB_FLAGS_DEFAULT |
                                   GWEN_PATH_FLAGS_CREATE_GROUP)) {
                DBG_ERROR(AQBANKING_LOGDOMAIN,
                          "Could not read file \"%s\"",
                          GWEN_Buffer_GetStart(nbuf));
              }
              else {
                const char *s;

                s=GWEN_DB_GetCharValue(dbT, "name", 0, 0);
                if (!s) {
                  DBG_ERROR(AQBANKING_LOGDOMAIN,
                            "Bad file \"%s\" (no name)",
                            GWEN_Buffer_GetStart(nbuf));
                }
                else {
                  GWEN_DB_NODE *dbTarget;
                  DBG_INFO(AQBANKING_LOGDOMAIN,
                           "File \"%s\" contains profile \"%s\"",
                           GWEN_Buffer_GetStart(nbuf), s);

                  dbTarget=GWEN_DB_GetGroup(db,
                                            GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                                            s);
                  assert(dbTarget);
                  GWEN_DB_AddGroupChildren(dbTarget, dbT);
                } /* if name */
              } /* if file successfully read */
              GWEN_DB_Group_free(dbT);
            } /* if !dir */
          } /* if stat was ok */
        } /* if conf */
      } /* if name has more than 4 chars */
    } /* if not "." and not ".." */
  } /* while */
  GWEN_Directory_Close(d);
  GWEN_Directory_free(d);
  GWEN_Buffer_free(nbuf);

  return 0;
}



GWEN_DB_NODE *AB_Banking_GetImExporterProfiles(AB_BANKING *ab,
                                               const char *name){
  GWEN_BUFFER *buf;
  GWEN_DB_NODE *db;
  int rv;
  const char *pkgdatadir;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_STRINGLISTENTRY *sentry;

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  db=GWEN_DB_Group_new("profiles");

  /* New loading code -- use path list from PluginManager but don't
     use its loading code */
  pm = GWEN_PluginManager_FindPluginManager("pkgdatadir");
  if (!pm) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not find plugin manager for \"%s\"",
              "pkgdatadir");
    return 0;
  }

  sentry = GWEN_StringList_FirstEntry(GWEN_PluginManager_GetPaths(pm));
  assert(sentry);
  /* For now, using the first entry will work both on windows (where
     this is the registry key) and on unix (where this is the
     compile-time variable). */
  pkgdatadir = GWEN_StringListEntry_Data(sentry);
  assert(pkgdatadir);

  /* read global profiles */
  GWEN_Buffer_AppendString(buf, pkgdatadir);
  GWEN_Buffer_AppendString(buf, DIRSEP "imexporters" DIRSEP);
  if (GWEN_Text_EscapeToBufferTolerant(name, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Bad name for importer/exporter");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_AppendString(buf, DIRSEP "profiles");
  rv=AB_Banking__ReadImExporterProfiles(ab,
                                        GWEN_Buffer_GetStart(buf),
                                        db);
  if (rv && rv!=AB_ERROR_NOT_FOUND) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error reading global profiles");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_Reset(buf);

  /* read local user profiles */
  if (AB_Banking_GetUserDataDir(ab, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not get user data dir");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_AppendString(buf, DIRSEP "imexporters" DIRSEP);
  if (GWEN_Text_EscapeToBufferTolerant(name, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Bad name for importer/exporter");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_AppendString(buf, DIRSEP "profiles");

  rv=AB_Banking__ReadImExporterProfiles(ab,
                                        GWEN_Buffer_GetStart(buf),
                                        db);
  if (rv && rv!=AB_ERROR_NOT_FOUND) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error reading users profiles");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_free(buf);

  return db;
}




AB_PROVIDER *AB_Banking_LoadProviderPluginFile(AB_BANKING *ab,
                                               const char *modname,
                                               const char *fname){
  GWEN_LIBLOADER *ll;
  AB_PROVIDER *pro;
  AB_PROVIDER_FACTORY_FN fn;
  void *p;
  GWEN_BUFFER *nbuf;
  const char *s;
  GWEN_ERRORCODE err;
  GWEN_DB_NODE *db;
  GWEN_PLUGIN *pl;
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_FindPluginManager("provider");
  if (!pm) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not find plugin manager for \"%s\"",
              "provider");
    return 0;
  }
  pl=GWEN_PluginManager_LoadPluginFile(pm, modname, fname);
  if (!pl) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not load %s plugin for \"%s\" (file %s)",
              "provider", modname, fname);
    return 0;
  }
  ll=GWEN_Plugin_GetLibLoader(pl);

  /* create name of init function */
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendString(nbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(nbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
    GWEN_Buffer_free(nbuf);
    GWEN_Plugin_free(pl);
    return 0;
  }
  GWEN_Buffer_free(nbuf);

  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "static/providers");
  assert(db);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT, modname);
  assert(db);

  fn=(AB_PROVIDER_FACTORY_FN)p;
  assert(fn);
  pro=fn(ab, db);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in plugin: No provider created");
    GWEN_Plugin_free(pl);
    return 0;
  }

  /* store libloader */
  AB_Provider_SetPlugin(pro, pl);

  return pro;
}



AB_PROVIDER *AB_Banking_LoadProviderPlugin(AB_BANKING *ab,
                                           const char *modname){
  GWEN_LIBLOADER *ll;
  AB_PROVIDER *pro;
  AB_PROVIDER_FACTORY_FN fn;
  void *p;
  const char *s;
  GWEN_ERRORCODE err;
  GWEN_BUFFER *mbuf;
  GWEN_DB_NODE *db;
  GWEN_PLUGIN *pl;
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_FindPluginManager("provider");
  if (!pm) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not find plugin manager for \"%s\"",
              "provider");
    return 0;
  }
  pl=GWEN_PluginManager_LoadPlugin(pm, modname);
  if (!pl) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not load %s plugin for \"%s\"",
	      "provider", modname);
    return 0;
  }
  ll=GWEN_Plugin_GetLibLoader(pl);

  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(mbuf, tolower(*(s++)));

  /* create name of init function */
  GWEN_Buffer_AppendString(mbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(mbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
    GWEN_Buffer_free(mbuf);
    GWEN_Plugin_free(pl);
    return 0;
  }
  GWEN_Buffer_free(mbuf);

  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "static/providers");
  assert(db);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      modname);
  assert(db);

  fn=(AB_PROVIDER_FACTORY_FN)p;
  assert(fn);
  pro=fn(ab, db);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in plugin: No provider created");
    GWEN_Plugin_free(pl);
    return 0;
  }

  /* store libloader */
  AB_Provider_SetPlugin(pro, pl);

  return pro;
}



AB_BANKINFO_PLUGIN *AB_Banking_LoadBankInfoPluginFile(AB_BANKING *ab,
                                                      const char *modname,
                                                      const char *fname){
  GWEN_LIBLOADER *ll;
  AB_BANKINFO_PLUGIN *bip;
  AB_BANKINFO_PLUGIN_FACTORY_FN fn;
  void *p;
  GWEN_BUFFER *nbuf;
  const char *s;
  GWEN_ERRORCODE err;
  GWEN_DB_NODE *db;
  GWEN_PLUGIN *pl;
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_FindPluginManager("bankinfo");
  if (!pm) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not find plugin manager for \"%s\"",
              "bankinfo");
    return 0;
  }
  pl=GWEN_PluginManager_LoadPluginFile(pm, modname, fname);
  if (!pl) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not load %s plugin for \"%s\" (file %s)",
              "bankinfo", modname, fname);
    return 0;
  }
  ll=GWEN_Plugin_GetLibLoader(pl);

  /* create name of init function */
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendString(nbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(nbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
    GWEN_Buffer_free(nbuf);
    GWEN_Plugin_free(pl);
    return 0;
  }
  GWEN_Buffer_free(nbuf);

  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "static/bankinfoplugins");
  assert(db);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT, modname);
  assert(db);

  fn=(AB_BANKINFO_PLUGIN_FACTORY_FN)p;
  assert(fn);
  bip=fn(ab, db);
  if (!bip) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error in plugin: No bankinfoplugin created");
    GWEN_Plugin_free(pl);
    return 0;
  }

  /* store libloader */
  AB_BankInfoPlugin_SetPlugin(bip, pl);

  return bip;
}



AB_BANKINFO_PLUGIN *AB_Banking_LoadBankInfoPlugin(AB_BANKING *ab,
                                                  const char *modname){
  GWEN_LIBLOADER *ll;
  AB_BANKINFO_PLUGIN *bip;
  AB_BANKINFO_PLUGIN_FACTORY_FN fn;
  void *p;
  const char *s;
  GWEN_ERRORCODE err;
  GWEN_BUFFER *mbuf;
  GWEN_DB_NODE *db;
  GWEN_PLUGIN *pl;
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_FindPluginManager("bankinfo");
  if (!pm) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not find plugin manager for \"%s\"",
              "bankinfo");
    return 0;
  }
  pl=GWEN_PluginManager_LoadPlugin(pm, modname);
  if (!pl) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not load %s plugin for \"%s\"",
              "bankinfo", modname);
    return 0;
  }
  ll=GWEN_Plugin_GetLibLoader(pl);

  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(mbuf, tolower(*(s++)));

  /* create name of init function */
  GWEN_Buffer_AppendString(mbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(mbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
    GWEN_Buffer_free(mbuf);
    GWEN_Plugin_free(pl);
    return 0;
  }
  GWEN_Buffer_free(mbuf);

  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "static/bankinfoplugins");
  assert(db);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      modname);
  assert(db);

  fn=(AB_BANKINFO_PLUGIN_FACTORY_FN)p;
  assert(fn);
  bip=fn(ab, db);
  if (!bip) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error in plugin: No bankinfoplugin created");
    GWEN_Plugin_free(pl);
    return 0;
  }

  /* store libloader */
  AB_BankInfoPlugin_SetPlugin(bip, pl);

  return bip;
}



AB_BANKINFO_PLUGIN *AB_Banking_GetBankInfoPlugin(AB_BANKING *ab,
                                                 const char *country) {
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);

  bip=AB_BankInfoPlugin_List_First(ab->bankInfoPlugins);
  while(bip) {
    if (strcasecmp(AB_BankInfoPlugin_GetCountry(bip), country)==0)
      return bip;
    bip=AB_BankInfoPlugin_List_Next(bip);
  }

  bip=AB_Banking_LoadBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "BankInfo plugin for country \"%s\" not found",
              country);
    return 0;
  }
  AB_BankInfoPlugin_List_Add(bip, ab->bankInfoPlugins);
  return bip;
}



AB_BANKINFO *AB_Banking_GetBankInfo(AB_BANKING *ab,
                                    const char *country,
                                    const char *branchId,
                                    const char *bankId){
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);
  bip=AB_Banking_GetBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "BankInfo plugin for country \"%s\" not found",
             country);
    return 0;
  }

  return AB_BankInfoPlugin_GetBankInfo(bip, branchId, bankId);
}



int AB_Banking_GetBankInfoByTemplate(AB_BANKING *ab,
                                     const char *country,
                                     AB_BANKINFO *tbi,
                                     AB_BANKINFO_LIST2 *bl){
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);
  bip=AB_Banking_GetBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "BankInfo plugin for country \"%s\" not found",
             country);
    return 0;
  }

  return AB_BankInfoPlugin_GetBankInfoByTemplate(bip, tbi, bl);
}



AB_BANKINFO_CHECKRESULT
AB_Banking_CheckAccount(AB_BANKING *ab,
                        const char *country,
                        const char *branchId,
                        const char *bankId,
                        const char *accountId) {
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);
  bip=AB_Banking_GetBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "BankInfo plugin for country \"%s\" not found",
             country);
    return AB_BankInfoCheckResult_UnknownResult;
  }

  return AB_BankInfoPlugin_CheckAccount(bip, branchId, bankId, accountId);
}



int AB_Banking__GetWizardPath(AB_BANKING *ab,
                              const char *backend,
                              GWEN_BUFFER *pbuf){
  const char *s;

  GWEN_Buffer_AppendString(pbuf,
                           AQBANKING_PLUGINS
                           DIRSEP
                           AB_PROVIDER_WIZARD_FOLDER
                           DIRSEP);
  s=backend;
  while(*s) GWEN_Buffer_AppendByte(pbuf, tolower(*(s++)));

  return 0;
}



int AB_Banking_GetWizardPath(AB_BANKING *ab,
                             const char *backend,
                             GWEN_BUFFER *pbuf){
  DBG_WARN(AQBANKING_LOGDOMAIN,
            "AB_Banking_GetWizardPath() is deprecated!");
  return AB_Banking__GetWizardPath(ab, backend, pbuf);
}



int AB_Banking__GetDebuggerPath(AB_BANKING *ab,
                                const char *backend,
                                GWEN_BUFFER *pbuf){
  const char *s;

  GWEN_Buffer_AppendString(pbuf,
                           AQBANKING_PLUGINS
			   DIRSEP
			   AB_PROVIDER_DEBUGGER_FOLDER
                           DIRSEP);
  s=backend;
  while(*s) GWEN_Buffer_AppendByte(pbuf, tolower(*(s++)));

  return 0;
}



int AB_Banking_FindDebugger(AB_BANKING *ab,
			    const char *backend,
			    const char *frontends,
			    GWEN_BUFFER *pbuf){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pl;
  char *s;
  char *p;

  pl=AB_Banking_GetDebuggerDescrs(ab, backend);
  if (!pl) {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "No debuggers available for backend \"%s\"", backend);
    return -1;
  }

  if (frontends==0) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;
    GWEN_PLUGIN_DESCRIPTION *pd;
    const char *name;

    pit=GWEN_PluginDescription_List2_First(pl);
    assert(pit);
    pd=GWEN_PluginDescription_List2Iterator_Data(pit);
    while(pd) {
      name=GWEN_PluginDescription_GetName(pd);
      if (!name) {
	DBG_WARN(AQBANKING_LOGDOMAIN,
		 "Found a plugin description with no name");
      }
      else {
	int rv;

	rv=AB_Banking__GetDebuggerPath(ab, backend, pbuf);
	if (rv) {
	  DBG_INFO(AQBANKING_LOGDOMAIN, "here");
	  return rv;
	}
	GWEN_Buffer_AppendByte(pbuf, '/');
	GWEN_Buffer_AppendString(pbuf, name);
	GWEN_PluginDescription_List2Iterator_free(pit);
	GWEN_PluginDescription_List2_freeAll(pl);
	return 0;
      }
      pd=GWEN_PluginDescription_List2Iterator_Next(pit);
    }
    GWEN_PluginDescription_List2Iterator_free(pit);
  } /* if no frontend list */

  /* check for every given frontend */
  s=strdup(frontends);

  p=s;
  while(*p) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;
    GWEN_PLUGIN_DESCRIPTION *pd;
    char *t;

    t=strchr(p, ';');
    if (t)
      *(t++)=0;

    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Trying frontend \"%s\"", p);

    pit=GWEN_PluginDescription_List2_First(pl);
    assert(pit);
    pd=GWEN_PluginDescription_List2Iterator_Data(pit);
    assert(pd);
    while(pd) {
      GWEN_XMLNODE *n;
      const char *fr;

      n=GWEN_PluginDescription_GetXmlNode(pd);
      assert(n);
      fr=GWEN_XMLNode_GetProperty(n, "frontend", "");
      if (-1!=GWEN_Text_ComparePattern(fr, p, 0)) {
	const char *name;

	name=GWEN_PluginDescription_GetName(pd);
	if (!name) {
	  DBG_WARN(AQBANKING_LOGDOMAIN,
		   "Found a plugin description with no name");
	}
	else {
	  int rv;

	  rv=AB_Banking__GetDebuggerPath(ab, backend, pbuf);
	  if (rv) {
	    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
	    return rv;
	  }
	  GWEN_Buffer_AppendByte(pbuf, '/');
	  GWEN_Buffer_AppendString(pbuf, name);
          free(s);
	  GWEN_PluginDescription_List2Iterator_free(pit);
	  GWEN_PluginDescription_List2_freeAll(pl);
	  return 0;
	}
      }
      pd=GWEN_PluginDescription_List2Iterator_Next(pit);
    } /* while pd */
    GWEN_PluginDescription_List2Iterator_free(pit);

    if (!t)
      break;
    p=t;
  } /* while */

  free(s);
  GWEN_PluginDescription_List2_freeAll(pl);
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No matching debugger found");
  return -1;
}



int AB_Banking_FindWizard(AB_BANKING *ab,
			  const char *backend,
			  const char *frontends,
			  GWEN_BUFFER *pbuf){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pl;
  char *s;
  char *p;

  pl=AB_Banking_GetWizardDescrs(ab, backend);
  if (!pl) {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "No debuggers available for backend \"%s\"", backend);
    return -1;
  }

  if (frontends==0) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;
    GWEN_PLUGIN_DESCRIPTION *pd;
    const char *name;

    pit=GWEN_PluginDescription_List2_First(pl);
    assert(pit);
    pd=GWEN_PluginDescription_List2Iterator_Data(pit);
    while(pd) {
      name=GWEN_PluginDescription_GetName(pd);
      if (!name) {
	DBG_WARN(AQBANKING_LOGDOMAIN,
		 "Found a plugin description with no name");
      }
      else {
	int rv;

	rv=AB_Banking__GetWizardPath(ab, backend, pbuf);
	if (rv) {
	  DBG_INFO(AQBANKING_LOGDOMAIN, "here");
	  return rv;
	}
	GWEN_Buffer_AppendByte(pbuf, '/');
	GWEN_Buffer_AppendString(pbuf, name);
	GWEN_PluginDescription_List2Iterator_free(pit);
	GWEN_PluginDescription_List2_freeAll(pl);
	return 0;
      }
      pd=GWEN_PluginDescription_List2Iterator_Next(pit);
    }
    GWEN_PluginDescription_List2Iterator_free(pit);
  } /* if no frontend list */

  /* check for every given frontend */
  s=strdup(frontends);

  p=s;
  while(*p) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;
    GWEN_PLUGIN_DESCRIPTION *pd;
    char *t;

    t=strchr(p, ';');
    if (t)
      *(t++)=0;

    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Trying frontend \"%s\"", p);

    pit=GWEN_PluginDescription_List2_First(pl);
    assert(pit);
    pd=GWEN_PluginDescription_List2Iterator_Data(pit);
    assert(pd);
    while(pd) {
      GWEN_XMLNODE *n;
      const char *fr;

      n=GWEN_PluginDescription_GetXmlNode(pd);
      assert(n);
      fr=GWEN_XMLNode_GetProperty(n, "frontend", "");
      if (-1!=GWEN_Text_ComparePattern(fr, p, 0)) {
	const char *name;

	name=GWEN_PluginDescription_GetName(pd);
	if (!name) {
	  DBG_WARN(AQBANKING_LOGDOMAIN,
		   "Found a plugin description with no name");
	}
	else {
	  int rv;

	  rv=AB_Banking__GetWizardPath(ab, backend, pbuf);
	  if (rv) {
	    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
	    return rv;
	  }
	  GWEN_Buffer_AppendByte(pbuf, '/');
	  GWEN_Buffer_AppendString(pbuf, name);
          free(s);
	  GWEN_PluginDescription_List2Iterator_free(pit);
	  GWEN_PluginDescription_List2_freeAll(pl);
	  return 0;
	}
      }
      pd=GWEN_PluginDescription_List2Iterator_Next(pit);
    } /* while pd */
    GWEN_PluginDescription_List2Iterator_free(pit);

    if (!t)
      break;
    p=t;
  } /* while */

  free(s);
  GWEN_PluginDescription_List2_freeAll(pl);
  DBG_ERROR(AQBANKING_LOGDOMAIN, "No matching debugger found");
  return -1;
}



int AB_Banking_SuspendProvider(AB_BANKING *ab, const char *backend){
  AB_PROVIDER *pro;

  pro=AB_Banking_FindProvider(ab, backend);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider \"%s\" not found", backend);
    return AB_ERROR_NOT_FOUND;
  }

  return AB_Banking_FiniProvider(ab, pro);
}



int AB_Banking_ResumeProvider(AB_BANKING *ab, const char *backend){
  AB_PROVIDER *pro;

  pro=AB_Banking_FindProvider(ab, backend);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider \"%s\" not found", backend);
    return AB_ERROR_NOT_FOUND;
  }

  return AB_Banking_InitProvider(ab, pro);
}



int AB_Banking_IsProviderActive(AB_BANKING *ab, const char *backend){
  AB_PROVIDER *pro;

  pro=AB_Banking_FindProvider(ab, backend);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider \"%s\" not found", backend);
    return 0;
  }

  return AB_Provider_IsInit(pro);
}



void AB_Banking__AddJobDir(const AB_BANKING *ab,
			   const char *as,
			   GWEN_BUFFER *buf) {
  AB_Banking_GetUserDataDir(ab, buf);
  GWEN_Buffer_AppendString(buf, DIRSEP "jobs" DIRSEP);
  GWEN_Buffer_AppendString(buf, as);
}



void AB_Banking__AddJobPath(const AB_BANKING *ab,
			    const char *as,
			    GWEN_TYPE_UINT32 jid,
			    GWEN_BUFFER *buf) {
  char buffer[16];

  AB_Banking__AddJobDir(ab, as, buf);
  GWEN_Buffer_AppendString(buf, DIRSEP);
  snprintf(buffer, sizeof(buffer), "%08lx", (unsigned long)jid);
  GWEN_Buffer_AppendString(buf, buffer);
  GWEN_Buffer_AppendString(buf, ".job");
}



int AB_Banking__OpenFile(const char *s, int wr) {
#ifndef OS_WIN32
  struct flock fl;
#endif
  int fd;

  if (wr) {
    if (GWEN_Directory_GetPath(s,
			       GWEN_DB_FLAGS_DEFAULT |
			       GWEN_PATH_FLAGS_VARIABLE)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not create path \"%s\"", s);
      return -1;
    }
    fd=open(s,
	    O_RDWR | O_CREAT,
	    S_IRUSR | S_IWUSR);
  }
  else {
    fd=open(s, O_RDONLY);
  }

  if (fd==-1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "open(%s): %s", s, strerror(errno));
    return -1;
  }

#ifndef OS_WIN32
  /* lock file for reading or writing */
  memset(&fl, 0, sizeof(fl));
  fl.l_type=wr?F_WRLCK:F_RDLCK;
  fl.l_whence=SEEK_SET;
  fl.l_start=0;
  fl.l_len=0;
  if (fcntl(fd, F_SETLKW, &fl)) {
# ifdef ENOLCK
    if (errno!=ENOLCK) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"fcntl(%s, F_SETLKW): %s", s, strerror(errno));
      close(fd);
      return -1;
    }
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Advisory locking is not supported at this file location.");
# else
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "fcntl(%s, F_SETLKW): %s", s, strerror(errno));
    close(fd);
    return -1;
# endif
  }
#endif

  return fd;
}



int AB_Banking__CloseFile(int fd){
#ifndef OS_WIN32
  struct flock fl;
#endif

  if (fd==-1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "File is not open");
    return -1;
  }

#ifndef OS_WIN32
  /* unlock file */
  memset(&fl, 0, sizeof(fl));
  fl.l_type=F_UNLCK;
  fl.l_whence=SEEK_SET;
  fl.l_start=0;
  fl.l_len=0;
  if (fcntl(fd, F_SETLK, &fl)) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "fcntl(%d, F_SETLK): %s",
	     fd, strerror(errno));
  }
#endif

  if (close(fd)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "close(%d): %s", fd, strerror(errno));
    return -1;
  }

  return 0;
}




int AB_Banking__OpenJobAs(AB_BANKING *ab,
			  GWEN_TYPE_UINT32 jid,
			  const char *as,
			  int wr){
  int fd;
  GWEN_BUFFER *pbuf;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AB_Banking__AddJobPath(ab, as, jid, pbuf);

  fd=AB_Banking__OpenFile(GWEN_Buffer_GetStart(pbuf), wr);
  if (fd!=-1 && wr)
    ftruncate(fd, 0);
  GWEN_Buffer_free(pbuf);

  return fd;
}



int AB_Banking__CloseJob(const AB_BANKING *ab, int fd){
  return AB_Banking__CloseFile(fd);
}



AB_JOB *AB_Banking__LoadJobFile(AB_BANKING *ab, const char *s){
  GWEN_DB_NODE *dbJob;
  AB_JOB *j;
  int fd;
  GWEN_BUFFEREDIO *bio;


  fd=AB_Banking__OpenFile(s, 0);
  if (fd==-1) {
    return 0;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);
  GWEN_BufferedIO_SubFlags(bio, GWEN_BUFFEREDIO_FLAGS_CLOSE);

  dbJob=GWEN_DB_Group_new("job");
  if (GWEN_DB_ReadFromStream(dbJob, bio,
			     GWEN_DB_FLAGS_DEFAULT |
			     GWEN_PATH_FLAGS_CREATE_GROUP)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error reading job data");
    GWEN_DB_Group_free(dbJob);
    GWEN_BufferedIO_free(bio);
    AB_Banking__CloseJob(ab, fd);
    return 0;
  }

  j=AB_Job_fromDb(ab, dbJob);
  GWEN_DB_Group_free(dbJob);
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  if (AB_Banking__CloseFile(fd)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error closing job, ignoring");
  }
  return j;
}



AB_JOB *AB_Banking__LoadJobAs(AB_BANKING *ab,
			      GWEN_TYPE_UINT32 jid,
                              const char *as){
  GWEN_DB_NODE *dbJob;
  AB_JOB *j;
  int fd;
  GWEN_BUFFEREDIO *bio;


  fd=AB_Banking__OpenJobAs(ab, jid, as, 0);
  if (fd==-1) {
    return 0;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);
  GWEN_BufferedIO_SubFlags(bio, GWEN_BUFFEREDIO_FLAGS_CLOSE);

  dbJob=GWEN_DB_Group_new("job");
  if (GWEN_DB_ReadFromStream(dbJob, bio,
			     GWEN_DB_FLAGS_DEFAULT |
			     GWEN_PATH_FLAGS_CREATE_GROUP)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error reading job data");
    GWEN_DB_Group_free(dbJob);
    GWEN_BufferedIO_free(bio);
    AB_Banking__CloseJob(ab, fd);
    return 0;
  }

  j=AB_Job_fromDb(ab, dbJob);
  GWEN_DB_Group_free(dbJob);
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  if (AB_Banking__CloseJob(ab, fd)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error closing job, ignoring");
  }
  return j;
}



int AB_Banking__SaveJobAs(AB_BANKING *ab,
			  AB_JOB *j,
			  const char *as){
  GWEN_DB_NODE *dbJob;
  int fd;
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;

  dbJob=GWEN_DB_Group_new("job");
  if (AB_Job_toDb(j, dbJob)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not store job");
    GWEN_DB_Group_free(dbJob);
    return -1;
  }

  fd=AB_Banking__OpenJobAs(ab,
			   AB_Job_GetJobId(j),
			   as, 1);
  if (fd==-1) {
    GWEN_DB_Group_free(dbJob);
    return -1;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);
  GWEN_BufferedIO_SubFlags(bio, GWEN_BUFFEREDIO_FLAGS_CLOSE);

  if (GWEN_DB_WriteToStream(dbJob, bio,
			    GWEN_DB_FLAGS_DEFAULT)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error reading job data");
    GWEN_DB_Group_free(dbJob);
    GWEN_BufferedIO_free(bio);
    AB_Banking__CloseJob(ab, fd);
    return -1;
  }

  GWEN_DB_Group_free(dbJob);
  err=GWEN_BufferedIO_Flush(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
    GWEN_BufferedIO_free(bio);
    AB_Banking__CloseJob(ab, fd);
    return -1;
  }
  GWEN_BufferedIO_free(bio);
  if (AB_Banking__CloseJob(ab, fd)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error closing job");
    return -1;
  }
  return 0;
}



int AB_Banking__UnlinkJobAs(AB_BANKING *ab,
			    AB_JOB *j,
			    const char *as){
  int fd;
  GWEN_BUFFER *pbuf;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AB_Banking__AddJobPath(ab, as, AB_Job_GetJobId(j), pbuf);

  fd=AB_Banking__OpenFile(GWEN_Buffer_GetStart(pbuf), 0);
  if (fd!=-1) {
#ifdef OS_WIN32
    /* For windows, first close the file, then unlink it */
    AB_Banking__CloseFile(fd);
    if (unlink(GWEN_Buffer_GetStart(pbuf))) {
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "unlink(%s): %s",
                GWEN_Buffer_GetStart(pbuf),
                strerror(errno));
      GWEN_Buffer_free(pbuf);
      return AB_ERROR_GENERIC;
    }
#else
    if (unlink(GWEN_Buffer_GetStart(pbuf))) {
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "unlink(%s): %s",
                GWEN_Buffer_GetStart(pbuf),
                strerror(errno));
      GWEN_Buffer_free(pbuf);
      AB_Banking__CloseFile(fd);
      return AB_ERROR_GENERIC;
    }
    AB_Banking__CloseFile(fd);
#endif
  }

  GWEN_Buffer_free(pbuf);

  return 0;
}




AB_JOB_LIST2 *AB_Banking__LoadJobsAs(AB_BANKING *ab, const char *as) {
  GWEN_BUFFER *pbuf;
  AB_JOB_LIST2 *l;
  GWEN_DIRECTORYDATA *d;
  GWEN_TYPE_UINT32 pos;

  l=AB_Job_List2_new();

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AB_Banking__AddJobDir(ab, as, pbuf);
  pos=GWEN_Buffer_GetPos(pbuf);

  d=GWEN_Directory_new();
  if (!GWEN_Directory_Open(d, GWEN_Buffer_GetStart(pbuf))) {
    char nbuffer[256];

    while(!GWEN_Directory_Read(d, nbuffer, sizeof(nbuffer))) {
      int i;

      i=strlen(nbuffer);
      if (i>4) {
	if (strcmp(nbuffer+i-4, ".job")==0) {
	  AB_JOB *j;

	  GWEN_Buffer_Crop(pbuf, 0, pos);
	  GWEN_Buffer_AppendString(pbuf, DIRSEP);
	  GWEN_Buffer_AppendString(pbuf, nbuffer);

	  /* job found */
	  j=AB_Banking__LoadJobFile(ab, GWEN_Buffer_GetStart(pbuf));
	  if (!j) {
	    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in job file \"%s\"",
		      GWEN_Buffer_GetStart(pbuf));
	  }
	  else {
	    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding job \"%s\"", GWEN_Buffer_GetStart(pbuf));
	    AB_Job_List2_PushBack(l, j);
	  }
	} /* if filename ends in ".job" */
      } /* if filename is long enough */
    } /* while still jobs */
    if (GWEN_Directory_Close(d)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error closing dir");
      AB_Job_List2_free(l);
      GWEN_Buffer_free(pbuf);
      return 0;
    }
  } /* if open */
  GWEN_Directory_free(d);
  GWEN_Buffer_free(pbuf);

  if (AB_Job_List2_GetSize(l)==0) {
    AB_Job_List2_free(l);
    return 0;
  }

  return l;
}



AB_JOB_LIST2 *AB_Banking_GetFinishedJobs(AB_BANKING *ab) {
  return AB_Banking__LoadJobsAs(ab, "finished");
}



int AB_Banking_DelFinishedJob(AB_BANKING *ab, AB_JOB *j){
  int rv=AB_ERROR_INVALID;

  assert(ab);
  assert(j);
  /* First check whether this job actually has sane data */
  if (ab->appName && AB_Job_GetCreatedBy(j)) {
    if (strcasecmp(ab->appName, AB_Job_GetCreatedBy(j))==0) {
      rv=AB_Banking__SaveJobAs(ab, j, "archived");
      if (rv) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Could not store job in archive (%d)", rv);
	return rv;
      }
      rv=AB_Banking__UnlinkJobAs(ab, j, "finished");
    }
    else {
      DBG_WARN(AQBANKING_LOGDOMAIN,
		"Job can only be removed by its creator application");
    }
  }
  return rv;
}



AB_JOB_LIST2 *AB_Banking_GetArchivedJobs(AB_BANKING *ab){
  return AB_Banking__LoadJobsAs(ab, "archived");
}



int AB_Banking_DelArchivedJob(AB_BANKING *ab, AB_JOB *j){
  int rv=AB_ERROR_INVALID;

  assert(ab);
  assert(j);
  /* First check whether this job actually has sane data */
  if (ab->appName && AB_Job_GetCreatedBy(j)) {
    if (strcasecmp(ab->appName, AB_Job_GetCreatedBy(j))==0) {
      rv=AB_Banking__UnlinkJobAs(ab, j, "archived");
    }
    else {
      DBG_WARN(AQBANKING_LOGDOMAIN,
		"Job can only be removed by its creator application");
    }
  }
  return rv;
}




AB_JOB_LIST2 *AB_Banking_GetPendingJobs(AB_BANKING *ab) {
  return AB_Banking__LoadJobsAs(ab, "pending");
}



int AB_Banking_DelPendingJob(AB_BANKING *ab, AB_JOB *j){
  int rv=AB_ERROR_INVALID;

  assert(ab);
  assert(j);
  /* First check whether this job actually has sane data */
  if (ab->appName && AB_Job_GetCreatedBy(j)) {
    if (strcasecmp(ab->appName, AB_Job_GetCreatedBy(j))==0) {
      rv=AB_Banking__SaveJobAs(ab, j, "archived");
      if (rv) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Could not store job in archive (%d)", rv);
	return rv;
      }
      rv=AB_Banking__UnlinkJobAs(ab, j, "pending");
    }
    else {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Job can only be removed by its creator application");
    }
  }
  return rv;
}



AB_JOB_LIST2 *AB_Banking_GetDeferredJobs(AB_BANKING *ab) {
  return AB_Banking__LoadJobsAs(ab, "deferred");
}



int AB_Banking_DelDeferredJob(AB_BANKING *ab, AB_JOB *j){
  int rv=AB_ERROR_INVALID;

  assert(ab);
  assert(j);
  /* First check whether this job actually has sane data */
  if (ab->appName && AB_Job_GetCreatedBy(j)) {
    if (strcasecmp(ab->appName, AB_Job_GetCreatedBy(j))==0) {
      rv=AB_Banking__SaveJobAs(ab, j, "archived");
      if (rv) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Could not store job in archive (%d)", rv);
	return rv;
      }
      rv=AB_Banking__UnlinkJobAs(ab, j, "deferred");
    }
    else {
      DBG_WARN(AQBANKING_LOGDOMAIN,
		"Job can only be removed by its creator application");
    }
  }
  return rv;
}



void *AB_Banking_GetUserData(AB_BANKING *ab) {
  assert(ab);
  return ab->user_data;
}


void AB_Banking_SetUserData(AB_BANKING *ab, void *user_data) {
  assert(ab);
  ab->user_data = user_data;
}



AB_IMEXPORTER *AB_Banking_FindImExporter(AB_BANKING *ab, const char *name) {
  AB_IMEXPORTER *ie;

  assert(ab);
  assert(name);
  ie=AB_ImExporter_List_First(ab->imexporters);
  while(ie) {
    if (strcasecmp(AB_ImExporter_GetName(ie), name)==0)
      break;
    ie=AB_ImExporter_List_Next(ie);
  } /* while */

  return ie;
}



AB_IMEXPORTER *AB_Banking_GetImExporter(AB_BANKING *ab, const char *name){
  AB_IMEXPORTER *ie;

  assert(ab);
  assert(name);

  ie=AB_Banking_FindImExporter(ab, name);
  if (ie)
    return ie;
  ie=AB_Banking_LoadImExporterPlugin(ab, name);
  if (ie) {
    AB_ImExporter_List_Add(ie, ab->imexporters);
  }

  return ie;
}



AB_IMEXPORTER *AB_Banking_LoadImExporterPluginFile(AB_BANKING *ab,
                                                   const char *modname,
                                                   const char *fname){
  GWEN_LIBLOADER *ll;
  AB_IMEXPORTER *ie;
  AB_IMEXPORTER_FACTORY_FN fn;
  void *p;
  GWEN_BUFFER *nbuf;
  const char *s;
  GWEN_ERRORCODE err;
  GWEN_DB_NODE *db;

  ll=GWEN_LibLoader_new();
  if (GWEN_LibLoader_OpenLibrary(ll, fname)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not load provider plugin \"%s\" (%s)",
              modname, fname);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* create name of init function */
  nbuf=GWEN_Buffer_new(0, 128, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(nbuf, tolower(*(s++)));
  GWEN_Buffer_AppendString(nbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(nbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
    GWEN_Buffer_free(nbuf);
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(nbuf);

  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "static/imexporters");
  assert(db);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT, modname);
  assert(db);

  fn=(AB_IMEXPORTER_FACTORY_FN)p;
  assert(fn);
  ie=fn(ab, db);
  if (!ie) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in plugin: No im/exporter created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  AB_ImExporter_SetLibLoader(ie, ll);

  return ie;
}



AB_IMEXPORTER *AB_Banking_LoadImExporterPlugin(AB_BANKING *ab,
                                               const char *modname){
  GWEN_LIBLOADER *ll;
  AB_IMEXPORTER *ie;
  AB_IMEXPORTER_FACTORY_FN fn;
  void *p;
  const char *s;
  GWEN_ERRORCODE err = 0;
  GWEN_BUFFER *mbuf;
  GWEN_DB_NODE *db;
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_STRINGLISTENTRY *sentry;
  const char *dirpath = 0;

  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(mbuf, tolower(*(s++)));
  modname=GWEN_Buffer_GetStart(mbuf);

  /* New loading code -- use path list from PluginManager but don't
     use its loading code */
  pm = GWEN_PluginManager_FindPluginManager("imexporters");
  if (!pm) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not find plugin manager for \"%s\"",
              "imexporters");
    return 0;
  }

  ll = GWEN_LibLoader_new();
  sentry = GWEN_StringList_FirstEntry(GWEN_PluginManager_GetPaths(pm));
  while (sentry) {
    dirpath = GWEN_StringListEntry_Data(sentry);
    assert(dirpath);
    err = GWEN_LibLoader_OpenLibraryWithPath(ll, dirpath, modname);
    if (GWEN_Error_IsOk(err))
      break;
    else {
      /* DBG_INFO_ERR(AQBANKING_LOGDOMAIN, err); */
    }
    sentry = GWEN_StringListEntry_Next(sentry);
  }

  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(AQBANKING_LOGDOMAIN, err);
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load provider plugin \"%s\"", modname);
    GWEN_Buffer_free(mbuf);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* create name of init function */
  GWEN_Buffer_AppendString(mbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(mbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
    GWEN_Buffer_free(mbuf);
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(mbuf);

  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "static/imexporters");
  assert(db);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT, modname);
  assert(db);

  fn=(AB_IMEXPORTER_FACTORY_FN)p;
  assert(fn);
  ie=fn(ab, db);
  if (!ie) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in plugin: No im/exporter created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  AB_ImExporter_SetLibLoader(ie, ll);

  return ie;
}






/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             High Level API
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



AB_ACCOUNT *AB_Banking__GetAccount(AB_BANKING *ab, const char *accountId){
  GWEN_DB_NODE *dbData;
  GWEN_TYPE_UINT32 uniqueId;
  AB_ACCOUNT *a;

  uniqueId=0;
  dbData=AB_Banking_GetAppData(ab);
  dbData=GWEN_DB_GetGroup(dbData, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			  "banking/aliases");
  if (dbData)
    uniqueId=GWEN_DB_GetIntValue(dbData, accountId, 0, 0);
  if (!uniqueId) {
    /* should not happen anyway */
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Account has no unique id. Should not happen");
    return 0;
  }

  a=AB_Banking_GetAccount(ab, uniqueId);
  if (!a) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Account with alias \"%s\" not found",
	      accountId);
    return 0;
  }

  return a;
}



AB_ACCOUNT *AB_Banking_GetAccountByAlias(AB_BANKING *ab,
                                         const char *accountId){
  return AB_Banking__GetAccount(ab, accountId);
}



void AB_Banking_SetAccountAlias(AB_BANKING *ab,
				AB_ACCOUNT *a, const char *alias){
  GWEN_DB_NODE *dbData;

  assert(a);
  assert(alias);

  dbData=AB_Banking_GetAppData(ab);
  dbData=GWEN_DB_GetGroup(dbData, GWEN_DB_FLAGS_DEFAULT,
			  "banking/aliases");
  assert(dbData);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      alias,
		      AB_Account_GetUniqueId(a));
}



int AB_Banking_RequestBalance(AB_BANKING *ab,
                              const char *bankCode,
                              const char *accountNumber) {
  AB_ACCOUNT *a;
  AB_JOB *j;
  int rv;

  if (!accountNumber) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Account number is required");
    return AB_ERROR_INVALID;
  }

  a=AB_Banking_GetAccountByCodeAndNumber(ab, bankCode, accountNumber);
  if (!a)
    return AB_ERROR_INVALID;

  /* TODO: check if there already is such a job in the queue */

  j=AB_JobGetBalance_new(a);
  assert(j);
  rv=AB_Job_CheckAvailability(j);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Job not available with the backend for this account (%d)",
	      rv);
    AB_Banking_MessageBox(ab,
			  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			  AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
			  I18N("Unsupported Request"),
			  I18N("The backend for this banking account "
			       "does not support your request."),
			  I18N("Dismiss"), 0, 0);
    AB_Job_free(j);
    return AB_ERROR_GENERIC;
  }

  rv=AB_Banking_EnqueueJob(ab, j);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not enqueue the job (%d)",
	      rv);
    AB_Banking_MessageBox(ab,
			  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			  AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                          I18N("Queue Error"),
			  I18N("Unable to enqueue your request."),
			  I18N("Dismiss"), 0, 0);
    AB_Job_free(j);
    return AB_ERROR_GENERIC;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN,
	   "Job successfully enqueued");
  AB_Job_free(j);
  return 0;
}



int AB_Banking_RequestTransactions(AB_BANKING *ab,
                                   const char *bankCode,
                                   const char *accountNumber,
				   const GWEN_TIME *firstDate,
				   const GWEN_TIME *lastDate){
  AB_ACCOUNT *a;
  AB_JOB *j;
  int rv;

  if (!accountNumber) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Account number is required");
    return AB_ERROR_INVALID;
  }

  a=AB_Banking_GetAccountByCodeAndNumber(ab, bankCode, accountNumber);
  if (!a)
    return AB_ERROR_INVALID;

  /* TODO: check if there already is such a job in the queue */

  j=AB_JobGetTransactions_new(a);
  assert(j);
  rv=AB_Job_CheckAvailability(j);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Job not available with the backend for this account (%d)",
	      rv);
    AB_Banking_MessageBox(ab,
			  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			  AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
			  I18N("Unsupported Request"),
			  I18N("The backend for this banking account "
			       "does not support your request."),
			  I18N("Dismiss"), 0, 0);
    AB_Job_free(j);
    return AB_ERROR_GENERIC;
  }

  if (firstDate)
    AB_JobGetTransactions_SetFromTime(j, firstDate);
  if (lastDate)
    AB_JobGetTransactions_SetToTime(j, lastDate);

  rv=AB_Banking_EnqueueJob(ab, j);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not enqueue the job (%d)",
	      rv);
    AB_Banking_MessageBox(ab,
			  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			  AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                          I18N("Queue Error"),
			  I18N("Unable to enqueue your request."),
			  I18N("Dismiss"), 0, 0);
    AB_Job_free(j);
    return AB_ERROR_GENERIC;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN,
	   "Job successfully enqueued");
  AB_Job_free(j);
  return 0;

}



int AB_Banking_RequestStandingOrders(AB_BANKING *ab,
                                     const char *bankCode,
                                     const char *accountNumber) {
  AB_ACCOUNT *a;
  AB_JOB *j;
  int rv;

  if (!accountNumber) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Account number is required");
    return AB_ERROR_INVALID;
  }

  a=AB_Banking_GetAccountByCodeAndNumber(ab, bankCode, accountNumber);
  if (!a)
    return AB_ERROR_INVALID;

  /* TODO: check if there already is such a job in the queue */

  j=AB_JobGetStandingOrders_new(a);
  assert(j);
  rv=AB_Job_CheckAvailability(j);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Job not available with the backend for this account (%d)",
	      rv);
    AB_Banking_MessageBox(ab,
			  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			  AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
			  I18N("Unsupported Request"),
			  I18N("The backend for this banking account "
			       "does not support your request."),
			  I18N("Dismiss"), 0, 0);
    AB_Job_free(j);
    return AB_ERROR_GENERIC;
  }

  rv=AB_Banking_EnqueueJob(ab, j);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not enqueue the job (%d)",
	      rv);
    AB_Banking_MessageBox(ab,
			  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			  AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                          I18N("Queue Error"),
			  I18N("Unable to enqueue your request."),
			  I18N("Dismiss"), 0, 0);
    AB_Job_free(j);
    return AB_ERROR_GENERIC;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN,
	   "Job successfully enqueued");
  AB_Job_free(j);
  return 0;
}



int AB_Banking_RequestDatedTransfers(AB_BANKING *ab,
                                     const char *bankCode,
                                     const char *accountNumber) {
  AB_ACCOUNT *a;
  AB_JOB *j;
  int rv;

  if (!accountNumber) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Account number is required");
    return AB_ERROR_INVALID;
  }

  a=AB_Banking_GetAccountByCodeAndNumber(ab, bankCode, accountNumber);
  if (!a)
    return AB_ERROR_INVALID;

  /* TODO: check if there already is such a job in the queue */

  j=AB_JobGetDatedTransfers_new(a);
  assert(j);
  rv=AB_Job_CheckAvailability(j);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Job not available with the backend for this account (%d)",
	      rv);
    AB_Banking_MessageBox(ab,
			  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			  AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
			  I18N("Unsupported Request"),
			  I18N("The backend for this banking account "
			       "does not support your request."),
			  I18N("Dismiss"), 0, 0);
    AB_Job_free(j);
    return AB_ERROR_GENERIC;
  }

  rv=AB_Banking_EnqueueJob(ab, j);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not enqueue the job (%d)",
	      rv);
    AB_Banking_MessageBox(ab,
			  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			  AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                          I18N("Queue Error"),
			  I18N("Unable to enqueue your request."),
			  I18N("Dismiss"), 0, 0);
    AB_Job_free(j);
    return AB_ERROR_GENERIC;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN,
	   "Job successfully enqueued");
  AB_Job_free(j);
  return 0;
}



int AB_Banking__isSameDay(const GWEN_TIME *t1, const GWEN_TIME *t2) {
  if (t1 && t2) {
    GWEN_BUFFER *d1, *d2;
    int i;

    d1=GWEN_Buffer_new(0, 16, 0, 1);
    d2=GWEN_Buffer_new(0, 16, 0, 1);
    GWEN_Time_toString(t1, "YYYYMMDD", d1);
    GWEN_Time_toString(t2, "YYYYMMDD", d2);
    i=strcasecmp(GWEN_Buffer_GetStart(d1),
		 GWEN_Buffer_GetStart(d2));
    GWEN_Buffer_free(d2);
    GWEN_Buffer_free(d1);
    return (i==0);
  }
  else {
    return 0;
  }
}


void AB_Banking__RemoveDuplicateJobs(AB_BANKING *ab, AB_JOB_LIST2 *jl) {
  AB_JOB *j;
  AB_JOB_LIST2_ITERATOR *jit;

  for (;;) {
    int removed=0;

    jit=AB_Job_List2_First(jl);
    if (!jit)
      break;
    j=AB_Job_List2Iterator_Data(jit);
    assert(j);

    while(j) {
      AB_JOB *j2;
      AB_JOB_LIST2_ITERATOR *jit2;
      AB_JOB_TYPE jt;
      const char *appName;

      appName=AB_Job_GetCreatedBy(j);
      jt=AB_Job_GetType(j);
      jit2=AB_Job_List2_First(jl);
      assert(jit2);
      j2=AB_Job_List2Iterator_Data(jit2);
      assert(j2);
      while(j2) {
	if (AB_Job_GetJobId(j2)!=AB_Job_GetJobId(j)) {
	  /* not the same job */
	  if ((AB_Job_GetType(j)==AB_Job_TypeGetTransactions) &&
	      (AB_Job_GetType(j2)==AB_Job_TypeGetTransactions)) {
	    /* ... but the same type */
	    if ((AB_Job_GetAccount(j)==AB_Job_GetAccount(j2))
		&&
		AB_Banking__isSameDay(AB_Job_GetLastStatusChange(j),
				      AB_Job_GetLastStatusChange(j2))
                &&
		AB_Banking__isSameDay(AB_JobGetTransactions_GetFromTime(j),
				      AB_JobGetTransactions_GetFromTime(j2))
		&&
		AB_Banking__isSameDay(AB_JobGetTransactions_GetFromTime(j),
				      AB_JobGetTransactions_GetFromTime(j2))){
	      DBG_ERROR(AQBANKING_LOGDOMAIN, "Erasing a job");
	      if (appName) {
		if (strcasecmp(appName, AB_Banking_GetAppName(ab))==0) {
		  int rv;
	
		  /* hey job: I created you, I can destroy you ;-) */
		  rv=AB_Banking_DelFinishedJob(ab, j);
		  if (rv) {
		    DBG_INFO(AQBANKING_LOGDOMAIN,
			     "Could not delete finished job (%d)", rv)
		  }
		} /* if it is our own job */
	      } /* if appName */
	      /* same job */
	      AB_Job_List2_Erase(jl, jit);
	      removed=1;
	      break;
	    }
	  }
	}
	j2=AB_Job_List2Iterator_Next(jit2);
      }

      j=AB_Job_List2Iterator_Next(jit);
    } /* while */
    if (!removed)
      break;
  } /* for */
}



int AB_Banking_GatherJobListResponses(AB_BANKING *ab,
                                      AB_JOB_LIST2 *jl,
                                      AB_IMEXPORTER_CONTEXT *ctx,
                                      int jm) {
  AB_JOB *j;
  AB_JOB_LIST2_ITERATOR *jit;


  jit=AB_Job_List2_First(jl);
  if (!jit) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "No jobs left");
    return 0;
  }

  j=AB_Job_List2Iterator_Data(jit);
  assert(j);
  while(j) {
    AB_IMEXPORTER_ACCOUNTINFO *ai;
    AB_ACCOUNT *a;
    int tryRemove;

    tryRemove=0;
    a=AB_Job_GetAccount(j);
    assert(a);
    ai=AB_ImExporterContext_GetAccountInfo(ctx,
                                           AB_Account_GetBankCode(a),
                                           AB_Account_GetAccountNumber(a));
    assert(ai);
    if (AB_Job_GetType(j)==AB_Job_TypeGetBalance) {
      const AB_ACCOUNT_STATUS *ast;

      ast=AB_JobGetBalance_GetAccountStatus(j);
      if (ast) {
        AB_ImExporterAccountInfo_AddAccountStatus(ai,
                                                  AB_AccountStatus_dup(ast));
      }
      tryRemove=1;
    }
    else if (AB_Job_GetType(j)==AB_Job_TypeGetTransactions) {
      AB_TRANSACTION_LIST2 *tl;

      tl=AB_JobGetTransactions_GetTransactions(j);
      if (tl) {
        AB_TRANSACTION_LIST2_ITERATOR *it;
        AB_TRANSACTION *t;

        it=AB_Transaction_List2_First(tl);
        assert(it);
        t=AB_Transaction_List2Iterator_Data(it);
        assert(t);
        while(t) {
	  AB_TRANSACTION *nt;

          DBG_NOTICE(AQBANKING_LOGDOMAIN, "Got a transaction");
	  nt=AB_Transaction_dup(t);
	  AB_Transaction_SetLocalAccountNumber(nt,
					       AB_Account_GetAccountNumber(a));
	  AB_Transaction_SetLocalBankCode(nt, AB_Account_GetBankCode(a));

	  AB_ImExporterAccountInfo_AddTransaction(ai, nt);
	  t=AB_Transaction_List2Iterator_Next(it);
	} /* while */
        AB_Transaction_List2Iterator_free(it);
      }

      tryRemove=1;
    }
    else if (AB_Job_GetType(j)==AB_Job_TypeGetStandingOrders) {
      AB_TRANSACTION_LIST2 *tl;

      tl=AB_JobGetStandingOrders_GetStandingOrders(j);
      if (tl) {
        AB_TRANSACTION_LIST2_ITERATOR *it;
        AB_TRANSACTION *t;

        it=AB_Transaction_List2_First(tl);
        assert(it);
        t=AB_Transaction_List2Iterator_Data(it);
        assert(t);
        while(t) {
	  AB_TRANSACTION *nt;

          DBG_NOTICE(AQBANKING_LOGDOMAIN, "Got a standing order");
	  nt=AB_Transaction_dup(t);
	  AB_Transaction_SetLocalAccountNumber(nt,
					       AB_Account_GetAccountNumber(a));
	  AB_Transaction_SetLocalBankCode(nt, AB_Account_GetBankCode(a));

	  AB_ImExporterAccountInfo_AddStandingOrder(ai, nt);
	  t=AB_Transaction_List2Iterator_Next(it);
	} /* while */
        AB_Transaction_List2Iterator_free(it);
      }

      tryRemove=1;
    }
    else if (AB_Job_GetType(j)==AB_Job_TypeGetDatedTransfers) {
      AB_TRANSACTION_LIST2 *tl;

      tl=AB_JobGetDatedTransfers_GetDatedTransfers(j);
      if (tl) {
        AB_TRANSACTION_LIST2_ITERATOR *it;
        AB_TRANSACTION *t;

        it=AB_Transaction_List2_First(tl);
        assert(it);
        t=AB_Transaction_List2Iterator_Data(it);
        assert(t);
        while(t) {
	  AB_TRANSACTION *nt;

          DBG_NOTICE(AQBANKING_LOGDOMAIN, "Got a standing order");
	  nt=AB_Transaction_dup(t);
	  AB_Transaction_SetLocalAccountNumber(nt,
					       AB_Account_GetAccountNumber(a));
	  AB_Transaction_SetLocalBankCode(nt, AB_Account_GetBankCode(a));

	  AB_ImExporterAccountInfo_AddDatedTransfer(ai, nt);
	  t=AB_Transaction_List2Iterator_Next(it);
	} /* while */
        AB_Transaction_List2Iterator_free(it);
      }

      tryRemove=1;
    }
    else if (AB_Job_GetType(j)==AB_Job_TypeTransfer) {
      const AB_TRANSACTION *t;

      t=AB_JobSingleTransfer_GetTransaction(j);
      if (t) {
        AB_TRANSACTION *nt;

        nt=AB_Transaction_dup(t);
        assert(nt);
        AB_Transaction_SetType(nt, AB_Transaction_TypeTransfer);
        switch(AB_Job_GetStatus(j)) {
        case AB_Job_StatusPending:
          AB_Transaction_SetStatus(nt, AB_Transaction_StatusPending);
          break;
        case AB_Job_StatusFinished:
          AB_Transaction_SetStatus(nt, AB_Transaction_StatusAccepted);
          break;
        case AB_Job_StatusError:
          AB_Transaction_SetStatus(nt, AB_Transaction_StatusRejected);
          break;
        default:
          AB_Transaction_SetStatus(nt, AB_Transaction_StatusUnknown);
        };
        AB_ImExporterAccountInfo_AddTransfer(ai, nt);
      }
      tryRemove=1;
    }
    else if (AB_Job_GetType(j)==AB_Job_TypeDebitNote) {
      const AB_TRANSACTION *t;

      t=AB_JobSingleDebitNote_GetTransaction(j);
      if (t) {
        AB_TRANSACTION *nt;

        nt=AB_Transaction_dup(t);
        assert(nt);
        AB_Transaction_SetType(nt, AB_Transaction_TypeDebitNote);
        switch(AB_Job_GetStatus(j)) {
        case AB_Job_StatusPending:
          AB_Transaction_SetStatus(nt, AB_Transaction_StatusPending);
          break;
        case AB_Job_StatusFinished:
          AB_Transaction_SetStatus(nt, AB_Transaction_StatusAccepted);
          break;
        case AB_Job_StatusError:
          AB_Transaction_SetStatus(nt, AB_Transaction_StatusRejected);
          break;
        default:
          AB_Transaction_SetStatus(nt, AB_Transaction_StatusUnknown);
        };
        AB_ImExporterAccountInfo_AddTransfer(ai, nt);
      }
      tryRemove=1;
    }
    else if (AB_Job_GetType(j)==AB_Job_TypeEuTransfer) {
      const AB_TRANSACTION *t;

      t=AB_JobEuTransfer_GetTransaction(j);
      if (t) {
        AB_TRANSACTION *nt;

        nt=AB_Transaction_dup(t);
        assert(nt);
        AB_Transaction_SetType(nt, AB_Transaction_TypeEuTransfer);
        switch(AB_Job_GetStatus(j)) {
        case AB_Job_StatusPending:
          AB_Transaction_SetStatus(nt, AB_Transaction_StatusPending);
          break;
        case AB_Job_StatusFinished:
          AB_Transaction_SetStatus(nt, AB_Transaction_StatusAccepted);
          break;
        case AB_Job_StatusError:
          AB_Transaction_SetStatus(nt, AB_Transaction_StatusRejected);
          break;
        default:
          AB_Transaction_SetStatus(nt, AB_Transaction_StatusUnknown);
        };
        AB_ImExporterAccountInfo_AddTransfer(ai, nt);
      }
      tryRemove=1;
    }
    else {
      tryRemove=1;
    }

    /* eventually remove the job from finished queue */
    if (tryRemove) {
      const char *appName;

      appName=AB_Job_GetCreatedBy(j);
      if (appName) {
        if (strcasecmp(appName, AB_Banking_GetAppName(ab))==0) {
          int rv;

          /* hey job: I created you, I can destroy you ;-) */
          if (jm==0) {
            rv=AB_Banking_DelFinishedJob(ab, j);
            if (rv) {
              DBG_INFO(AQBANKING_LOGDOMAIN,
                       "Could not delete finished job (%d)", rv)
            }
          }
        } /* if it is our own job */
      } /* if appName */
    } /* if tryRemove */

    j=AB_Job_List2Iterator_Next(jit);
  } /* while */
  AB_Job_List2Iterator_free(jit);

  return 0;
}



int AB_Banking_GatherResponses(AB_BANKING *ab,
			       AB_IMEXPORTER_CONTEXT *ctx) {
  AB_JOB_LIST2 *jl;
  int someOk=0;
  int rv;

  jl=AB_Banking_GetFinishedJobs(ab);
  if (jl) {
    AB_Banking__RemoveDuplicateJobs(ab, jl);
    rv=AB_Banking_GatherJobListResponses(ab, jl, ctx, 0);
    AB_Job_List2_FreeAll(jl);
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return rv;
    }
    someOk=1;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "No finished jobs");
  }

  jl=AB_Banking_GetPendingJobs(ab);
  if (jl) {
    AB_Banking__RemoveDuplicateJobs(ab, jl);
    rv=AB_Banking_GatherJobListResponses(ab, jl, ctx, 1);
    AB_Job_List2_FreeAll(jl);
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return rv;
    }
    someOk=1;
  }
  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN,
              "No pending jobs");
  }

  if (someOk)
    return 0;
  return AB_ERROR_NOT_FOUND;
}



void AB_Banking_SetGetPinFn(AB_BANKING *ab,
			    AB_BANKING_GETPIN_FN f){
  assert(ab);
  ab->getPinFn=f;
}



void AB_Banking_SetSetPinStatusFn(AB_BANKING *ab,
				  AB_BANKING_SETPINSTATUS_FN f){
  assert(ab);
  ab->setPinStatusFn=f;
}



void AB_Banking_SetGetTanFn(AB_BANKING *ab,
			    AB_BANKING_GETTAN_FN f){
  assert(ab);
  ab->getTanFn=f;
}



void AB_Banking_SetSetTanStatusFn(AB_BANKING *ab,
				  AB_BANKING_SETTANSTATUS_FN f){
  assert(ab);
  ab->setTanStatusFn=f;
}





int AB_Banking__GetPin(AB_BANKING *ab,
                       GWEN_TYPE_UINT32 flags,
                       const char *token,
                       const char *title,
                       const char *text,
                       char *buffer,
                       int minLen,
                       int maxLen){

  assert(ab);

  if (ab->getPinFn) {
    return ab->getPinFn(ab, flags, token, title, text, buffer,
                        minLen, maxLen);
  }
  else {
    return AB_Banking_InputBox(ab,
                               flags,
                               title,
                               text,
                               buffer,
                               minLen,
                               maxLen);
  }
}



int AB_Banking_GetPin(AB_BANKING *ab,
                      GWEN_TYPE_UINT32 flags,
                      const char *token,
		      const char *title,
		      const char *text,
		      char *buffer,
		      int minLen,
                      int maxLen){
  AB_PIN *p;
  int rv;
  int i;

  assert(ab);
  assert(token);

  /* check whether we already know the pin */
  p=AB_Pin_List_First(ab->pinList);
  while(p) {
    const char *s;

    s=AB_Pin_GetToken(p);
    if (s) {
      if (strcasecmp(s, token)==0) {
        break;
      }
    }
    p=AB_Pin_List_Next(p);
  }

  if (!p) {
    /* no pin yet, ask program for it */
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Have no pin for \"%s\", getting it",
	     token);

    rv=AB_Banking__GetPin(ab, flags, token, title, text, buffer,
                          minLen, maxLen);
    if (rv)
      return rv;
    p=AB_Pin_new();
    AB_Pin_SetToken(p, token);
    AB_Pin_SetValue(p, buffer);
    AB_Pin_SetHash(p, 0);
    AB_Pin_SetStatus(p, "unknown");
    DBG_DEBUG(AQBANKING_LOGDOMAIN,
	      "Adding pin for \"%s\"",
	      token);
    AB_Pin_List_Add(p, ab->pinList);
  }

  for (i=0 ; ; i++) {
    const char *st;
    const char *t;
    int l;
    int doSet;

    if (i)
      flags|=AB_BANKING_INPUT_FLAGS_RETRY;

    if (i>AB_BANKING_MAX_PIN_TRY) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "No valid PIN within %d tries, giving up", i);
      AB_Banking_MessageBox(ab,
                            AB_BANKING_MSG_FLAGS_TYPE_ERROR |
                            AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                            I18N("Error"),
                            I18N("No valid PIN (tried too often).\n"
				 "Aborting."),
                            I18N("Dismiss"), 0, 0);
      return AB_ERROR_INVALID;
    }

    l=strlen(AB_Pin_GetValue(p));
    if (l>=minLen && l<=maxLen) {
      /* check whether PIN is bad */
      if (flags & AB_BANKING_INPUT_FLAGS_CONFIRM) {
        /* got a working pin */
        memmove(buffer, AB_Pin_GetValue(p), l+1);
        /* a confirmed pin is always ok */
        DBG_INFO(AQBANKING_LOGDOMAIN, "Confirmed PIN");
        AB_Pin_SetStatus(p, "ok");
        break;
      }
      AB_Banking__CheckBadPin(ab, p);
      st=AB_Pin_GetStatus(p);
      assert(st);
      DBG_INFO(AQBANKING_LOGDOMAIN, "Pin status: %s", st);
      if (strcasecmp(st, "bad")!=0) {
        /* got a working pin */
        memmove(buffer, AB_Pin_GetValue(p), l+1);
        break;
      }
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Pin is registered as \"bad\"");
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Pin is too short/long");
      AB_Pin_SetStatus(p, "bad");
    }
    rv=AB_Banking__GetPin(ab, flags, token, title, text, buffer,
                          minLen, maxLen);
    if (rv)
      return rv;

    doSet=0;
    t=AB_Pin_GetValue(p);
    if (t) {
      if (strcmp(buffer, t)!=0)
	doSet=1;
      else {
	int lrv;

	lrv=AB_Banking_MessageBox(ab,
				  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
				  AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
				  AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS,
				  I18N("Enforce PIN"),
				  I18N(
				  "You entered the same PIN twice.\n"
				  "The PIN is marked as bad, do you want\n"
				  "to use it anyway?"
				  "<html>"
                                  "<p>"
				  "You entered the same PIN twice."
				  "</p>"
                                  "<p>"
				  "The PIN is marked as <b>bad</b>, "
				  "do you want to use it anyway?"
				  "</p>"
				  "</html>"),
				  I18N("Use this"),
				  I18N("Re-enter"),
				  0);
	if (lrv==1) {
          /* accept this input */
          break;
	}
      }
    }
    else
      doSet=1;

    if (doSet) {
      AB_Pin_SetValue(p, buffer);
      AB_Pin_SetHash(p, 0);
      AB_Pin_SetStatus(p, "unknown");
    }
  } /* for */

  return 0;
}



int AB_Banking_SetPinStatus(AB_BANKING *ab,
			    const char *token,
                            const char *pin,
			    AB_BANKING_PINSTATUS status){
  AB_PIN *p;
  const char *s;

  assert(ab);
  assert(token);
  assert(pin);

  DBG_DEBUG(AQBANKING_LOGDOMAIN,
	    "Setting PIN status for \"%s\" to %d",
	    token, status);

  p=AB_Pin_List_First(ab->pinList);
  while(p) {
    const char *s;

    s=AB_Pin_GetToken(p);
    if (s) {
      if (strcasecmp(s, token)==0) {
        break;
      }
    }
    p=AB_Pin_List_Next(p);
  }

  if (!p) {
    /* no pin yet, create it */
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Pin \"%s\" is new", token);
    p=AB_Pin_new();
    AB_Pin_SetToken(p, token);
    AB_Pin_SetValue(p, pin);
    AB_Pin_SetHash(p, 0);
    AB_Pin_SetStatus(p, "unknown");
    AB_Pin_List_Add(p, ab->pinList);
  }

  /* we already know the pin, save the status */
  switch(status) {
  case AB_Banking_PinStatusBad: s="bad"; break;
  case AB_Banking_PinStatusOk:  s="ok"; break;
  default:                      s="unknown"; break;
  }
  AB_Pin_SetStatus(p, s);

  if (ab->setPinStatusFn) {
    return ab->setPinStatusFn(ab, token, pin, status);
  }
  else {
    return 0;
  }
}



void AB_Banking_SetPinCacheEnabled(AB_BANKING *ab, int enabled){
  assert(ab);
  ab->pinCacheEnabled = enabled;
}



int AB_Banking_GetPinCacheEnabled(const AB_BANKING *ab){
  assert(ab);
  return ab->pinCacheEnabled;
}



int AB_Banking_GetTan(AB_BANKING *ab,
                      const char *token,
                      const char *title,
                      const char *text,
                      char *buffer,
                      int minLen,
		      int maxLen){
  assert(ab);
  if (ab->getTanFn) {
    return ab->getTanFn(ab, token, title, text, buffer,
                        minLen, maxLen);
  }
  else {
    return AB_Banking_InputBox(ab,
                               AB_BANKING_INPUT_FLAGS_SHOW,
                               title,
                               text,
                               buffer,
                               minLen,
                               maxLen);
  }
}



int AB_Banking_SetTanStatus(AB_BANKING *ab,
                            const char *token,
                            const char *tan,
                            AB_BANKING_TANSTATUS status){
  DBG_NOTICE(AQBANKING_LOGDOMAIN,
             "Setting status of TAN to %d",
             status);
  assert(ab);
  if (ab->setTanStatusFn) {
    return ab->setTanStatusFn(ab, token, tan, status);
  }
  else {
    return 0;
  }
}



int AB_Banking__HashPin(AB_PIN *p) {
  const char *st;

  st=AB_Pin_GetStatus(p);
  if (st) {
    const char *token;
    const char *value;

    /* found a bad pin */
    token=AB_Pin_GetToken(p);
    value=AB_Pin_GetValue(p);
    if (token && value) {
      GWEN_BUFFER *buf;
      char hash[21];
      unsigned int bs;

      buf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Buffer_AppendString(buf, token);
      GWEN_Buffer_AppendByte(buf, '-');
      GWEN_Buffer_AppendString(buf, value);
      bs=sizeof(hash);
      if (GWEN_MD_Hash("RMD160",
                       GWEN_Buffer_GetStart(buf),
                       GWEN_Buffer_GetUsedBytes(buf),
                       hash, &bs)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Error on hash");
        GWEN_Buffer_free(buf);
        return AB_ERROR_GENERIC;
      }
      GWEN_Buffer_Reset(buf);
      if (GWEN_Text_ToHexBuffer(hash, bs, buf, 0, 0, 0)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Error encoding hash");
        GWEN_Buffer_free(buf);
        return AB_ERROR_GENERIC;
      }
      bs=GWEN_Buffer_GetUsedBytes(buf);
      if (*(GWEN_Buffer_GetStart(buf)+bs-1)=='/')
        /* cut of trailing slash */
        GWEN_Buffer_Crop(buf, 0, bs-1);
      AB_Pin_SetHash(p, GWEN_Buffer_GetStart(buf));
      GWEN_Buffer_free(buf);
    }
    else
      return AB_ERROR_GENERIC;
  }
  else
    return AB_ERROR_GENERIC;

  return 0;
}



int AB_Banking__SaveBadPins(AB_BANKING *ab) {
  AB_PIN *p;
  GWEN_DB_NODE *dbPins;

  dbPins=GWEN_DB_GetGroup(ab->data,
                          GWEN_DB_FLAGS_DEFAULT,
                          "static/pins");
  assert(dbPins);
  p=AB_Pin_List_First(ab->pinList);
  while(p) {
    const char *st;

    DBG_NOTICE(AQBANKING_LOGDOMAIN,
               "Checking pin \"%s\"",
               AB_Pin_GetToken(p));
    st=AB_Pin_GetStatus(p);
    if (st) {
      if (strcasecmp(st, "bad")==0) {
        const char *hash;

        /* only save bad pins */
        hash=AB_Pin_GetHash(p);
        if (!hash) {
          int rv;

          rv=AB_Banking__HashPin(p);
          if (rv) {
            return rv;
          }
          hash=AB_Pin_GetHash(p);
          assert(hash);
        } /* if no hash */
        GWEN_DB_SetCharValue(dbPins, GWEN_DB_FLAGS_OVERWRITE_VARS,
                             hash, st);
      } /* if pin is bad */
    } /* if status known */
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No status for pin \"%s\"",
                AB_Pin_GetToken(p));
    }
    p=AB_Pin_List_Next(p);
  } /* while */

  return 0;
}



int AB_Banking__CheckBadPin(AB_BANKING *ab, AB_PIN *p) {
  GWEN_DB_NODE *dbPins;
  const char *hash;
  const char *st;

  st=AB_Pin_GetStatus(p);
  if (st) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Pin status: %s", st);
    if (strcasecmp(st, "ok")==0) {
      /* pin is explicitly marked as "ok", assume it is */
      DBG_INFO(AQBANKING_LOGDOMAIN, "Pin is marked \"ok\"");
      return 0;
    }
  }
  dbPins=GWEN_DB_GetGroup(ab->data,
                          GWEN_DB_FLAGS_DEFAULT,
                          "static/pins");
  assert(dbPins);

  hash=AB_Pin_GetHash(p);
  if (!hash) {
    int rv;

    rv=AB_Banking__HashPin(p);
    if (rv) {
      return rv;
    }
    hash=AB_Pin_GetHash(p);
    assert(hash);
  } /* if no hash */
  if (!st) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No status, assuming unknown");
    st="unknown";
  }
  st=GWEN_DB_GetCharValue(dbPins, hash, 0, st);
  DBG_INFO(AQBANKING_LOGDOMAIN, "Saved pin status: %s", st);
  if (strcasecmp(st, "bad")==0) {
    AB_Pin_SetStatus(p, "bad");
    return AB_ERROR_BAD_DATA;
  }

  return 0;
}




GWEN_NETTRANSPORTSSL_ASKADDCERT_RESULT
AB_Banking_AskAddCert(GWEN_NETTRANSPORT *tr,
                      GWEN_DB_NODE *cert,
                      void *user_data){
  int rv;
  AB_BANKING *ab;
  GWEN_DB_NODE *pd;
  int isNew;
  int isError;
  int isWarning;
  const char *hash;
  const char *status;
  const char *ipAddr;
  const char *statusOn;
  const char *statusOff;
  char varName[128];
  char dbuffer1[32];
  char dbuffer2[32];
  char buffer[8192];
  GWEN_TYPE_UINT32 ti;
  char *msg=I18N_NOOP(
    "The following certificate has been received:\n"
    "Name        : %s\n"
    "Organisation: %s\n"
    "Department  : %s\n"
    "Country     : %s\n"
    "City        : %s\n"
    "State       : %s\n"
    "Valid after : %s\n"
    "Valid until : %s\n"
    "Hash        : %s\n"
    "Status      : %s\n"
    "Do you wish to accept this certificate?"

    "<html>"
    " <p>"
    "  The following certificate has been received:"
    " </p>"
    " <table>"
    "  <tr><td>Name</td><td>%s</td></tr>"
    "  <tr><td>Organisation</td><td>%s</td></tr>"
    "  <tr><td>Department</td><td>%s</td></tr>"
    "  <tr><td>Country</td><td>%s</td></tr>"
    "  <tr><td>City</td><td>%s</td></tr>"
    "  <tr><td>State</td><td>%s</td></tr>"
    "  <tr><td>Valid after</td><td>%s</td></tr>"
    "  <tr><td>Valid until</td><td>%s</td></tr>"
    "  <tr><td>Hash</td><td>%s</td></tr>"
    "  <tr><td>Status</td><td>%s%s%s</td></tr>"
    " </table>"
    " <p>"
    "  Do you wish to accept this certificate?"
    " </p>"
    "</html>"
    );

  assert(user_data);
  ab=(AB_BANKING*)user_data;

  pd=ab->data;
  assert(pd);
  pd=GWEN_DB_GetGroup(pd, GWEN_DB_FLAGS_DEFAULT, "static");
  assert(pd);

  memset(dbuffer1, 0, sizeof(dbuffer1));
  memset(dbuffer2, 0, sizeof(dbuffer2));
  memset(varName, 0, sizeof(varName));

  isNew=GWEN_DB_GetIntValue(cert, "isNew", 0, 1);
  isError=GWEN_DB_GetIntValue(cert, "isError", 0, 0);
  isWarning=GWEN_DB_GetIntValue(cert, "isWarning", 0, 0);
  hash=GWEN_DB_GetCharValue(cert, "HexFingerPrint", 0, 0);
  if (!hash) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "No fingerprint, falling back to hash");
    hash=GWEN_DB_GetCharValue(cert, "hash", 0, 0);
  }
  status=GWEN_DB_GetCharValue(cert, "statusText", 0, 0);
  ipAddr=GWEN_DB_GetCharValue(cert, "ipAddr", 0, 0);
  if (!ab->alwaysAskForCert && !isNew && hash && status && ipAddr) {
    GWEN_BUFFER *dbuf;
    const char *result;
    char msgHash[64];
    unsigned int bsize;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, "certificates/");
    GWEN_Buffer_AppendString(dbuf, ipAddr);
    GWEN_Buffer_AppendString(dbuf, "/");
    GWEN_Buffer_AppendString(dbuf, hash);
    GWEN_Buffer_AppendString(dbuf, "/");
    bsize=sizeof(msgHash);
    if (GWEN_MD_Hash("rmd160", status, strlen(status),
		     msgHash, &bsize)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Hash algo RMD160 not found");
      abort();
    }
    GWEN_Text_ToHexBuffer(msgHash, bsize, dbuf, 0, 0, 0);
    if (strlen(GWEN_Buffer_GetStart(dbuf))>=sizeof(varName)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Uuups, varname buffer is too small (%d needed)",
		strlen(GWEN_Buffer_GetStart(dbuf)));
      abort();
    }
    strncpy(varName, GWEN_Buffer_GetStart(dbuf),
	    sizeof(varName)-1);
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "Certificate path: %s", varName);
    result=GWEN_DB_GetCharValue(pd, GWEN_Buffer_GetStart(dbuf), 0,
                                0);
    if (!result)
      /* check temporary config */
      result=GWEN_DB_GetCharValue(ab->dbTempConfig,
				  GWEN_Buffer_GetStart(dbuf), 0,
				  0);
    if (result) {
      if (strcasecmp(result, "accept")==0) {
	DBG_NOTICE(AQBANKING_LOGDOMAIN,
		   "Automatically accepting certificate \"%s\"", hash);
	return GWEN_NetTransportSSL_AskAddCertResultPerm;
      }
      else if (strcasecmp(result, "temp")==0) {
	DBG_NOTICE(AQBANKING_LOGDOMAIN,
		   "Automatically accepting certificate \"%s\"", hash);
	return GWEN_NetTransportSSL_AskAddCertResultTmp;
      }
    }
    else
      isNew=1;
  }

  ti=(GWEN_TYPE_UINT32)GWEN_DB_GetIntValue(cert, "notBefore", 0, 0);
  if (ti) {
    GWEN_TIME *gt;
    GWEN_BUFFER *tbuf;

    gt=GWEN_Time_fromSeconds(ti);
    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    /* TRANSLATORS: This string is used as a template string to
       convert a given time into your local translated timeformat. The
       following characters are accepted in the template string: Y -
       digit of the year, M - digit of the month, D - digit of the day
       of month, h - digit of the hour, m - digit of the minute, s-
       digit of the second. All other characters are left unchanged. */
    if (GWEN_Time_toString(gt, I18N("YYYY/MM/DD hh:mm:ss"), tbuf)) {
      DBG_ERROR(0, "Could not convert beforeDate to string");
      abort();
    }
    strncpy(dbuffer1, GWEN_Buffer_GetStart(tbuf), sizeof(dbuffer1)-1);
    GWEN_Buffer_free(tbuf);
    GWEN_Time_free(gt);
  }

  ti=(GWEN_TYPE_UINT32)GWEN_DB_GetIntValue(cert, "notAfter", 0, 0);
  if (ti) {
    GWEN_TIME *gt;
    GWEN_BUFFER *tbuf;

    gt=GWEN_Time_fromSeconds(ti);
    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    if (GWEN_Time_toString(gt, I18N("YYYY/MM/DD hh:mm:ss"), tbuf)) {
      DBG_ERROR(0, "Could not convert untilDate to string");
      abort();
    }
    strncpy(dbuffer2, GWEN_Buffer_GetStart(tbuf), sizeof(dbuffer2)-1);
    GWEN_Buffer_free(tbuf);
    GWEN_Time_free(gt);
  }

  if (isError) {
    statusOn="<font color=red>";
    statusOff="</font>";
  }
  else if (isWarning) {
    statusOn="<font color=blue>";
    statusOff="</font>";
  }
  else {
    statusOn="<font color=green>";
    statusOff="</font>";
  }

  snprintf(buffer, sizeof(buffer)-1,
	   I18N(msg),
	   GWEN_DB_GetCharValue(cert, "commonName", 0, I18N("unknown")),
	   GWEN_DB_GetCharValue(cert, "organizationName", 0, I18N("unknown")),
	   GWEN_DB_GetCharValue(cert, "organizationalUnitName", 0, I18N("unknown")),
	   GWEN_DB_GetCharValue(cert, "countryName", 0, I18N("unknown")),
	   GWEN_DB_GetCharValue(cert, "localityName", 0, I18N("unknown")),
	   GWEN_DB_GetCharValue(cert, "stateOrProvinceName", 0, I18N("unknown")),
	   dbuffer1, dbuffer2,
	   hash,
	   GWEN_DB_GetCharValue(cert, "statusText", 0, I18N("unknown")),
	   /* the same again for HTML */
	   GWEN_DB_GetCharValue(cert, "commonName", 0, I18N("unknown")),
	   GWEN_DB_GetCharValue(cert, "organizationName", 0, I18N("unknown")),
	   GWEN_DB_GetCharValue(cert, "organizationalUnitName", 0, I18N("unknown")),
	   GWEN_DB_GetCharValue(cert, "countryName", 0, I18N("unknown")),
	   GWEN_DB_GetCharValue(cert, "localityName", 0, I18N("unknown")),
	   GWEN_DB_GetCharValue(cert, "stateOrProvinceName", 0, I18N("unknown")),
	   dbuffer1, dbuffer2,
           hash,
           statusOn,
           GWEN_DB_GetCharValue(cert, "statusText", 0, I18N("unknown")),
           statusOff
          );

  rv=AB_Banking_MessageBox(ab,
			   AB_BANKING_MSG_FLAGS_TYPE_WARN |
			   AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
			   AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS,
			   I18N("Certificate Received"),
			   buffer,
			   I18N("Yes"), I18N("No"), 0);
  if (rv==1) {
    rv=AB_Banking_MessageBox(ab,
			     AB_BANKING_MSG_FLAGS_TYPE_WARN |
			     AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
			     AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS,
			     I18N("Certificate"),
			     I18N(
    "Do you want to accept this certificate permanently?"
    "<html>Do you want to accept this certificate permanently?</html>"),
			     I18N("Permanently"),
			     I18N("This session only"),
			     I18N("Abort"));
    if (rv==1) {
      DBG_NOTICE(AQBANKING_LOGDOMAIN,
		 "User accepted certificate permanently");
      assert(varName);
      GWEN_DB_SetCharValue(pd, GWEN_DB_FLAGS_OVERWRITE_VARS,
			   varName, "accept");
      return GWEN_NetTransportSSL_AskAddCertResultPerm;
    }
    else if (rv==2) {
      DBG_NOTICE(AQBANKING_LOGDOMAIN,
		 "User accepted certificate temporarily");
      GWEN_DB_SetCharValue(ab->dbTempConfig,
			   GWEN_DB_FLAGS_OVERWRITE_VARS,
			   varName, "temp");
      return GWEN_NetTransportSSL_AskAddCertResultTmp;
    }
    else {
      DBG_NOTICE(AQBANKING_LOGDOMAIN,
		 "User aborted");
      return GWEN_NetTransportSSL_AskAddCertResultNo;
    }
  }
  else {
    DBG_NOTICE(AQBANKING_LOGDOMAIN,
	       "User rejected certificate");
    GWEN_DB_DeleteVar(pd, varName);
    GWEN_DB_DeleteVar(ab->dbTempConfig, varName);
    return GWEN_NetTransportSSL_AskAddCertResultNo;
  }
}



int AB_Banking_GetAlwaysAskForCert(const AB_BANKING *ab){
  assert(ab);
  return ab->alwaysAskForCert;
}



void AB_Banking_SetAlwaysAskForCert(AB_BANKING *ab, int i){
  assert(ab);
  ab->alwaysAskForCert=i;
}



const AB_COUNTRY *AB_Banking_FindCountryByName(AB_BANKING *ab,
                                               const char *name){
  assert(ab);
  return AB_Country_FindByName(name);
}



const AB_COUNTRY *AB_Banking_FindCountryByLocalName(AB_BANKING *ab,
                                               const char *name){
  assert(ab);
  return AB_Country_FindByLocalName(name);
}



const AB_COUNTRY *AB_Banking_FindCountryByCode(AB_BANKING *ab,
                                               const char *code){
  assert(ab);
  return AB_Country_FindByCode(code);
}



const AB_COUNTRY *AB_Banking_FindCountryByNumeric(AB_BANKING *ab,
                                                  int numid){
  assert(ab);
  return AB_Country_FindByNumeric(numid);
}



AB_COUNTRY_CONSTLIST2 *AB_Banking_ListCountriesByName(AB_BANKING *ab,
                                                      const char *name){
  assert(ab);
  return AB_Country_ListByName(name);
}



AB_COUNTRY_CONSTLIST2 *AB_Banking_ListCountriesByLocalName(AB_BANKING *ab,
                                                           const char *name){
  assert(ab);
  return AB_Country_ListByLocalName(name);
}



int AB_Banking__TransformIban(const char *iban, int len,
			      char *newIban, int maxLen) {
  int i, j;
  const char *p;
  char *s;

  assert(iban);
  /* format IBAN */
  i=0;
  j=0;
  p=iban;
  s=newIban;
  while(j<len && i<maxLen) {
    int c;

    c=toupper(*p);
    if (c!=' ') {
      if (c>='A' && c<='Z') {
	c=10+(c-'A');
	*s='0'+(c/10);
	s++; i++;
	if (i>=maxLen) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (too long)");
	  return -1;
	}
	*s='0'+(c%10);
	s++; i++;
      }
      else if (isdigit(c)) {
	*s=c;
	s++; i++;
      }
      else {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (bad char)");
	return -1;
      }
    }
    p++;
    j++;
  } /* while */
  if (j<len) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (too long)");
    return -1;
  }
  *s=0;

  return 0;
}



int AB_Banking_CheckIban(const char *iban) {
  char newIban[256];
  char tmp[10];
  int i;
  unsigned int j;
  const char *p;
  char *s;

  if (strlen(iban)<5) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (too short)");
    return -1;
  }
  p=iban+4;

  /* convert IBAN+4 to buffer */
  if (AB_Banking__TransformIban(p, strlen(p),
				newIban, sizeof(newIban)-1)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return -1;
  }

  /* append country and checksum */
  p=iban;
  s=newIban+strlen(newIban);
  if (AB_Banking__TransformIban(p, 4, s, sizeof(newIban)-strlen(newIban)-1)){
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return -1;
  }

  /* calculate checksum in 9er steps */
  p=newIban;
  tmp[0]=0;
  j=0;
  while(*p) {
    i=strlen(tmp);
    for (i=strlen(tmp); i<9;  i++) {
      if (!*p)
        break;
      tmp[i]=*(p++);
    }
    tmp[i]=0;
    if (1!=sscanf(tmp, "%u", &j)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (bad char)");
      return -1;
    }
    j=j%97; /* modulo 97 */
    snprintf(tmp, sizeof(tmp), "%u", j);
  } /* while */

  if (j!=1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad IBAN (bad checksum)");
    return 1;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN, "IBAN is valid");
  return 0;
}













