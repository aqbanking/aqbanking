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
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/bio_file.h>
#include <gwenhywfar/text.h>

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



GWEN_INHERIT_FUNCTIONS(AB_BANKING)

#include <aqbanking/error.h>


AB_BANKING *AB_Banking_new(const char *appName, const char *fname){
  AB_BANKING *ab;
  GWEN_BUFFER *buf;
  GWEN_BUFFER *nbuf;

  assert(appName);

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
    GWEN_Buffer_AppendString(buf, "/" AB_BANKING_CONFIGFILE);
    fname=GWEN_Buffer_GetStart(buf);
  }

  GWEN_NEW_OBJECT(AB_BANKING, ab);
  GWEN_INHERIT_INIT(AB_BANKING, ab);
  ab->providers=AB_Provider_List_new();
  ab->imexporters=AB_ImExporter_List_new();
  ab->accounts=AB_Account_List_new();
  ab->enqueuedJobs=AB_Job_List_new();
  ab->appEscName=strdup(GWEN_Buffer_GetStart(nbuf));
  ab->appName=strdup(appName);
  ab->activeProviders=GWEN_StringList_new();
  GWEN_StringList_SetSenseCase(ab->activeProviders, 0);
  ab->data=GWEN_DB_Group_new("BankingData");
  ab->configFile=strdup(fname);
  GWEN_Buffer_free(buf);
  GWEN_Buffer_free(nbuf);
  return ab;
}



void AB_Banking_free(AB_BANKING *ab){
  if (ab) {
    DBG_NOTICE(AQBANKING_LOGDOMAIN, "Freeing AB_BANKING");
    GWEN_INHERIT_FINI(AB_BANKING, ab);
    AB_Job_List_free(ab->enqueuedJobs);
    AB_Account_List_free(ab->accounts);
    AB_Provider_List_free(ab->providers);
    AB_ImExporter_List_free(ab->imexporters);
    GWEN_StringList_free(ab->activeProviders);
    GWEN_DB_Group_free(ab->data);
    free(ab->appName);
    free(ab->appEscName);
    free(ab->configFile);
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
  assert(ab);
  return ++(ab->lastUniqueId);
}



const char *AB_Banking_GetAppName(const AB_BANKING *ab){
  assert(ab);
  return ab->appName;
}



const char *AB_Banking_GetEscapedAppName(const AB_BANKING *ab){
  assert(ab);
  return ab->appEscName;
}



GWEN_DB_NODE *AB_Banking_GetProviderData(AB_BANKING *ab,
                                         const AB_PROVIDER *pro){
  const char *name;
  GWEN_DB_NODE *db;

  name=AB_Provider_GetEscapedName(pro);
  assert(name);

  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT,
                      "static/providers");
  assert(db);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT, name);
  assert(db);
  return db;
}



int AB_Banking__GetAppConfigFileName(AB_BANKING *ab, GWEN_BUFFER *buf) {
  if (AB_Banking_GetUserDataDir(ab, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not get user data dir");
    return AB_ERROR_GENERIC;
  }
  GWEN_Buffer_AppendString(buf, "/appcfg/");
  GWEN_Buffer_AppendString(buf, ab->appEscName);
  GWEN_Buffer_AppendString(buf, ".conf");
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
  DBG_NOTICE(0, "Reading file \"%s\"", GWEN_Buffer_GetStart(pbuf));
  if (GWEN_DB_ReadFile(db, GWEN_Buffer_GetStart(pbuf),
		       GWEN_DB_FLAGS_DEFAULT |
		       GWEN_PATH_FLAGS_CREATE_GROUP)) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Could not load config file \"%s\", creating it later",
	     GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);
    return 0;
  }

  /* sucessfully read */
  return 0;
}



int AB_Banking__SaveAllAppData(AB_BANKING *ab) {
  GWEN_BUFFER *pbuf;
  GWEN_DB_NODE *db;
  GWEN_TYPE_UINT32 pos;
  int errors;

  assert(ab);

  db=GWEN_DB_GetGroup(ab->data, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
		      "static/apps");
  if (!db) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No application data to save");
    return 0;
  }

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);

  if (AB_Banking_GetUserDataDir(ab, pbuf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not get user data dir");
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_GENERIC;
  }
  GWEN_Buffer_AppendString(pbuf, "/appcfg/");
  pos=GWEN_Buffer_GetPos(pbuf);

  errors=0;
  db=GWEN_DB_GetFirstGroup(db);
  while(db) {
    GWEN_Buffer_Crop(pbuf, 0, pos);
    GWEN_Buffer_AppendString(pbuf, GWEN_DB_GroupName(db));
    GWEN_Buffer_AppendString(pbuf, ".conf");
    if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(pbuf),
			       GWEN_PATH_FLAGS_VARIABLE)) {
      DBG_ERROR(0, "Could not create file \"%s\"",
		GWEN_Buffer_GetStart(pbuf));
      errors++;
    }
    else {
      if (GWEN_DB_WriteFile(db, GWEN_Buffer_GetStart(pbuf),
			    GWEN_DB_FLAGS_DEFAULT)) {
	DBG_ERROR(0, "Could not save file \"%s\"",
		  GWEN_Buffer_GetStart(pbuf));
	errors++;
      }
    }
    db=GWEN_DB_GetNextGroup(db);
  }

  /* sucessfully written */
  if (errors) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Some errors occurred");
    return AB_ERROR_GENERIC;
  }
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
      DBG_ERROR(0, "Could not load app data file");
      return 0;
    }
  }

  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
		       ab->appEscName);
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
  DBG_WARN(AQBANKING_LOGDOMAIN, "No messageBox function set");
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
  DBG_WARN(AQBANKING_LOGDOMAIN, "No showBox function set");
  return 0;
}



void AB_Banking_HideBox(AB_BANKING *ab, GWEN_TYPE_UINT32 id){
  assert(ab);
  if (ab->hideBoxFn) {
    ab->hideBoxFn(ab, id);
    return;
  }
  DBG_WARN(AQBANKING_LOGDOMAIN, "No hideBox function set");
}



GWEN_TYPE_UINT32 AB_Banking_ProgressStart(AB_BANKING *ab,
                                          const char *title,
                                          const char *text,
                                          GWEN_TYPE_UINT32 total){
  assert(ab);
  if (ab->progressStartFn) {
    return ab->progressStartFn(ab, title, text, total);
  }
  DBG_WARN(AQBANKING_LOGDOMAIN, "No progressStart function set");
  return 0;
}



int AB_Banking_ProgressAdvance(AB_BANKING *ab,
                               GWEN_TYPE_UINT32 id,
                               GWEN_TYPE_UINT32 progress){
  assert(ab);
  if (ab->progressAdvanceFn) {
    return ab->progressAdvanceFn(ab, id, progress);
  }
  DBG_WARN(AQBANKING_LOGDOMAIN, "No progressAdvance function set");
  return 0;
}



int AB_Banking_ProgressLog(AB_BANKING *ab,
                           GWEN_TYPE_UINT32 id,
                           AB_BANKING_LOGLEVEL level,
                           const char *text){
  assert(ab);
  if (ab->progressLogFn) {
    return ab->progressLogFn(ab, id, level, text);
  }
  DBG_WARN(AQBANKING_LOGDOMAIN, "No progressLog function set");
  return 0;
}



int AB_Banking_ProgressEnd(AB_BANKING *ab, GWEN_TYPE_UINT32 id){
  assert(ab);
  if (ab->progressEndFn) {
    return ab->progressEndFn(ab, id);
  }
  DBG_WARN(AQBANKING_LOGDOMAIN, "No progressEnd function set");
  return 0;
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
    if (AB_Provider_Init(pro)) {
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




int AB_Banking_Init(AB_BANKING *ab) {
  GWEN_DB_NODE *dbT;
  GWEN_DB_NODE *dbTsrc;
  AB_JOB_LIST2 *jl;
  int i;

  assert(ab);

  if (!GWEN_Logger_IsOpen(AQBANKING_LOGDOMAIN)) {
    GWEN_Logger_Open(AQBANKING_LOGDOMAIN,
                     "aqbanking", 0,
                     GWEN_LoggerTypeConsole,
                     GWEN_LoggerFacilityUser);
  }

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
                       GWEN_PATH_FLAGS_CREATE_GROUP)) {
    GWEN_DB_Group_free(dbT);
    return AB_ERROR_BAD_CONFIG_FILE;
  }

  ab->lastUniqueId=GWEN_DB_GetIntValue(dbT, "lastUniqueId", 0, 0);

  /* read active providers */
  for (i=0; ; i++) {
    const char *p;

    p=GWEN_DB_GetCharValue(dbT, "activeProviders", i, 0);
    if (!p)
      break;
    GWEN_StringList_AppendString(ab->activeProviders, p, 0, 1);
  }

  /* read data */
  dbTsrc=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "banking");
  if (dbTsrc) {
    GWEN_DB_NODE *dbTdst;

    dbTdst=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT, "static");
    GWEN_DB_AddGroupChildren(dbTdst, dbTsrc);
  }

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
        DBG_WARN(AQBANKING_LOGDOMAIN, "Error loading/initializing backend \"%s\"", p);
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

      a=AB_Account_fromDb(ab, dbA);
      if (a) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Adding account");
        AB_Account_List_Add(a, ab->accounts);
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
  return 0;
}




int AB_Banking_Fini(AB_BANKING *ab) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbT;
  AB_ACCOUNT *a;
  AB_JOB *j;
  int rv;

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

  /* deinit active providers */
  if (GWEN_StringList_Count(ab->activeProviders)) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(ab->activeProviders);
    assert(se);
    while(se) {
      const char *p;
      int rv;
      AB_PROVIDER *pro;

      p=GWEN_StringListEntry_Data(se);
      assert(p);

      pro=AB_Banking_FindProvider(ab, p);
      if (pro) {
        rv=AB_Provider_Fini(pro);
        if (rv) {
          DBG_WARN(AQBANKING_LOGDOMAIN, "Error deinitializing backend \"%s\"", p);
        }
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* store appplication specific data */
  rv=AB_Banking__SaveAllAppData(ab);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not save configuration");
    GWEN_DB_Group_free(db);
    return rv;
  }
  GWEN_DB_DeleteGroup(ab->data, "static/apps");

  /* store static config data as "banking" */
  dbT=GWEN_DB_GetGroup(ab->data, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "static");
  if (dbT) {
    GWEN_DB_NODE *dbTdst;

    dbTdst=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "banking");
    assert(dbTdst);
    GWEN_DB_AddGroupChildren(dbTdst, dbT);
  }

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "lastUniqueId", ab->lastUniqueId);

  /* write config file. TODO: make backups */
  if (GWEN_DB_WriteFile(db, ab->configFile,
                        GWEN_DB_FLAGS_DEFAULT)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not save configuration");
    GWEN_DB_Group_free(db);
    return AB_ERROR_BAD_CONFIG_FILE;
  }

  GWEN_DB_Group_free(db);

  AB_Job_List_Clear(ab->enqueuedJobs);
  AB_Account_List_Clear(ab->accounts);
  AB_Provider_List_Clear(ab->providers);
  GWEN_DB_ClearGroup(ab->data, 0);

  GWEN_Logger_Close(AQBANKING_LOGDOMAIN);
  return 0;
}




GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetProviderDescrs(AB_BANKING *ab){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *l;

  l=GWEN_LoadPluginDescrs(AQBANKING_PLUGINS "/providers");
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
                           "/"
                           AB_PROVIDER_WIZARD_FOLDER
                           "/");
  GWEN_Buffer_AppendString(pbuf, pn);

  wdl=GWEN_LoadPluginDescrs(GWEN_Buffer_GetStart(pbuf));

  GWEN_Buffer_free(pbuf);
  return wdl;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetImExporterDescrs(AB_BANKING *ab){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *l;

  l=GWEN_LoadPluginDescrs(AQBANKING_PLUGINS "/imexporters");
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

      DBG_NOTICE(AQBANKING_LOGDOMAIN, "Comparing \"%s\" against \"%s\"",
                 caccountId, accountId);
      if ((strcasecmp(accountId, caccountId)==0) &&
          (strcasecmp(bankCode, cbankCode)==0)) {
        DBG_NOTICE(AQBANKING_LOGDOMAIN, "Match");
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
    if (AB_Banking__MergeInAccount(ab, a)) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Could not merge in account");
    }
    else
      successful++;
    a=AB_Account_List2Iterator_Next(ait);
  }
  AB_Account_List2Iterator_free(ait);

  if (!successful) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No account imported");
    return AB_ERROR_EMPTY;
  }
  return 0;
}



int AB_Banking_EnqueueJob(AB_BANKING *ab, AB_JOB *j){
  int rv;
  AB_JOB_STATUS jst;

  assert(ab);
  assert(j);
  AB_Job_SetUniqueId(j, AB_Banking_GetUniqueId(ab));
  rv=AB_Job_CheckAvailability(j);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Job is not available, refusing to enqueue.");
    return rv;
  }
  AB_Job_Attach(j);
  AB_Job_List_Add(j, ab->enqueuedJobs);
  jst=AB_Job_GetStatus(j);
  if (jst!=AB_Job_StatusEnqueued &&
      jst!=AB_Job_StatusPending)
    AB_Job_SetStatus(j, AB_Job_StatusEnqueued);
  return 0;
}



int AB_Banking_DequeueJob(AB_BANKING *ab, AB_JOB *j){
  int rv;
  AB_JOB_STATUS jst;

  assert(ab);
  assert(j);
  jst=AB_Job_GetStatus(j);
  if (jst==AB_Job_StatusEnqueued)
    AB_Job_SetStatus(j, AB_Job_StatusNew);
  AB_Job_List_Del(j);
  rv=AB_Banking__UnlinkJobAs(ab, j, "todo");
  AB_Job_free(j);
  return rv;
}



int AB_Banking__ExecuteQueue(AB_BANKING *ab){
  AB_PROVIDER *pro;
  int succ;

  assert(ab);
  pro=AB_Provider_List_First(ab->providers);
  succ=0;

  while(pro) {
    AB_JOB *j;
    int jobs;
    int rv;

    j=AB_Job_List_First(ab->enqueuedJobs);
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
          else
            jobs++;
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

	DBG_NOTICE(AQBANKING_LOGDOMAIN, "Error executing backend's queue");
	if (AB_Provider_List_Next(pro)) {
	  int lrv;

          lrv=AB_Banking_MessageBox(ab,
                                    AB_BANKING_MSG_FLAGS_TYPE_ERROR |
                                    AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
                                    AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                                    "Error",
                                    "Error executing backend's queue.\n"
                                    "What shall we do ?",
                                    "Continue", "Abort", 0);
          if (lrv!=1) {
            DBG_INFO(AQBANKING_LOGDOMAIN, "Aborted by user");
            return AB_ERROR_USER_ABORT;
          }
        } /* if more backends to go */
      }
      else
        succ++;
    } /* if jobs in backend's queue */

    pro=AB_Provider_List_Next(pro);
  } /* while */

  if (!succ) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Not a single job successfully executed");
    return AB_ERROR_GENERIC;
  }

  return 0;
}



int AB_Banking_ExecuteQueue(AB_BANKING *ab){
  int rv;
  AB_JOB *j;

  assert(ab);

  rv=AB_Banking__ExecuteQueue(ab);

  /* clear queue from jobs with status !=enqueued */
  j=AB_Job_List_First(ab->enqueuedJobs);
  while(j) {
    AB_JOB *nj;

    nj=AB_Job_List_Next(j);

    if (AB_Job_GetStatus(j)!=AB_Job_StatusEnqueued) {
      AB_Job_Attach(j);
      AB_Banking_DequeueJob(ab, j);

      switch(AB_Job_GetStatus(j)) {
      case AB_Job_StatusPending:
	if (AB_Banking__SaveJobAs(ab, j, "pending")) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not save job as \"pending\"");
	}
	break;

      case AB_Job_StatusSent:
      case AB_Job_StatusFinished:
      case AB_Job_StatusError:
      default:
	if (AB_Banking__SaveJobAs(ab, j, "finished")) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not save job as \"finished\"");
	}
	break;
      }
      AB_Job_free(j);
    }
    j=nj;
  } /* while */

  return rv;
}



int AB_Banking_ActivateProvider(AB_BANKING *ab, const char *pname) {
  int rv;
  AB_PROVIDER *pro;

  if (GWEN_StringList_HasString(ab->activeProviders, pname)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Provider already active");
    return AB_ERROR_FOUND;
  }

  pro=AB_Banking_FindProvider(ab, pname);
  if (pro) {
    if (!AB_Provider_IsInit(pro)) {
      rv=AB_Provider_Init(pro);
      if (rv) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not initialize backend \"%s\"", pname);
        return rv;
      }
    }
  }

  rv=AB_Banking_ImportProviderAccounts(ab, pname);
  if (rv) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Could not import accounts from backend \"%s\"",
             pname);
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
    AB_Provider_Fini(pro);

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

  if (GWEN_Directory_GetHomeDirectory(home, sizeof(home))) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not determine home directory, aborting.");
    return -1;
  }
  GWEN_Buffer_AppendString(buf, home);
  GWEN_Buffer_AppendString(buf, "/.banking");
  return 0;
}



int AB_Banking_GetAppUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf){
  int rv;

  rv=AB_Banking_GetUserDataDir(ab, buf);
  if (rv)
    return rv;
  GWEN_Buffer_AppendString(buf, "/apps");
  return 0;
}



int AB_Banking_GetProviderUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf){
  int rv;

  rv=AB_Banking_GetUserDataDir(ab, buf);
  if (rv)
    return rv;
  GWEN_Buffer_AppendString(buf, "/backends");
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
	  GWEN_Buffer_AppendByte(nbuf, '/');
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
                  DBG_INFO(AQBANKING_LOGDOMAIN,
                           "File \"%s\" contains profile \"%s\"",
                           GWEN_Buffer_GetStart(nbuf), s);
                  GWEN_DB_NODE *dbTarget;

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

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  db=GWEN_DB_Group_new("profiles");

  /* read global profiles */
  GWEN_Buffer_AppendString(buf, DATADIR "/banking/imexporters/");
  if (GWEN_Text_EscapeToBufferTolerant(name, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Bad name for importer/exporter");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_AppendString(buf, "/profiles");
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
  GWEN_Buffer_Reset(buf);

  /* read local user profiles */
  if (AB_Banking_GetUserDataDir(ab, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not get user data dir");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_AppendString(buf, "/imexporters/");
  if (GWEN_Text_EscapeToBufferTolerant(name, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Bad name for importer/exporter");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_AppendString(buf, "/profiles");

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
                      "static/providers");
  assert(db);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT, modname);
  assert(db);

  fn=(AB_PROVIDER_FACTORY_FN)p;
  assert(fn);
  pro=fn(ab, db);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in plugin: No provider created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  AB_Provider_SetLibLoader(pro, ll);

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

  ll=GWEN_LibLoader_new();
  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(mbuf, tolower(*(s++)));
  modname=GWEN_Buffer_GetStart(mbuf);
  if (GWEN_LibLoader_OpenLibraryWithPath(ll,
                                         AQBANKING_PLUGINS
                                         "/"
                                         AB_PROVIDER_FOLDER,
                                         modname)) {
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
                      "static/providers");
  assert(db);
  db=GWEN_DB_GetGroup(ab->data, GWEN_DB_FLAGS_DEFAULT, modname);
  assert(db);

  fn=(AB_PROVIDER_FACTORY_FN)p;
  assert(fn);
  pro=fn(ab, db);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in plugin: No provider created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  AB_Provider_SetLibLoader(pro, ll);

  return pro;
}



int AB_Banking_GetWizardPath(AB_BANKING *ab,
                             const char *backend,
                             GWEN_BUFFER *pbuf){
  const char *s;

  GWEN_Buffer_AppendString(pbuf,
                           AQBANKING_PLUGINS
                           "/"
                           AB_PROVIDER_WIZARD_FOLDER
                           "/");
  s=backend;
  while(*s) GWEN_Buffer_AppendByte(pbuf, tolower(*(s++)));

  return 0;
}



int AB_Banking_SuspendProvider(AB_BANKING *ab, const char *backend){
  AB_PROVIDER *pro;

  pro=AB_Banking_FindProvider(ab, backend);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider not found");
    return AB_ERROR_NOT_FOUND;
  }

  return AB_Provider_Fini(pro);
}



int AB_Banking_ResumeProvider(AB_BANKING *ab, const char *backend){
  AB_PROVIDER *pro;

  pro=AB_Banking_FindProvider(ab, backend);
  if (!pro) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Provider not found");
    return AB_ERROR_NOT_FOUND;
  }

  return AB_Provider_Init(pro);
}




void AB_Banking__AddJobDir(const AB_BANKING *ab,
			   const char *as,
			   GWEN_BUFFER *buf) {
  AB_Banking_GetUserDataDir(ab, buf);
  GWEN_Buffer_AppendString(buf, "/jobs/");
  GWEN_Buffer_AppendString(buf, as);
}



void AB_Banking__AddJobPath(const AB_BANKING *ab,
			    const char *as,
			    GWEN_TYPE_UINT32 jid,
			    GWEN_BUFFER *buf) {
  char buffer[16];

  AB_Banking__AddJobDir(ab, as, buf);
  GWEN_Buffer_AppendByte(buf, '/');
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
	    O_RDWR|O_CREAT /* |O_TRUNC */ ,
	    S_IRUSR|S_IWUSR);
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
    DBG_ERROR(AQBANKING_LOGDOMAIN, "fcntl(%s, F_SETLKW): %s", s, strerror(errno));
    close(fd);
    return -1;
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
    DBG_ERROR(AQBANKING_LOGDOMAIN, "fcntl(F_SETLK): %s", strerror(errno));
    close(fd);
    return -1;
  }
#endif

  if (close(fd)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "close: %s", strerror(errno));
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
    GWEN_BufferedIO_Abandon(bio);
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
    GWEN_BufferedIO_Abandon(bio);
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
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    AB_Banking__CloseJob(ab, fd);
    return -1;
  }

  GWEN_DB_Group_free(dbJob);
  GWEN_BufferedIO_Close(bio);
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
    if (unlink(GWEN_Buffer_GetStart(pbuf))) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "unlink(%s): %s",
		GWEN_Buffer_GetStart(pbuf),
		strerror(errno));
      GWEN_Buffer_free(pbuf);
      AB_Banking__CloseFile(fd);
      return AB_ERROR_GENERIC;
    }
    AB_Banking__CloseFile(fd);
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
	  GWEN_Buffer_AppendByte(pbuf, '/');
	  GWEN_Buffer_AppendString(pbuf, nbuffer);

	  /* job found */
	  j=AB_Banking__LoadJobFile(ab, GWEN_Buffer_GetStart(pbuf));
	  if (!j) {
	    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in job file \"%s\"",
		      GWEN_Buffer_GetStart(pbuf));
	  }
	  else {
	    DBG_INFO(AQBANKING_LOGDOMAIN, "Adding job \"%s\"", GWEN_Buffer_GetStart(pbuf));
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
  int rv;

  assert(ab);
  assert(j);
  if (strcasecmp(ab->appName, AB_Job_GetCreatedBy(j))==0)
    rv=AB_Banking__UnlinkJobAs(ab, j, "finished");
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Job can only be removed by its creator application");
    rv=AB_ERROR_INVALID;
  }
  return rv;
}




AB_JOB_LIST2 *AB_Banking_GetPendingJobs(AB_BANKING *ab) {
  return AB_Banking__LoadJobsAs(ab, "pending");
}



int AB_Banking_DelPendingJob(AB_BANKING *ab, AB_JOB *j){
  int rv;

  assert(ab);
  assert(j);
  if (strcasecmp(ab->appName, AB_Job_GetCreatedBy(j))==0)
    rv=AB_Banking__UnlinkJobAs(ab, j, "pending");
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Job can only be removed by its creator application");
    rv=AB_ERROR_INVALID;
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
  GWEN_ERRORCODE err;
  GWEN_BUFFER *mbuf;
  GWEN_DB_NODE *db;

  ll=GWEN_LibLoader_new();
  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(mbuf, tolower(*(s++)));
  modname=GWEN_Buffer_GetStart(mbuf);
  if (GWEN_LibLoader_OpenLibraryWithPath(ll,
                                         AQBANKING_PLUGINS
                                         "/"
                                         AB_IMEXPORTER_FOLDER,
                                         modname)) {
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










