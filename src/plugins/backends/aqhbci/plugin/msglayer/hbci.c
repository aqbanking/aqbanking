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

/* #define AH_HBCI_HEAVY_DEBUG */

#ifdef OS_WIN32
# define AH_PATH_SEP "\\"
#else
# define AH_PATH_SEP "/"
#endif

#include "hbci_p.h"
#include "aqhbci_l.h"
#include "hbci-updates_l.h"
#include "medium_l.h"

#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/net2.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/pathmanager.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>



AH_HBCI *AH_HBCI_new(AB_PROVIDER *pro){
  AH_HBCI *hbci;
  char numbuf[32];
  int rv;

  assert(pro);

  if (!GWEN_Logger_IsOpen(AQHBCI_LOGDOMAIN)) {
    GWEN_Logger_Open(AQHBCI_LOGDOMAIN,
		     "aqhbci",
		     0,
		     GWEN_LoggerTypeConsole,
		     GWEN_LoggerFacilityUser);
  }

  GWEN_NEW_OBJECT(AH_HBCI, hbci);
  hbci->provider=pro;
  hbci->banking=AB_Provider_GetBanking(pro);
  hbci->activeMedia=AH_Medium_List_new();
  hbci->productName=strdup("AQHBCI");
  rv=snprintf(numbuf, sizeof(numbuf), "%d.%d",
              AQHBCI_VERSION_MAJOR, AQHBCI_VERSION_MINOR);
  if (rv==-1 || rv>=sizeof(numbuf)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "What ?? %zd bytes isn't enough space for two decimals ?!",
	      sizeof(numbuf));
    hbci->productVersion=strdup("0");
  }
  else
    hbci->productVersion=strdup(numbuf);

  hbci->transferTimeout=AH_HBCI_DEFAULT_TRANSFER_TIMEOUT;
  hbci->connectTimeout=AH_HBCI_DEFAULT_CONNECT_TIMEOUT;

  return hbci;
}



void AH_HBCI_free(AH_HBCI *hbci){
  if (hbci) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Destroying AH_HBCI");

    /* free current medium */
    if (hbci->currentMedium) {
      if (AH_Medium_IsMounted(hbci->currentMedium)) {
        if (AH_Medium_Unmount(hbci->currentMedium, 1)) {
          DBG_WARN(AQHBCI_LOGDOMAIN, "Could not unmount medium");
        }
      }
      AH_Medium_free(hbci->currentMedium);
      hbci->currentMedium=0;
    }

    /* free active media */
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "%d active media",
             AH_Medium_List_GetCount(hbci->activeMedia));
    AH_Medium_List_free(hbci->activeMedia);

    free(hbci->productName);
    free(hbci->productVersion);

    GWEN_XMLNode_free(hbci->defs);

    GWEN_FREE_OBJECT(hbci);
    GWEN_Logger_Close(AQHBCI_LOGDOMAIN);
  }
}



int AH_HBCI__LoadMedia(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  GWEN_DB_NODE *dbMedia;
  GWEN_DB_NODE *dbT;

  dbMedia=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                           "media");
  if (dbMedia) {
    dbT=GWEN_DB_FindFirstGroup(dbMedia, "medium");
    while(dbT) {
      AH_MEDIUM *m;
      const char *typeName;
      const char *subTypeName;
      const char *name;
  
      name=GWEN_DB_GetCharValue(dbT, "mediumName", 0, 0);
      assert(name);
      typeName=GWEN_DB_GetCharValue(dbT, "mediumTypeName", 0, 0);
      assert(typeName);
      subTypeName=GWEN_DB_GetCharValue(dbT, "mediumSubTypeName", 0, 0);
  
      m=AH_HBCI_FindMedium(hbci, typeName, name);
      if (m) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Medium \"%s\"already registered, invalid setup",
                  name);
        return -1;
      }
  
      m=AH_HBCI_MediumFactoryDb(hbci, typeName, subTypeName, dbT);
      assert(m);
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Loaded medium \"%s\"", name);
      AH_HBCI__AddMedium(hbci, m);
  
      dbT=GWEN_DB_FindNextGroup(dbT, "medium");
    }
  }
  else {
    DBG_WARN(AQHBCI_LOGDOMAIN, "No media in configuration file");
  }
  return 0;
}



int AH_HBCI__SaveMedia(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  AH_MEDIUM *m;

  m=AH_Medium_List_First(hbci->activeMedia);
  if (m) {
    GWEN_DB_NODE *dbMedia;

    dbMedia=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                             "media");
    assert(dbMedia);
    while(m) {
      GWEN_DB_NODE *dbT;

      dbT=GWEN_DB_GetGroup(dbMedia, GWEN_PATH_FLAGS_CREATE_GROUP, "medium");
      assert(dbT);
      if (AH_Medium_ToDB(m, dbT)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Error storing medium \"%s\"",
                  AH_Medium_GetMediumName(m));
        return -1;
      }
      m=AH_Medium_List_Next(m);
    }
  }

  return 0;
}



int AH_HBCI_Init(AH_HBCI *hbci) {
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *db;
  int rv;

  assert(hbci);

  /* load and update config data */
  db=AB_Provider_GetData(hbci->provider);

  hbci->lastVersion=GWEN_DB_GetIntValue(db, "lastVersion", 0, 0);
  rv=AH_HBCI_UpdateDb(hbci, db);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  GWEN_PathManager_DefinePath(AH_PM_LIBNAME, AH_PM_XMLDATADIR);
  GWEN_PathManager_AddPath(AH_PM_LIBNAME,
                           AH_PM_LIBNAME,
                           AH_PM_XMLDATADIR,
                           AH_XMLDATADIR);
  GWEN_PathManager_AddPathFromWinReg(AH_PM_LIBNAME,
                                     AH_PM_LIBNAME,
                                     AH_PM_XMLDATADIR,
                                     AH_REGKEY_PATHS,
                                     AH_REGKEY_XMLDATADIR);

  /* Load XML files */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Loading XML files");
  node=AH_HBCI_LoadDefaultXmlFiles(hbci);
  if (!node) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "AqHBCI: Error loading XML files.");
    return 0;
  }

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Adding XML descriptions");
  if (AH_HBCI_AddDefinitions(hbci, node)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "ERROR: Could not add XML definitions.\n");
    GWEN_XMLNode_free(node);
    return 0;
  }
  GWEN_XMLNode_free(node);

  hbci->sharedRuntimeData=GWEN_DB_Group_new("sharedRuntimeData");

  hbci->transferTimeout=GWEN_DB_GetIntValue(db, "transferTimeout", 0,
					    AH_HBCI_DEFAULT_TRANSFER_TIMEOUT);
  hbci->connectTimeout=GWEN_DB_GetIntValue(db, "connectTimeout", 0,
					   AH_HBCI_DEFAULT_CONNECT_TIMEOUT);
  hbci->lastMediumId=GWEN_DB_GetIntValue(db, "lastMediumId", 0, 0);


  /* load media */
  AH_HBCI__LoadMedia(hbci, db);

  return 0;
}



int AH_HBCI_Fini(AH_HBCI *hbci) {
  GWEN_DB_NODE *db;
  GWEN_TYPE_UINT32 currentVersion;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Deinitializing AH_HBCI");
  assert(hbci);

  currentVersion=
    (AQHBCI_VERSION_MAJOR<<24) |
    (AQHBCI_VERSION_MINOR<<16) |
    (AQHBCI_VERSION_PATCHLEVEL<<8) |
    AQHBCI_VERSION_BUILD;

  /* free current medium */
  if (hbci->currentMedium) {
    if (AH_Medium_IsMounted(hbci->currentMedium)) {
      if (AH_Medium_Unmount(hbci->currentMedium, 1)) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "Could not unmount medium");
      }
    }
    AH_Medium_free(hbci->currentMedium);
    hbci->currentMedium=0;
  }

  /* save configuration */
  db=AB_Provider_GetData(hbci->provider);
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Setting version %08x",
             currentVersion);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "lastVersion", currentVersion);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "lastMediumId", hbci->lastMediumId);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "transferTimeout",
                      hbci->transferTimeout);

  GWEN_DB_SetIntValue(db,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "connectTimeout",
                      hbci->connectTimeout);

  /* save all active media */
  AH_HBCI__SaveMedia(hbci, db);

  /* clear active media */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "%d active media",
            AH_Medium_List_GetCount(hbci->activeMedia));
  AH_Medium_List_Clear(hbci->activeMedia);

  GWEN_PathManager_UndefinePath(AH_PM_LIBNAME, AH_PM_XMLDATADIR);

  GWEN_DB_Group_free(hbci->sharedRuntimeData);
  hbci->sharedRuntimeData=0;

  GWEN_XMLNode_free(hbci->defs);
  hbci->defs=0;

  return 0;
}



int AH_HBCI_Update(AH_HBCI *hbci,
                   GWEN_TYPE_UINT32 lastVersion,
                   GWEN_TYPE_UINT32 currentVersion) {
  GWEN_DB_NODE *db;
  int rv;

  db=AB_Provider_GetData(hbci->provider);
  assert(db);
  rv=AH_HBCI_Update2(hbci, db, lastVersion, currentVersion);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



AB_BANKING *AH_HBCI_GetBankingApi(const AH_HBCI *hbci){
  assert(hbci);
  return hbci->banking;
}



const char *AH_HBCI_GetProductName(const AH_HBCI *hbci){
  assert(hbci);
  return hbci->productName;
}



void AH_HBCI_SetProductName(AH_HBCI *hbci, const char *s){
  assert(hbci);
  assert(s);
  free(hbci->productName);
  hbci->productName=strdup(s);
}



const char *AH_HBCI_GetProductVersion(const AH_HBCI *hbci){
  assert(hbci);
  return hbci->productVersion;
}



void AH_HBCI_SetProductVersion(AH_HBCI *hbci, const char *s){
  assert(hbci);
  assert(s);
  free(hbci->productVersion);
  hbci->productVersion=strdup(s);
}



GWEN_XMLNODE *AH_HBCI_GetDefinitions(const AH_HBCI *hbci){
  assert(hbci);
  return hbci->defs;
}


GWEN_XMLNODE *AH_HBCI_LoadDefaultXmlFiles(const AH_HBCI *hbci){
  GWEN_XMLNODE *xmlNode;
  GWEN_STRINGLIST *slist;

  xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");

  slist=GWEN_PathManager_GetPaths(AH_PM_LIBNAME, AH_PM_XMLDATADIR);
  if (GWEN_XML_ReadFileSearch(xmlNode,
                              "hbci.xml",
                              GWEN_XML_FLAGS_DEFAULT |
			      GWEN_XML_FLAGS_HANDLE_HEADERS,
                              slist) ) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not load XML file.\n");
    GWEN_StringList_free(slist);
    GWEN_XMLNode_free(xmlNode);
    return 0;
  }
  GWEN_StringList_free(slist);

  return xmlNode;
}



int AH_HBCI_AddDefinitions(AH_HBCI *hbci, GWEN_XMLNODE *node) {
  GWEN_XMLNODE *nsrc, *ndst;

  assert(node);

  if (!hbci->defs) {
    hbci->defs=GWEN_XMLNode_dup(node);
    return 0;
  }

  nsrc=GWEN_XMLNode_GetChild(node);
  while(nsrc) {
    if (GWEN_XMLNode_GetType(nsrc)==GWEN_XMLNodeTypeTag) {
      ndst=GWEN_XMLNode_FindNode(hbci->defs, GWEN_XMLNodeTypeTag,
                                 GWEN_XMLNode_GetData(nsrc));
      if (ndst) {
	GWEN_XMLNODE *n;

        n=GWEN_XMLNode_GetChild(nsrc);
	while (n) {
	  GWEN_XMLNODE *newNode;

          DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding node \"%s\"", GWEN_XMLNode_GetData(n));
          newNode=GWEN_XMLNode_dup(n);
          GWEN_XMLNode_AddChild(ndst, newNode);
	  n=GWEN_XMLNode_Next(n);
	} /* while n */
      }
      else {
	GWEN_XMLNODE *newNode;

        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding branch \"%s\"", GWEN_XMLNode_GetData(nsrc));
	newNode=GWEN_XMLNode_dup(nsrc);
        GWEN_XMLNode_AddChild(hbci->defs, newNode);
      }
    } /* if TAG */
    nsrc=GWEN_XMLNode_Next(nsrc);
  } /* while */

  return 0;
}



void AH_HBCI_AppendUniqueName(AH_HBCI *hbci, GWEN_BUFFER *nbuf) {
  char buffer[64];
  time_t currentTime;
  struct tm *currentTimeTm;
  int rv;

  currentTime=time(0);
  currentTimeTm=localtime(&currentTime);
  assert(currentTimeTm);

  rv=snprintf(buffer,
              sizeof(buffer)-1, "%04d%02d%02d-%02d%02d%02d-%d",
              currentTimeTm->tm_year+1900,
              currentTimeTm->tm_mon+1,
              currentTimeTm->tm_mday,
              currentTimeTm->tm_hour,
              currentTimeTm->tm_min,
              currentTimeTm->tm_sec,
              ++(hbci->counter));
  assert(rv>0 && rv<sizeof(buffer));
  GWEN_Buffer_AppendString(nbuf, buffer);
}




int AH_HBCI_GetMedium(AH_HBCI *hbci, AB_USER *u, AH_MEDIUM **pm){
  AH_MEDIUM *m;
  int rv;

  assert(hbci);
  assert(u);

  if (hbci->currentMedium==AH_User_GetMedium(u)) {
    /* return current medium if it is the wanted one, release it otherwise */
    if (AH_Medium_SelectContext(hbci->currentMedium,
                                AH_User_GetContextIdx(u))==0) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Returning current medium");
      *pm=hbci->currentMedium;
      return 0;
    }
    if (AH_Medium_IsMounted(hbci->currentMedium)) {
      rv=AH_Medium_Unmount(hbci->currentMedium, 0);
      if (rv) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not unmount medium (%d)", rv);
        return rv;
      }
    }
    AH_Medium_free(hbci->currentMedium);
    hbci->currentMedium=0;
  }

  m=AH_User_GetMedium(u);
  assert(m);

  if (!AH_Medium_IsMounted(m)) {
    rv=AH_Medium_Mount(m);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not mount medium (%d)", rv);
      return rv;
    }
  }

  /* store current medium */
  hbci->currentMedium=m;
  AH_Medium_Attach(m);

  rv=AH_Medium_SelectContext(hbci->currentMedium,
                             AH_User_GetContextIdx(u));
  if (rv){
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Error selecting context %d for \"%s:%s/%s\" (%d)",
              AH_User_GetContextIdx(u),
              AB_User_GetCountry(u),
              AB_User_GetBankCode(u),
              AB_User_GetUserId(u),
              rv);
    return rv;
  }

  *pm=m;
  return 0;
}



AH_MEDIUM*
AH_HBCI_MediumFactory(AH_HBCI *hbci,
                      const char *typeName,
		      const char *subTypeName,
		      const char *mediumName) {
  AH_MEDIUM *m;

  m=AH_Medium_new(hbci, typeName, subTypeName, mediumName);
  if (!m) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No medium created");
    return 0;
  }
  return m;
}



AH_MEDIUM*
AH_HBCI_MediumFactoryDb(AH_HBCI *hbci,
                        const char *typeName,
			const char *subTypeName,
			GWEN_DB_NODE *db) {
  AH_MEDIUM *m;
  int rv;
  const char *mediumName;

  mediumName=GWEN_DB_GetCharValue(db, "mediumName", 0, 0);
  m=AH_HBCI_MediumFactory(hbci, typeName, subTypeName, mediumName);
  if (!m) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Medium plugin does not create a medium");
    return 0;
  }

  rv=AH_Medium_FromDB(m, db);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error restoring medium \"%s\" (%d)",
             typeName, rv);
    AH_Medium_free(m);
    return 0;
  }

  return m;
}



int AH_HBCI_CheckMedium(AH_HBCI *hbci,
                        GWEN_CRYPTTOKEN_DEVICE devt,
                        GWEN_BUFFER *typeName,
                        GWEN_BUFFER *subTypeName,
                        GWEN_BUFFER *tokenName) {
  GWEN_PLUGIN_MANAGER *pm;
  int rv;

  pm=GWEN_PluginManager_FindPluginManager("crypttoken");
  if (pm==0) {
    DBG_ERROR(0, "Plugin manager not found");
    return AB_ERROR_GENERIC;
  }

  rv=GWEN_CryptManager_CheckToken(pm, devt,
                                  typeName,
                                  subTypeName,
                                  tokenName);
  if (rv) {
    DBG_ERROR(0, "Token is not supported by any plugin");
    return AB_ERROR_NOT_FOUND;
  }

  return 0;
}



AH_MEDIUM *AH_HBCI_FindMedium(const AH_HBCI *hbci,
                              const char *typeName,
                              const char *mediumName){
  AH_MEDIUM *m;

  assert(hbci);
  assert(typeName);
  m=AH_Medium_List_First(hbci->activeMedia);
  if (!mediumName)
    mediumName="";

  while(m) {
    if (strcasecmp(typeName, AH_Medium_GetMediumTypeName(m))==0 &&
        strcasecmp(mediumName, AH_Medium_GetMediumName(m))==0)
      return m;
    m=AH_Medium_List_Next(m);
  } /* while */

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Medium \"%s\" (%s) not found", mediumName, typeName);
  return 0;
}



AH_MEDIUM *AH_HBCI_FindMediumById(const AH_HBCI *hbci, GWEN_TYPE_UINT32 id){
  AH_MEDIUM *m;

  assert(hbci);
  m=AH_Medium_List_First(hbci->activeMedia);
  while(m) {
    if (id==AH_Medium_GetUniqueId(m))
      return m;
    m=AH_Medium_List_Next(m);
  } /* while */

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Medium \"%08x\" not found", id);
  return 0;
}



AH_MEDIUM *AH_HBCI_SelectMedium(AH_HBCI *hbci,
                                const char *typeName,
				const char *subTypeName,
                                const char *mediumName){
  AH_MEDIUM *m;

  m=0;
  if (mediumName && typeName)
    m=AH_HBCI_FindMedium(hbci, typeName, mediumName);
  if (!m) {
    m=AH_HBCI_MediumFactory(hbci, typeName, subTypeName, mediumName);
    if (!m) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create medium \"%s\" (%s)",
               mediumName, typeName);
      return 0;
    }
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding medium \"%s\" (%s) to the list",
             mediumName, typeName);
    AH_Medium_List_Add(m, hbci->activeMedia);
  }

  return m;
}



AH_MEDIUM *AH_HBCI_SelectMediumDb(AH_HBCI *hbci,
                                  const char *typeName,
				  const char *subTypeName,
                                  GWEN_DB_NODE *db) {
  AH_MEDIUM *m;
  const char *mediumName;

  m=0;
  mediumName=GWEN_DB_GetCharValue(db, "mediumName", 0, 0);
  if (mediumName && typeName)
    m=AH_HBCI_FindMedium(hbci, typeName, mediumName);
  if (!m) {
    m=AH_HBCI_MediumFactoryDb(hbci, typeName, subTypeName, db);
    if (!m) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create medium \"%s\" (%s)",
               mediumName, typeName);
      return 0;
    }
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding medium \"%s\" (%s) to the list",
             mediumName, typeName);
    AH_Medium_List_Add(m, hbci->activeMedia);
  }

  return m;
}



int AH_HBCI__AddMedium(AH_HBCI *hbci, AH_MEDIUM *m) {
  assert(hbci);
  assert(m);

  if (AH_HBCI_FindMediumById(hbci,
                             AH_Medium_GetUniqueId(m))) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Medium already listed");
    return -1;
  }

  AH_Medium_List_Add(m, hbci->activeMedia);
  return 0;
}



int AH_HBCI_AddMedium(AH_HBCI *hbci, AH_MEDIUM *m) {
  GWEN_TYPE_UINT32 uid;

  assert(hbci);
  assert(m);

  uid=AH_Medium_GetUniqueId(m);
  if (uid==0)
    AH_Medium_SetUniqueId(m, ++(hbci->lastMediumId));

  return AH_HBCI__AddMedium(hbci, m);
}



int AH_HBCI_RemoveMedium(AH_HBCI *hbci, AH_MEDIUM *m) {
  assert(hbci);
  assert(m);
  AH_Medium_List_Del(m);
  return 0;
}



const AH_MEDIUM_LIST *AH_HBCI_GetMediaList(const AH_HBCI *hbci) {
  assert(hbci);
  return hbci->activeMedia;
}



GWEN_PLUGIN_DESCRIPTION_LIST2*
AH_HBCI_GetMediumPluginDescrs(AH_HBCI *hbci,
			      GWEN_CRYPTTOKEN_DEVICE dev) {
  GWEN_PLUGIN_DESCRIPTION_LIST2 *cl = 0;
  GWEN_PLUGIN_MANAGER *pm;

  /* load complete list */
  pm=GWEN_PluginManager_FindPluginManager("crypttoken");
  if (!pm) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "Could not find plugin manager for \"%s\"",
              "crypttoken");
    return 0;
  }

  cl=GWEN_CryptManager_GetPluginDescrs(pm, dev);
  if (!cl) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No matching plugin descriptions");
    return 0;
  }

  return cl;
}



int AH_HBCI_AddBankCertFolder(AH_HBCI *hbci,
                              const AB_USER *u,
                              GWEN_BUFFER *nbuf) {
  AH_HBCI_AddObjectPath(hbci,
                        AB_User_GetCountry(u),
                        AB_User_GetBankCode(u),
                        0, 0, 0, nbuf);
  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP "certs");
  return 0;
}



int AH_HBCI_RemoveAllBankCerts(AH_HBCI *hbci, const AB_USER *u) {
  GWEN_DIRECTORYDATA *d;
  GWEN_BUFFER *nbuf;
  char nbuffer[64];
  unsigned int pathLen;

  assert(hbci);

  /* create path */
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AH_HBCI_AddBankCertFolder(hbci, u, nbuf);
  pathLen=GWEN_Buffer_GetUsedBytes(nbuf);

  d=GWEN_Directory_new();
  if (GWEN_Directory_Open(d, GWEN_Buffer_GetStart(nbuf))) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Path \"%s\" is not available",
             GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);
    GWEN_Directory_free(d);
    return -1;
  }

  while(!GWEN_Directory_Read(d,
                             nbuffer,
                             sizeof(nbuffer))) {
    if (strcmp(nbuffer, ".") &&
        strcmp(nbuffer, "..")) {
      struct stat st;

      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Removing cert \"%s\"", nbuffer);
      GWEN_Buffer_Crop(nbuf, 0, pathLen);
      GWEN_Buffer_SetPos(nbuf, pathLen);
      GWEN_Buffer_AppendByte(nbuf, '/');
      GWEN_Buffer_AppendString(nbuf, nbuffer);

      if (stat(GWEN_Buffer_GetStart(nbuf), &st)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "stat(%s): %s",
                  GWEN_Buffer_GetStart(nbuf),
                  strerror(errno));
      }
      else {
        if (!S_ISDIR(st.st_mode)) {
          DBG_DEBUG(AQHBCI_LOGDOMAIN, "Removing cert \"%s\"", nbuffer);
          if (unlink(GWEN_Buffer_GetStart(nbuf))) {
            DBG_ERROR(AQHBCI_LOGDOMAIN, "unlink(%s): %s",
                      GWEN_Buffer_GetStart(nbuf),
                      strerror(errno));
          }
        } /* if !dir */
      } /* if stat was ok */
    } /* if not "." and not ".." */
  } /* while */
  GWEN_Directory_Close(d);
  GWEN_Directory_free(d);
  GWEN_Buffer_free(nbuf);

  return 0;
}



int AH_HBCI_SaveSettings(const char *path, GWEN_DB_NODE *db){
  GWEN_BUFFER *nbuf;

  /* check for existence of that file */
  if (GWEN_Directory_GetPath(path,
                             GWEN_PATH_FLAGS_ESCAPE |
                             GWEN_PATH_FLAGS_TOLERANT_ESCAPE |
                             GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Path \"%s\" is not available", path);
    return -1;
  }

  /* escape path */
  nbuf=GWEN_Buffer_new(0, 64, 0, 1);
  if (GWEN_Path_Convert(path, nbuf,
                        GWEN_PATH_FLAGS_CHECKROOT |
                        GWEN_PATH_FLAGS_ESCAPE |
                        GWEN_PATH_FLAGS_TOLERANT_ESCAPE |
                        GWEN_PATH_FLAGS_VARIABLE)) {
    GWEN_Buffer_free(nbuf);
    return -1;
  }

  /* write file */
  if (GWEN_DB_WriteFile(db,
                        GWEN_Buffer_GetStart(nbuf),
                        GWEN_DB_FLAGS_DEFAULT)) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not write file \"%s\"",
             GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);
    return -1;
  }
  GWEN_Buffer_free(nbuf);

  return 0;
}



GWEN_DB_NODE *AH_HBCI_LoadSettings(const char *path) {
  GWEN_BUFFER *nbuf;
  GWEN_DB_NODE *db;

  /* check for existence of the file */
  if (GWEN_Directory_GetPath(path,
                             GWEN_PATH_FLAGS_PATHMUSTEXIST |
                             GWEN_PATH_FLAGS_ESCAPE |
                             GWEN_PATH_FLAGS_TOLERANT_ESCAPE |
                             GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Path \"%s\" does not exist", path);
    return 0;
  }

  /* escape path */
  nbuf=GWEN_Buffer_new(0, 64, 0, 1);
  if (GWEN_Path_Convert(path, nbuf,
                        GWEN_PATH_FLAGS_CHECKROOT |
                        GWEN_PATH_FLAGS_ESCAPE |
                        GWEN_PATH_FLAGS_TOLERANT_ESCAPE |
                        GWEN_PATH_FLAGS_VARIABLE)) {
    GWEN_Buffer_free(nbuf);
    return 0;
  }

  /* file exists, load it */
  db=GWEN_DB_Group_new("cfg");
  if (GWEN_DB_ReadFile(db,
                       GWEN_Buffer_GetStart(nbuf),
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_CREATE_GROUP)) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not read file \"%s\"",
             GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);
    GWEN_DB_Group_free(db);
    return 0;
  }
  GWEN_Buffer_free(nbuf);

  return db;
}



int AH_HBCI_SaveMessage(AH_HBCI *hbci,
                        const AB_USER *u,
                        GWEN_DB_NODE *dbMsg) {
  GWEN_BUFFER *nbuf;
  int rv;

  assert(hbci);
  assert(u);

  /* create path */
  nbuf=GWEN_Buffer_new(0, 64, 0, 1);
  AH_HBCI_AddCustomerPath(hbci, u, nbuf);

  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP "messages" 
			   AH_PATH_SEP "in" AH_PATH_SEP);
  AH_HBCI_AppendUniqueName(hbci, nbuf);
  GWEN_Buffer_AppendString(nbuf, ".msg");

  rv=AH_HBCI_SaveSettings(GWEN_Buffer_GetStart(nbuf), dbMsg);
  GWEN_Buffer_free(nbuf);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not save message");
    return -1;

  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message for customer saved");
  return 0;
}



int AH_HBCI_AddBankPath(const AH_HBCI *hbci,
                        const AB_USER *u,
                        GWEN_BUFFER *nbuf) {
  const char *country;
  const char *bankId;

  assert(hbci);
  assert(nbuf);
  country=AB_User_GetCountry(u);
  if (!country)
    country="de";
  bankId=AB_User_GetBankCode(u);
  assert(bankId);

  AB_Provider_GetUserDataDir(hbci->provider, nbuf);
  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP "banks" AH_PATH_SEP);
  while(*country)
    GWEN_Buffer_AppendByte(nbuf, tolower(*(country++)));
  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP);
  GWEN_Buffer_AppendString(nbuf, bankId);
  return 0;
}



int AH_HBCI_AddUserPath(const AH_HBCI *hbci,
                        const AB_USER *u,
                        GWEN_BUFFER *nbuf) {
  const char *userId;

  assert(hbci);
  assert(u);
  if (AH_HBCI_AddBankPath(hbci, u, nbuf))
    return -1;

  userId=AB_User_GetUserId(u);
  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP "users" AH_PATH_SEP);

  /* escape and append user name */
  if (GWEN_Path_Convert(userId,
                        nbuf,
                        GWEN_PATH_FLAGS_ESCAPE |
                        GWEN_PATH_FLAGS_TOLERANT_ESCAPE)) {
    return -1;
  }
  return 0;
}



int AH_HBCI_AddCustomerPath(const AH_HBCI *hbci,
                            const AB_USER *u,
                            GWEN_BUFFER *nbuf) {
  const char *customerId;

  assert(hbci);
  assert(u);
  if (AH_HBCI_AddUserPath(hbci, u, nbuf))
    return -1;
  GWEN_Buffer_AppendByte(nbuf, '/');

  /* escape and append customer name */
  customerId=AB_User_GetCustomerId(u);
  if (GWEN_Path_Convert(customerId,
                        nbuf,
                        GWEN_PATH_FLAGS_ESCAPE |
                        GWEN_PATH_FLAGS_TOLERANT_ESCAPE)) {
    return -1;
  }
  return 0;
}



int AH_HBCI_AddObjectPath(const AH_HBCI *hbci,
                          const char *country,
                          const char *bankId,
                          const char *accountId,
                          const char *userId,
                          const char *customerId,
                          GWEN_BUFFER *nbuf) {
  assert(hbci);

  AB_Provider_GetUserDataDir(hbci->provider, nbuf);
  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP "banks" AH_PATH_SEP);

  if (country==0)
    return 0;
  GWEN_Buffer_AppendString(nbuf, country);

  if (!bankId)
    return 0;
  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP);
  GWEN_Buffer_AppendString(nbuf, bankId);

  if (accountId) {
    GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP "accounts" AH_PATH_SEP);
    if (GWEN_Path_Convert(accountId,
                          nbuf,
                          GWEN_PATH_FLAGS_ESCAPE |
                          GWEN_PATH_FLAGS_TOLERANT_ESCAPE)) {
      return -1;
    }
  }
  else {
    /* escape and append user name */
    if (!userId)
      return 0;
    GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP "users" AH_PATH_SEP);
    if (GWEN_Path_Convert(userId,
                          nbuf,
                          GWEN_PATH_FLAGS_ESCAPE |
                          GWEN_PATH_FLAGS_TOLERANT_ESCAPE)) {
      return -1;
    }

    /* escape and append customer name */
    if (!customerId)
      return 0;
    if (GWEN_Path_Convert(customerId,
                          nbuf,
                          GWEN_PATH_FLAGS_ESCAPE |
                          GWEN_PATH_FLAGS_TOLERANT_ESCAPE)) {
      return -1;
    }
  }

  return 0;
}



int AH_HBCI_CheckStringSanity(const char *s) {
  assert(s);
  while(*s) {
    if (iscntrl(*s) || isspace(*s)) {
      return -1;
    }
    s++;
  } /* while */
  return 0;
}



AB_PROVIDER *AH_HBCI_GetProvider(const AH_HBCI *hbci) {
  assert(hbci);
  return hbci->provider;
}



GWEN_DB_NODE *AH_HBCI_GetSharedRuntimeData(const AH_HBCI *hbci){
  assert(hbci);
  return hbci->sharedRuntimeData;
}



int AH_HBCI_UnmountCurrentMedium(AH_HBCI *hbci){
  if (hbci->currentMedium) {
    if (AH_Medium_IsMounted(hbci->currentMedium)) {
      if (AH_Medium_Unmount(hbci->currentMedium, 0)) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Could not unmount medium");
        return 0;
      }
    }
    AH_Medium_free(hbci->currentMedium);
    hbci->currentMedium=0;
  }
  return 0;
}



void AH_HBCI_HbciToUtf8(const char *p,
                        int size,
                        GWEN_BUFFER *buf) {
  assert(p);
  assert(buf);
  if (size==0)
    size=strlen(p);

  while(*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    switch(c) {
    case 0xc4: /* AE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x84);
      break;

    case 0xe4: /* ae */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xe4);
      break;

    case 0xd6: /* OE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x96);
      break;

    case 0xf6: /* oe */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xf6);
      break;

    case 0xdc: /* UE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9c);
      break;

    case 0xfc: /* ue */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xfc);
      break;

    case 0xdf: /* sz */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9f);
      break;

    case 0xa7: /* section sign */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa7);
      break;

      /* english chars */
    case 0xa3: /* pound swign */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa3);
      break;

      /* french chars */
    case 0xc7: /* C cedille */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xc7);
      break;

    case 0xe0: /* a accent grave */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xe0);
      break;

    case 0xe1: /* a accent aigu */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xe1);
      break;

    case 0xe2: /* a accent circumflex */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xe2);
      break;

    case 0xe7: /* c cedille */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xe7);
      break;

    case 0xe8: /* e accent grave */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xe8);
      break;

    case 0xe9: /* e accent aigu */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xe9);
      break;

    case 0xea: /* e accent circumflex */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xea);
      break;

    case 0xec: /* i accent grave (never heard of this) */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xec);
      break;

    case 0xed: /* i accent aigu (never heard of this, either) */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xed);
      break;

    case 0xee: /* i accent circumflex (never heard of this, either) */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xee);
      break;

    case 0xf2: /* o accent grave */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xf2);
      break;

    case 0xf3: /* o accent aigu */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xf3);
      break;

    case 0xf4: /* o accent circumflex */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xf4);
      break;

    case 0xf9: /* u accent grave */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xf9);
      break;

    case 0xfa: /* u accent aigu */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xfa);
      break;

    case 0xfb: /* u accent circumflex */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xfb);
      break;

    default:
      GWEN_Buffer_AppendByte(buf, c);
    }
    if (size!=-1)
      size--;
  } /* while */
}



int AH_HBCI_GetTransferTimeout(const AH_HBCI *hbci) {
  assert(hbci);
  return hbci->transferTimeout;
}



void AH_HBCI_SetTransferTimeout(AH_HBCI *hbci, int i){
  assert(hbci);
  hbci->transferTimeout=i;
}



int AH_HBCI_GetConnectTimeout(const AH_HBCI *hbci) {
  assert(hbci);
  return hbci->connectTimeout;
}



void AH_HBCI_SetConnectTimeout(AH_HBCI *hbci, int i){
  assert(hbci);
  hbci->connectTimeout=i;
}



GWEN_TYPE_UINT32 AH_HBCI_GetNextMediumId(AH_HBCI *hbci) {
  assert(hbci);
  return ++(hbci->lastMediumId);
}



GWEN_TYPE_UINT32 AH_HBCI_GetLastVersion(const AH_HBCI *hbci) {
  assert(hbci);
  return hbci->lastVersion;
}






