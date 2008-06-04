/***************************************************************************
 $RCSfile: banking.c,v $
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* don't warn about our own deprecated functions */
#define AQBANKING_NOWARN_DEPRECATED 


#include "banking_p.h"
#include "provider_l.h"
#include "imexporter_l.h"
#include "bankinfoplugin_l.h"
#include "i18n_l.h"
#include "country_l.h"
#include "userfns_l.h"

#include <gwenhywfar/version.h>
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/bio_file.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/ctplugin.h>

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

GWEN_INHERIT_FUNCTIONS(AB_BANKING)

#include <aqbanking/error.h>


#include "banking_init.c"
#include "banking_account.c"
#include "banking_user.c"
#include "banking_online.c"


void AB_Banking__GetConfigFileNameAndDataDir(AB_BANKING *ab,
					     const char *dname) {
  GWEN_BUFFER *buf;
  char home[256];

  if (GWEN_Directory_GetHomeDirectory(home, sizeof(home))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not determine home directory, aborting.");
    abort();
  }

  buf=GWEN_Buffer_new(0, 256, 0, 1);

  if (dname) {
    /* determine config file name */
    GWEN_Buffer_AppendString(buf, dname);
    GWEN_Buffer_AppendString(buf, DIRSEP AB_BANKING_CONFIGFILE);
    ab->configFile=strdup(GWEN_Buffer_GetStart(buf));
    /* setup data dir */
    ab->dataDir=strdup(dname);
  }
  else {
    uint32_t pos;
    FILE *f;
    const char *s;

    /* determine config directory */
    s=getenv("AQBANKING_HOME");
    if (s && !*s)
      s=0;
    if (s)
      GWEN_Buffer_AppendString(buf, s);
    else
      GWEN_Buffer_AppendString(buf, home);
    GWEN_Buffer_AppendString(buf, DIRSEP);
    pos=GWEN_Buffer_GetPos(buf);
    GWEN_Buffer_AppendString(buf, AB_BANKING_USERDATADIR);
    /* as we are at it: store default data dir */
    ab->dataDir=strdup(GWEN_Buffer_GetStart(buf));

    /* first try new default file */
    GWEN_Buffer_AppendString(buf, DIRSEP AB_BANKING_CONFIGFILE);
    f=fopen(GWEN_Buffer_GetStart(buf), "r");
    if (f) {
      fclose(f);
      ab->configFile=strdup(GWEN_Buffer_GetStart(buf));
    }
    else {
      /* try old default file */
      GWEN_Buffer_Crop(buf, 0, pos);
      GWEN_Buffer_AppendString(buf, AB_BANKING_OLD_CONFIGFILE);
      f=fopen(GWEN_Buffer_GetStart(buf), "r");
      if (f) {
	fclose(f);
	ab->configFile=strdup(GWEN_Buffer_GetStart(buf));
	/* New file did not exist, if the old file exists we will move it
	 * upon AB_Banking_Fini(). */
      }
      else {
	/* file not found, create new default file later */
	GWEN_Buffer_Crop(buf, 0, pos);
	GWEN_Buffer_AppendString(buf,
				 AB_BANKING_USERDATADIR DIRSEP
				 AB_BANKING_CONFIGFILE);
	ab->configFile=strdup(GWEN_Buffer_GetStart(buf));
      }
    }
  }
  GWEN_Buffer_free(buf);
}




AB_BANKING *AB_Banking_new(const char *appName,
			   const char *dname,
			   uint32_t extensions){
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
    while(*s) {
      *s=tolower(*s);
      s++;
    }
  }

  GWEN_NEW_OBJECT(AB_BANKING, ab);
  GWEN_INHERIT_INIT(AB_BANKING, ab);
  ab->providers=AB_Provider_List_new();
  ab->users=AB_User_List_new();
  ab->accounts=AB_Account_List_new();
  ab->appEscName=strdup(GWEN_Buffer_GetStart(nbuf));
  ab->appName=strdup(appName);
  ab->activeProviders=GWEN_StringList_new();
  ab->cryptTokenList=GWEN_Crypt_Token_List2_new();

  GWEN_StringList_SetSenseCase(ab->activeProviders, 0);
  ab->data=GWEN_DB_Group_new("BankingData");
  GWEN_Buffer_free(nbuf);

  AB_Banking__GetConfigFileNameAndDataDir(ab, dname);

  ab->dbTempConfig=GWEN_DB_Group_new("tmpConfig");

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



void AB_Banking_free(AB_BANKING *ab){
  if (ab) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Destroying AB_BANKING");

    GWEN_INHERIT_FINI(AB_BANKING, ab);

    AB_Banking_ClearCryptTokenList(ab, 0);
    GWEN_Crypt_Token_List2_free(ab->cryptTokenList);
    AB_Account_List_free(ab->accounts);
    AB_User_List_free(ab->users);
    AB_Provider_List_free(ab->providers);
    GWEN_StringList_free(ab->activeProviders);
    GWEN_DB_Group_free(ab->data);
    GWEN_DB_Group_free(ab->dbTempConfig);
    free(ab->startFolder);
    free(ab->appName);
    free(ab->appEscName);
    free(ab->configFile);
    free(ab->dataDir);
    GWEN_FREE_OBJECT(ab);
    GWEN_Fini();
  }
}



uint32_t AB_Banking_GetUniqueId(AB_BANKING *ab){
  GWEN_BUFFER *nbuf;
  uint32_t uniqueId;
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
    int err;
    unsigned long int i;

    buffer[0]=0;
    bio=GWEN_BufferedIO_File_new(fd);
    GWEN_BufferedIO_SubFlags(bio, GWEN_BUFFEREDIO_FLAGS_CLOSE);
    GWEN_BufferedIO_SetReadBuffer(bio, 0, 256);
    if (!GWEN_BufferedIO_CheckEOF(bio)) {
      err=GWEN_BufferedIO_ReadLine(bio, buffer, sizeof(buffer)-1);
      if (err) {
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
    if (err) {
      DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
      GWEN_BufferedIO_free(bio);
      AB_Banking__CloseFile(fd);
      GWEN_Buffer_free(nbuf);
      return 0;
    }
    err=GWEN_BufferedIO_Flush(bio);
    if (err) {
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



int AB_Banking___LoadData(AB_BANKING *ab,
                          const char *prefix,
                          const char *name) {
  GWEN_BUFFER *pbuf;
  GWEN_DB_NODE *db;

  assert(ab);
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (AB_Banking_GetUserDataDir(ab, pbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not get user data dir");
    GWEN_Buffer_free(pbuf);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_AppendString(pbuf, DIRSEP);
  GWEN_Buffer_AppendString(pbuf, prefix);
  GWEN_Buffer_AppendString(pbuf, DIRSEP);
  GWEN_Buffer_AppendString(pbuf, name);
  GWEN_Buffer_AppendString(pbuf, DIRSEP "settings.conf");

  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "external");
  assert(db);
  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, prefix);
  assert(db);
  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, name);
  assert(db);
  DBG_INFO(AQBANKING_LOGDOMAIN,
             "Reading file \"%s\"", GWEN_Buffer_GetStart(pbuf));
  if (GWEN_DB_ReadFile(db, GWEN_Buffer_GetStart(pbuf),
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_CREATE_GROUP |
		       GWEN_DB_FLAGS_LOCKFILE, 0, 2000)) {
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



int AB_Banking___SaveData(AB_BANKING *ab,
                          const char *prefix,
                          const char *name) {
  GWEN_BUFFER *pbuf;
  GWEN_BUFFER *rpbuf;
  GWEN_DB_NODE *db;

  assert(ab);
  db=GWEN_DB_GetGroup(ab->data, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      "external");
  if (!db)
    return 0;
  db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, prefix);
  if (!db)
    return 0;
  db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, name);
  if (!db)
    return 0;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (AB_Banking_GetUserDataDir(ab, pbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not get user data dir");
    GWEN_Buffer_free(pbuf);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_AppendString(pbuf, DIRSEP);
  GWEN_Buffer_AppendString(pbuf, prefix);
  GWEN_Buffer_AppendString(pbuf, DIRSEP);
  GWEN_Buffer_AppendString(pbuf, name);
  GWEN_Buffer_AppendString(pbuf, DIRSEP "settings.conf");


  DBG_INFO(AQBANKING_LOGDOMAIN,
             "Writing file \"%s\"", GWEN_Buffer_GetStart(pbuf));
  if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(pbuf),
			     GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Path \"%s\" is not available",
              GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);
    return GWEN_ERROR_GENERIC;
  }
  rpbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(pbuf)+4, 0, 1);
  GWEN_Buffer_AppendBuffer(rpbuf, pbuf);
  GWEN_Buffer_AppendString(pbuf, ".tmp");
  if (GWEN_DB_WriteFile(db, GWEN_Buffer_GetStart(pbuf),
                        GWEN_DB_FLAGS_DEFAULT |
			GWEN_DB_FLAGS_LOCKFILE, 0, 2000)) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Could not save app config file \"%s\"",
	     GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(rpbuf);
    GWEN_Buffer_free(pbuf);
    return GWEN_ERROR_GENERIC;
  }
#ifdef OS_WIN32
  if (unlink(GWEN_Buffer_GetStart(rpbuf))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not delete old file \"%s\": %s",
              GWEN_Buffer_GetStart(rpbuf),
              strerror(errno));
    GWEN_Buffer_free(rpbuf);
    GWEN_Buffer_free(pbuf);
    return GWEN_ERROR_GENERIC;
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
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(rpbuf);
  GWEN_Buffer_free(pbuf);

  /* sucessfully written */
  return 0;
}



int AB_Banking__LoadData(AB_BANKING *ab,
                         const char *prefix,
                         const char *name) {
  GWEN_BUFFER *pbuf;
  int rv;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (GWEN_Text_EscapeToBuffer(name, pbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error escaping string, aborting (%s).", name);
    GWEN_Buffer_free(pbuf);
    abort();
  }

  rv=AB_Banking___LoadData(ab, prefix, GWEN_Buffer_GetStart(pbuf));
  GWEN_Buffer_free(pbuf);
  return rv;
}



int AB_Banking__SaveData(AB_BANKING *ab,
                         const char *prefix,
                         const char *name) {
  GWEN_BUFFER *pbuf;
  int rv;

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (GWEN_Text_EscapeToBuffer(name, pbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error escaping string, aborting (%s).", name);
    GWEN_Buffer_free(pbuf);
    abort();
  }

  rv=AB_Banking___SaveData(ab, prefix, GWEN_Buffer_GetStart(pbuf));
  GWEN_Buffer_free(pbuf);
  return rv;
}



int AB_Banking__SaveExternalData(AB_BANKING *ab) {
  GWEN_DB_NODE *db;

  db=GWEN_DB_GetGroup(ab->data, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "external");
  if (!db)
    return 0;
  db=GWEN_DB_GetFirstGroup(db);
  while(db) {
    const char *prefix;
    GWEN_DB_NODE *dbName;

    prefix=GWEN_DB_GroupName(db);
    dbName=GWEN_DB_GetFirstGroup(db);
    while(dbName) {
      int rv;

      rv=AB_Banking__SaveData(ab, prefix, GWEN_DB_GroupName(dbName));
      if (rv)
        return rv;
      dbName=GWEN_DB_GetNextGroup(dbName);
    }
    db=GWEN_DB_GetNextGroup(db);
  }

  return 0;
}



int AB_Banking__LoadAppData(AB_BANKING *ab) {
  return AB_Banking__LoadData(ab, "apps", ab->appEscName);
}



int AB_Banking__LoadSharedData(AB_BANKING *ab, const char *name) {
  return AB_Banking__LoadData(ab, "shared", name);
}



int AB_Banking__SaveSharedData(AB_BANKING *ab, const char *name) {
  return AB_Banking__SaveData(ab, "shared", name);
}



GWEN_DB_NODE *AB_Banking_GetAppData(AB_BANKING *ab) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbT;

  assert(ab);
  assert(ab->appEscName);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "external/apps");
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



GWEN_DB_NODE *AB_Banking_GetSharedData(AB_BANKING *ab, const char *name) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbT;

  assert(ab);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "external/shared");
  assert(db);

  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, name);
  if (!dbT) {
    if (AB_Banking__LoadSharedData(ab, name)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load app data file");
      return 0;
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, name);
  assert(dbT);
  return dbT;
}



int AB_Banking_Save(AB_BANKING *ab) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbT;
  AB_ACCOUNT *a;
  AB_USER *u;
  int rvExternal;
  GWEN_BUFFER *rpbuf;

  assert(ab);

  if (ab->initCount==0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Online banking not initialised");
    return GWEN_ERROR_INVALID;
  }

  db=GWEN_DB_Group_new("config");
  assert(db);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "lastVersion",
                      (AQBANKING_VERSION_MAJOR<<24) |
                      (AQBANKING_VERSION_MINOR<<16) |
                      (AQBANKING_VERSION_PATCHLEVEL<<8) |
                      AQBANKING_VERSION_BUILD);

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

  /* save users */
  u=AB_User_List_First(ab->users);
  while(u) {
    GWEN_DB_NODE *dbTdst;
    int rv;
    AB_PROVIDER *pro;

    /* let provider store not yet stored data */
    pro=AB_User_GetProvider(u);
    if (pro) {
      rv=AB_Provider_ExtendUser(pro, u, AB_ProviderExtendMode_Save);
      if (rv) {
        DBG_WARN(AQBANKING_LOGDOMAIN, "Error extending user (%d)", rv);
      }
    }
    else {
      DBG_WARN(AQBANKING_LOGDOMAIN, "No provider for user \"%08x\"",
               AB_User_GetUniqueId(u));
    }

    /* store user data */
    dbTdst=GWEN_DB_GetGroup(db,
                            GWEN_DB_FLAGS_DEFAULT |
                            GWEN_PATH_FLAGS_CREATE_GROUP,
                            "users/user");
    assert(dbTdst);
    rv=AB_User_toDb(u, dbTdst);
    if (rv) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error saving user \"%08x\"",
                AB_User_GetUniqueId(u));
      GWEN_DB_Group_free(db);
      return rv;
    }
    u=AB_User_List_Next(u);
  } /* while */

  /* save accounts */
  a=AB_Account_List_First(ab->accounts);
  while(a) {
    GWEN_DB_NODE *dbTdst;
    int rv;
    AB_PROVIDER *pro;

    /* let provider store not yet stored data */
    pro=AB_Account_GetProvider(a);
    if (pro) {
      rv=AB_Provider_ExtendAccount(pro, a, AB_ProviderExtendMode_Save);
      if (rv) {
        DBG_WARN(AQBANKING_LOGDOMAIN, "Error extending account (%d)", rv);
      }
    }

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

  /* store bad pins */
  dbT=GWEN_DB_GetGroup(ab->data,
                       GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "banking/pins");
  if (dbT) {
    GWEN_DB_NODE *dbTdst;

    dbTdst=GWEN_DB_GetGroup(db,
                            GWEN_DB_FLAGS_DEFAULT,
                            "pins");
    GWEN_DB_AddGroupChildren(dbTdst, dbT);
  }

  /* store certificates */
  dbT=GWEN_DB_GetGroup(ab->data,
                       GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "banking/certificates");
  if (dbT) {
    GWEN_DB_NODE *dbTdst;

    dbTdst=GWEN_DB_GetGroup(db,
                            GWEN_DB_FLAGS_DEFAULT,
                            "certificates");
    GWEN_DB_AddGroupChildren(dbTdst, dbT);
  }

  /* store backends */
  dbT=GWEN_DB_GetGroup(ab->data,
                       GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "banking/backends");
  if (dbT) {
    GWEN_DB_NODE *dbTdst;

    dbTdst=GWEN_DB_GetGroup(db,
                            GWEN_DB_FLAGS_DEFAULT,
                            "backends");
    GWEN_DB_AddGroupChildren(dbTdst, dbT);
  }

  /* store external data */
  rvExternal=AB_Banking__SaveExternalData(ab);

  /* write config file. TODO: make backups */
  rpbuf=GWEN_Buffer_new(0, strlen(ab->configFile)+4, 0, 1);
  GWEN_Buffer_AppendString(rpbuf, ab->configFile);
  GWEN_Buffer_AppendString(rpbuf, ".tmp");
  if (GWEN_DB_WriteFile(db, GWEN_Buffer_GetStart(rpbuf),
			GWEN_DB_FLAGS_DEFAULT|GWEN_DB_FLAGS_LOCKFILE, 0, 2000)) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "Could not save app config file \"%s\"",
             ab->configFile);
    GWEN_Buffer_free(rpbuf);
    GWEN_DB_Group_free(db);
    return GWEN_ERROR_GENERIC;
  }
#ifdef OS_WIN32
  if (access(ab->configFile, F_OK) == 0) {
    /* Need to unlink the old file only if it already existed */
    if (unlink(ab->configFile)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Could not delete old file \"%s\": %s",
		ab->configFile,
		strerror(errno));
      GWEN_Buffer_free(rpbuf);
      GWEN_DB_Group_free(db);
      return GWEN_ERROR_GENERIC;
    }
  }
#endif
  if (rename(GWEN_Buffer_GetStart(rpbuf), ab->configFile)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not rename file to \"%s\": %s",
              ab->configFile,
              strerror(errno));
    GWEN_Buffer_free(rpbuf);
    GWEN_DB_Group_free(db);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(rpbuf);
  GWEN_DB_Group_free(db);

  if (rvExternal) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not save external configuration");
    return rvExternal;
  }

  return 0;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetImExporterDescrs(AB_BANKING *ab){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *l;

  l=GWEN_LoadPluginDescrs(AQBANKING_PLUGINS DIRSEP AB_IMEXPORTER_FOLDER);
  return l;
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
    GWEN_Buffer_AppendString(buf, DIRSEP AB_BANKING_USERDATADIR);
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
    GWEN_Buffer_AppendString(buf, DIRSEP AB_BANKING_USERDATADIR);
  }

  GWEN_Buffer_AppendString(buf, DIRSEP "shared" DIRSEP);
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
  GWEN_DIRECTORY *d;
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
    return GWEN_ERROR_NOT_FOUND;
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
				   GWEN_PATH_FLAGS_CREATE_GROUP, 0, 2000)) {
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
  GWEN_STRINGLIST *sl;
  GWEN_STRINGLISTENTRY *sentry;

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  db=GWEN_DB_Group_new("profiles");

  sl=AB_Banking_GetGlobalDataDirs(ab);
  assert(sl);

  sentry=GWEN_StringList_FirstEntry(sl);
  assert(sentry);

  while(sentry) {
    const char *pkgdatadir;

    pkgdatadir = GWEN_StringListEntry_Data(sentry);
    assert(pkgdatadir);

    /* read global profiles */
    GWEN_Buffer_AppendString(buf, pkgdatadir);
    GWEN_Buffer_AppendString(buf,
			     DIRSEP
			     "aqbanking"
			     DIRSEP
			     AB_IMEXPORTER_FOLDER
			     DIRSEP);
    if (GWEN_Text_EscapeToBufferTolerant(name, buf)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Bad name for importer/exporter");
      GWEN_StringList_free(sl);
      GWEN_DB_Group_free(db);
      GWEN_Buffer_free(buf);
      return 0;
    }
    GWEN_Buffer_AppendString(buf, DIRSEP "profiles");
    rv=AB_Banking__ReadImExporterProfiles(ab,
                                          GWEN_Buffer_GetStart(buf),
                                          db);
    if (rv && rv!=GWEN_ERROR_NOT_FOUND) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Error reading global profiles");
      GWEN_StringList_free(sl);
      GWEN_DB_Group_free(db);
      GWEN_Buffer_free(buf);
      return 0;
    }
    GWEN_Buffer_Reset(buf);
    sentry=GWEN_StringListEntry_Next(sentry);
  }
  GWEN_StringList_free(sl);

  /* read local user profiles */
  if (AB_Banking_GetUserDataDir(ab, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not get user data dir");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_AppendString(buf, DIRSEP AB_IMEXPORTER_FOLDER DIRSEP);
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
  if (rv && rv!=GWEN_ERROR_NOT_FOUND) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error reading users profiles");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_free(buf);

  return db;
}







AB_BANKINFO_PLUGIN *AB_Banking__LoadBankInfoPlugin(AB_BANKING *ab,
						   const char *modname){
  GWEN_PLUGIN *pl;

  pl=GWEN_PluginManager_GetPlugin(ab_pluginManagerBankInfo, modname);
  if (pl) {
    AB_BANKINFO_PLUGIN *bip;
    GWEN_DB_NODE *db;

    db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
			"banking/bankinfoplugins");
    assert(db);
    db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT, modname);
    assert(db);

    bip=AB_Plugin_BankInfo_Factory(pl, ab, db);
    if (!bip) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Error in plugin [%s]: No bank info created",
		modname);
      return NULL;
    }
    return bip;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Plugin [%s] not found", modname);
    return NULL;
  }
}



AB_BANKINFO_PLUGIN *AB_Banking__GetBankInfoPlugin(AB_BANKING *ab,
                                                  const char *country) {
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);

  bip=AB_BankInfoPlugin_List_First(ab_bankInfoPlugins);
  while(bip) {
    if (strcasecmp(AB_BankInfoPlugin_GetCountry(bip), country)==0)
      return bip;
    bip=AB_BankInfoPlugin_List_Next(bip);
  }

  bip=AB_Banking__LoadBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "BankInfo plugin for country \"%s\" not found",
              country);
    return 0;
  }
  AB_BankInfoPlugin_List_Add(bip, ab_bankInfoPlugins);
  return bip;
}



AB_BANKINFO *AB_Banking_GetBankInfo(AB_BANKING *ab,
                                    const char *country,
                                    const char *branchId,
                                    const char *bankId){
  AB_BANKINFO_PLUGIN *bip;

  assert(ab);
  assert(country);
  bip=AB_Banking__GetBankInfoPlugin(ab, country);
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
  bip=AB_Banking__GetBankInfoPlugin(ab, country);
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
  bip=AB_Banking__GetBankInfoPlugin(ab, country);
  if (!bip) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "BankInfo plugin for country \"%s\" not found",
             country);
    return AB_BankInfoCheckResult_UnknownResult;
  }

  return AB_BankInfoPlugin_CheckAccount(bip, branchId, bankId, accountId);
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
  ie=AB_ImExporter_List_First(ab_imexporters);
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
  ie=AB_Banking__LoadImExporterPlugin(ab, name);
  if (ie) {
    AB_ImExporter_List_Add(ie, ab_imexporters);
  }

  return ie;
}



AB_IMEXPORTER *AB_Banking__LoadImExporterPlugin(AB_BANKING *ab,
                                                const char *modname){
  GWEN_PLUGIN *pl;

  pl=GWEN_PluginManager_GetPlugin(ab_pluginManagerImExporter, modname);
  if (pl) {
    AB_IMEXPORTER *ie;
    GWEN_DB_NODE *db;

    db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
			"banking/imexporters");
    assert(db);
    db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT, modname);
    assert(db);

    ie=AB_Plugin_ImExporter_Factory(pl, ab, db);
    if (!ie) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Error in plugin [%s]: No im/exporter created",
		modname);
      return NULL;
    }
    return ie;
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Plugin [%s] not found", modname);
    return NULL;
  }
}






/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             High Level API
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */



AB_ACCOUNT *AB_Banking__GetAccount(AB_BANKING *ab, const char *accountId){
  GWEN_DB_NODE *dbData;
  uint32_t uniqueId;
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



GWEN_STRINGLIST *AB_Banking_GetGlobalDataDirs() {
  GWEN_STRINGLIST *sl;

  sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME, AB_PM_DATADIR);
  return sl;
}



GWEN_STRINGLIST *AB_Banking_GetGlobalSysconfDirs() {
  GWEN_STRINGLIST *sl;

  sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME, AB_PM_SYSCONFDIR);
  return sl;
}



void AB_Banking_GetVersion(int *major,
			   int *minor,
			   int *patchlevel,
			   int *build) {
  if (major)
    *major=AQBANKING_VERSION_MAJOR;
  if (minor)
    *minor=AQBANKING_VERSION_MINOR;
  if (patchlevel)
    *patchlevel=AQBANKING_VERSION_PATCHLEVEL;
  if (build)
    *build=AQBANKING_VERSION_BUILD;
}




int AB_Banking_ExportWithProfile(AB_BANKING *ab,
				 const char *exporterName,
				 AB_IMEXPORTER_CONTEXT *ctx,
				 const char *profileName,
				 const char *profileFile,
				 GWEN_IO_LAYER *io,
				 uint32_t guiid) {
  AB_IMEXPORTER *exporter;
  GWEN_DB_NODE *dbProfiles;
  GWEN_DB_NODE *dbProfile;
  int rv;

  exporter=AB_Banking_GetImExporter(ab, exporterName);
  if (!exporter) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Export module \"%s\" not found",
	      exporterName);
    return GWEN_ERROR_NOT_FOUND;
  }

  /* get profiles */
  if (profileFile) {
    dbProfiles=GWEN_DB_Group_new("profiles");
    if (GWEN_DB_ReadFile(dbProfiles, profileFile,
                         GWEN_DB_FLAGS_DEFAULT |
			 GWEN_PATH_FLAGS_CREATE_GROUP,
			 0,
			 2000)) {
      DBG_ERROR(0, "Error reading profiles from \"%s\"",
                profileFile);
      return GWEN_ERROR_GENERIC;
    }
  }
  else {
    dbProfiles=AB_Banking_GetImExporterProfiles(ab, exporterName);
  }

  /* select profile */
  dbProfile=GWEN_DB_GetFirstGroup(dbProfiles);
  while(dbProfile) {
    const char *name;

    name=GWEN_DB_GetCharValue(dbProfile, "name", 0, 0);
    assert(name);
    if (strcasecmp(name, profileName)==0)
      break;
    dbProfile=GWEN_DB_GetNextGroup(dbProfile);
  }
  if (!dbProfile) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Profile \"%s\" for exporter \"%s\" not found",
	      profileName, exporterName);
    GWEN_DB_Group_free(dbProfiles);
    return GWEN_ERROR_NOT_FOUND;
  }

  rv=AB_ImExporter_Export(exporter,
			  ctx,
			  io,
			  dbProfile,
			  guiid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbProfiles);
    return rv;
  }

  GWEN_DB_Group_free(dbProfiles);
  return 0;
}



int AB_Banking_ImportWithProfile(AB_BANKING *ab,
				 const char *importerName,
				 AB_IMEXPORTER_CONTEXT *ctx,
				 const char *profileName,
				 const char *profileFile,
				 GWEN_IO_LAYER *io,
				 uint32_t guiid) {
  AB_IMEXPORTER *importer;
  GWEN_DB_NODE *dbProfiles;
  GWEN_DB_NODE *dbProfile;
  int rv;

  importer=AB_Banking_GetImExporter(ab, importerName);
  if (!importer) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Import module \"%s\" not found",
	      importerName);
    return GWEN_ERROR_NOT_FOUND;
  }

  /* get profiles */
  if (profileFile) {
    dbProfiles=GWEN_DB_Group_new("profiles");
    if (GWEN_DB_ReadFile(dbProfiles, profileFile,
                         GWEN_DB_FLAGS_DEFAULT |
			 GWEN_PATH_FLAGS_CREATE_GROUP,
			 0,
			 2000)) {
      DBG_ERROR(0, "Error reading profiles from \"%s\"",
                profileFile);
      return GWEN_ERROR_GENERIC;
    }
  }
  else {
    dbProfiles=AB_Banking_GetImExporterProfiles(ab, importerName);
  }

  /* select profile */
  dbProfile=GWEN_DB_GetFirstGroup(dbProfiles);
  while(dbProfile) {
    const char *name;

    name=GWEN_DB_GetCharValue(dbProfile, "name", 0, 0);
    assert(name);
    if (strcasecmp(name, profileName)==0)
      break;
    dbProfile=GWEN_DB_GetNextGroup(dbProfile);
  }
  if (!dbProfile) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Profile \"%s\" for importer \"%s\" not found",
	      profileName, importerName);
    GWEN_DB_Group_free(dbProfiles);
    return GWEN_ERROR_NOT_FOUND;
  }

  rv=AB_ImExporter_Import(importer,
			  ctx,
			  io,
			  dbProfile,
			  guiid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbProfiles);
    return rv;
  }

  GWEN_DB_Group_free(dbProfiles);
  return 0;
}





