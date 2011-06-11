/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2011 by Martin Preuss
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

#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/ctplugin.h>

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
		     GWEN_LoggerType_Console,
		     GWEN_LoggerFacility_User);
  }

  GWEN_NEW_OBJECT(AH_HBCI, hbci);
  hbci->provider=pro;
  hbci->banking=AB_Provider_GetBanking(pro);
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

    GWEN_DB_Group_free(hbci->dbProviderConfig);

    free(hbci->productName);
    free(hbci->productVersion);

    GWEN_XMLNode_free(hbci->defs);

    GWEN_FREE_OBJECT(hbci);
    GWEN_Logger_Close(AQHBCI_LOGDOMAIN);
  }
}



int AH_HBCI_Init(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  GWEN_XMLNODE *node;

  assert(hbci);

  /* store config */
  GWEN_DB_Group_free(hbci->dbProviderConfig);
  hbci->dbProviderConfig=GWEN_DB_Group_dup(db);

  /* load and update config data */
  hbci->lastVersion=GWEN_DB_GetIntValue(db, "lastVersion", 0, 0);

  GWEN_PathManager_DefinePath(AH_PM_LIBNAME, AH_PM_XMLDATADIR);
#if defined(OS_WIN32) || defined(ENABLE_LOCAL_INSTALL)
  GWEN_PathManager_AddPathFromWinReg(AH_PM_LIBNAME,
				     AH_PM_LIBNAME,
				     AH_PM_XMLDATADIR,
				     AH_REGKEY_PATHS,
				     AH_REGKEY_XMLDATADIR);
  GWEN_PathManager_AddRelPath(AH_PM_LIBNAME,
			      AH_PM_LIBNAME,
			      AH_PM_XMLDATADIR,
			      AH_XMLDATADIR,
			      GWEN_PathManager_RelModeExe);
#else
  GWEN_PathManager_AddPath(AH_PM_LIBNAME,
			   AH_PM_LIBNAME,
			   AH_PM_XMLDATADIR,
			   AH_XMLDATADIR);
#endif

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
  return 0;
}



int AH_HBCI_Fini(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  uint32_t currentVersion;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Deinitializing AH_HBCI");
  assert(hbci);

  currentVersion=
    (AQHBCI_VERSION_MAJOR<<24) |
    (AQHBCI_VERSION_MINOR<<16) |
    (AQHBCI_VERSION_PATCHLEVEL<<8) |
    AQHBCI_VERSION_BUILD;

  /* save configuration */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Setting version %08x",
             currentVersion);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "lastVersion", currentVersion);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "transferTimeout",
                      hbci->transferTimeout);

  GWEN_DB_SetIntValue(db,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "connectTimeout",
                      hbci->connectTimeout);

  GWEN_PathManager_UndefinePath(AH_PM_LIBNAME, AH_PM_XMLDATADIR);

  GWEN_DB_Group_free(hbci->sharedRuntimeData);
  hbci->sharedRuntimeData=0;

  GWEN_XMLNode_free(hbci->defs);
  hbci->defs=0;

  return 0;
}



AB_BANKING *AH_HBCI_GetBankingApi(const AH_HBCI *hbci){
  assert(hbci);
  return hbci->banking;
}



GWEN_DB_NODE *AH_HBCI_GetProviderDb(const AH_HBCI *hbci) {
  assert(hbci);
  return hbci->dbProviderConfig;
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
  GWEN_STRINGLIST *paths;

  paths=GWEN_PathManager_GetPaths(AH_PM_LIBNAME, AH_PM_XMLDATADIR);
  if (paths) {
    GWEN_BUFFER *fbuf;
    int rv;

    fbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Directory_FindFileInPaths(paths,
				      "hbci.xml",
				      fbuf);
    GWEN_StringList_free(paths);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "XML data file not found (%d)", rv);
      GWEN_Buffer_free(fbuf);
      return NULL;
    }
    else {
      GWEN_XMLNODE *xmlNode;

      xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");

      rv=GWEN_XML_ReadFile(xmlNode,
			   GWEN_Buffer_GetStart(fbuf),
			   GWEN_XML_FLAGS_DEFAULT |
			   GWEN_XML_FLAGS_HANDLE_HEADERS);
      if (rv) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not load XML file [%s]: %d.\n",
		  GWEN_Buffer_GetStart(fbuf), rv);
	GWEN_XMLNode_free(xmlNode);
	GWEN_Buffer_free(fbuf);
	return NULL;
      }
      GWEN_Buffer_free(fbuf);

      return xmlNode;
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No data files found.");
    return NULL;
  }
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
  char buffer[16];
  GWEN_TIME *ti;

  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, "YYYYMMDD-hhmmss", nbuf);
  GWEN_Time_free(ti);

  snprintf(buffer, sizeof(buffer), "%03d", ++(hbci->counter));
  GWEN_Buffer_AppendString(nbuf, "-");
  GWEN_Buffer_AppendString(nbuf, buffer);
}




int AH_HBCI_SaveSettings(const char *path, GWEN_DB_NODE *db){
  /* check for existence of that file */
  if (GWEN_Directory_GetPath(path,
                             GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Path \"%s\" is not available", path);
    return -1;
  }

  /* write file */
  if (GWEN_DB_WriteFile(db, path, GWEN_DB_FLAGS_DEFAULT)) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Could not write file \"%s\"", path);
    return -1;
  }

  return 0;
}



GWEN_DB_NODE *AH_HBCI_LoadSettings(const char *path) {
  GWEN_DB_NODE *db;

  /* check for existence of the file */
  if (GWEN_Directory_GetPath(path,
                             GWEN_PATH_FLAGS_PATHMUSTEXIST |
                             GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Path \"%s\" does not exist", path);
    return 0;
  }

  /* file exists, load it */
  db=GWEN_DB_Group_new("cfg");
  if (GWEN_DB_ReadFile(db,
                       path,
		       GWEN_DB_FLAGS_DEFAULT |
		       GWEN_PATH_FLAGS_CREATE_GROUP)) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Could not read file \"%s\"",
	     path);
    GWEN_DB_Group_free(db);
    return 0;
  }

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
  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP);

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



uint32_t AH_HBCI_GetLastVersion(const AH_HBCI *hbci) {
  assert(hbci);
  return hbci->lastVersion;
}



