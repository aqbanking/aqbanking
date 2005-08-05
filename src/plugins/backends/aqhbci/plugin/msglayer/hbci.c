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
#include "bank_l.h"

#include <aqbanking/banking_be.h>

#include <aqhbci/connection.h>
#include <aqhbci/msgengine.h>
#include <aqhbci/connectionhbci.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/nettransportsock.h>
#include <gwenhywfar/net.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/nettransportssl.h>
#include <gwenhywfar/netconnectionhttp.h>
#include <gwenhywfar/plugin.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>


GWEN_INHERIT_FUNCTIONS(AH_HBCI);



AH_CRYPT_MODE AH_CryptMode_fromString(const char *s) {
  if (strcasecmp(s, "none")==0)
    return AH_CryptMode_None;
  else if (strcasecmp(s, "ddv")==0)
    return AH_CryptMode_Ddv;
  else if (strcasecmp(s, "pintan")==0)
    return AH_CryptMode_Pintan;
  else if (strcasecmp(s, "rdh")==0)
    return AH_CryptMode_Rdh;
  else
    return AH_CryptMode_Unknown;
}



const char *AH_CryptMode_toString(AH_CRYPT_MODE v) {
  switch(v) {
  case AH_CryptMode_None:   return "none";
  case AH_CryptMode_Ddv:    return "ddv";
  case AH_CryptMode_Pintan: return "pintan";
  case AH_CryptMode_Rdh:    return "rdh";
  default:                  return "unknown";
  }
}



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
  GWEN_INHERIT_INIT(AH_HBCI, hbci);
  hbci->provider=pro;
  hbci->banking=AB_Provider_GetBanking(pro);
  hbci->activeMedia=AH_Medium_List_new();
  hbci->banks=AH_Bank_List_new();
  hbci->dialogs=AH_Dialog_List_new();
  hbci->libId=GWEN_Net_GetLibraryId();
  hbci->productName=strdup("AQHBCI");
  rv=snprintf(numbuf, sizeof(numbuf), "%d.%d",
              AQHBCI_VERSION_MAJOR, AQHBCI_VERSION_MINOR);
  if (rv==-1 || rv>=sizeof(numbuf)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "What ?? %d bytes isn't enough space for two decimals ?!",
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
    GWEN_INHERIT_FINI(AH_HBCI, hbci);

    /* close all connections */
    AH_HBCI_CloseAllConnections(hbci);

    /* free dialogs */
    AH_Dialog_List_free(hbci->dialogs);

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

    /* free active banks */
    AH_Bank_List_free(hbci->banks);

    /* free active media */
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "%d active media",
             AH_Medium_List_GetCount(hbci->activeMedia));
    AH_Medium_List_free(hbci->activeMedia);

    free(hbci->productName);
    free(hbci->productVersion);

    GWEN_XMLNode_free(hbci->defs);

    free(hbci);
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
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Loaded medium \"%s\"", name);
      AH_HBCI_AddMedium(hbci, m);
  
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
  GWEN_DB_NODE *gr;
  GWEN_DB_NODE *db;
  GWEN_TYPE_UINT32 oldVersion;
  GWEN_TYPE_UINT32 currentVersion;
  GWEN_PLUGIN_MANAGER *pm;

  assert(hbci);

  /* create xml file plugin manager -- well, there are no "plugins",
     but since it includes the registry path management, we simply
     abuse this here for registry lookup. */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Registering xmldata plugin manager");
  pm=GWEN_PluginManager_new("xmldata");
  GWEN_PluginManager_AddPathFromWinReg(pm,
				       "Software\\Aqhbci\\Paths",
				       "xmldatadir");
  GWEN_PluginManager_AddPath(pm,
			     AQHBCI_DATA AH_PATH_SEP "xml");
  if (GWEN_PluginManager_Register(pm)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Could not register xmldata plugin manager");
    return AB_ERROR_GENERIC;
  }

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

  /* load config data */
  db=AB_Provider_GetData(hbci->provider);

  hbci->transferTimeout=GWEN_DB_GetIntValue(db, "transferTimeout", 0,
					    AH_HBCI_DEFAULT_TRANSFER_TIMEOUT);
  hbci->connectTimeout=GWEN_DB_GetIntValue(db, "connectTimeout", 0,
					   AH_HBCI_DEFAULT_CONNECT_TIMEOUT);

  oldVersion=GWEN_DB_GetIntValue(db, "lastVersion", 0,
                                 AH_HBCI_LAST_VERSION_NONE);
  currentVersion=
    (AQHBCI_VERSION_MAJOR<<24) |
    (AQHBCI_VERSION_MINOR<<16) |
    (AQHBCI_VERSION_PATCHLEVEL<<8) |
    AQHBCI_VERSION_BUILD;

  if (oldVersion!=AH_HBCI_LAST_VERSION_NONE &&
      oldVersion<((1<<24) | (2<<16) | (0<<8) | 3)) {
    GWEN_DB_NODE *dbMedia;
    GWEN_DB_NODE *dbBanks;

    DBG_WARN(AQHBCI_LOGDOMAIN,
             "Updating from version prior to 1.2.0.3");

    dbMedia=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "media");
    assert(dbMedia);

    dbBanks=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "banks");
    if (dbBanks) {
      GWEN_DB_NODE *dbBank;

      dbBank=GWEN_DB_FindFirstGroup(dbBanks, "bank");
      while(dbBank) {
        GWEN_DB_NODE *dbUsers;

        dbUsers=GWEN_DB_GetGroup(dbBank,
                                 GWEN_PATH_FLAGS_NAMEMUSTEXIST, "users");
        if (dbUsers) {
          GWEN_DB_NODE *dbUser;

          dbUser=GWEN_DB_FindFirstGroup(dbUsers, "user");
          while(dbUser) {
            GWEN_DB_NODE *dbMedium;

            dbMedium=GWEN_DB_GetGroup(dbUser,
                                      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                      "medium");
            if (dbMedium) {
              GWEN_DB_NODE *dbDst;
              const char *s;
              int i;

              dbDst=GWEN_DB_GetGroup(dbMedia, GWEN_PATH_FLAGS_CREATE_GROUP,
				     "medium");
	      assert(dbDst);
	      s=GWEN_DB_GetCharValue(dbMedium, "mediumTypeName", 0, 0);
	      if (s)
		GWEN_DB_SetCharValue(dbDst, GWEN_DB_FLAGS_OVERWRITE_VARS,
				     "mediumTypeName", s);
	      s=GWEN_DB_GetCharValue(dbMedium, "mediumSubTypeName", 0, 0);
              if (s)
                GWEN_DB_SetCharValue(dbDst, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                     "mediumSubTypeName", s);
              s=GWEN_DB_GetCharValue(dbMedium, "mediumName", 0, 0);
              if (s)
                GWEN_DB_SetCharValue(dbDst, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                     "mediumName", s);
              s=GWEN_DB_GetCharValue(dbMedium, "descriptiveName", 0, 0);
              if (s)
                GWEN_DB_SetCharValue(dbDst, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                     "descriptiveName", s);
              i=GWEN_DB_GetIntValue(dbMedium, "flags", 0, 0);
              if (i)
                GWEN_DB_SetIntValue(dbDst, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                    "flags", i);
              s=GWEN_DB_GetCharValue(dbMedium, "mediumType", 0, 0);
              if (s)
                GWEN_DB_SetCharValue(dbUser, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                     "cryptMode", s);
	    } /* if medium */

            dbUser=GWEN_DB_FindNextGroup(dbUser, "user");
	  } /* if user */
	} /* if users */
        dbBank=GWEN_DB_FindNextGroup(dbBank, "bank");
      } /* if bank */
    } /* if banks */
  } /* if update from before 1.2.0.3 */

  /* load media */
  AH_HBCI__LoadMedia(hbci, db);

  /* load banks */
  gr=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "banks");
  if (gr) {
    gr=GWEN_DB_FindFirstGroup(gr, "bank");
    while(gr) {
      AH_BANK *b;

      b=AH_Bank_fromDb(hbci, gr);
      if (!b) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "Error loading bank");
	return AB_ERROR_BAD_CONFIG_FILE;
      }
      AH_Bank_List_Add(b, hbci->banks);
      gr=GWEN_DB_FindNextGroup(gr, "bank");
    } /* while */
  } /* if "banks" */

  if (oldVersion!=AH_HBCI_LAST_VERSION_NONE &&
      currentVersion>oldVersion) {
    AH_BANK *b;

    if (oldVersion<
        ((1<<24) |
         (0<<16) |
         (3<<8) |
         9)) {
      AB_Banking_MessageBox(hbci->banking,
                            AB_BANKING_MSG_FLAGS_TYPE_INFO |
                            AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
                            AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                            I18N("AqHBCI-Notice"),
                            I18N(
      "Since version 1.0.3.9 AqHBCI no longer\n"
      "stores the PIN in your private logfiles when\n"
      "logging PIN/TAN connections.\n"
      "Because previous versions did, you should\n"
      "delete all logfiles in your local "
      "AqBanking\n"
      "Logfolder (usually something like\n"
      "$HOME/.banking/backends/aqhbci/data/...)\n"
      "\n"
      "Logfiles have the extension \".log\", \n"
      "please do only delete those files!"
      "\n"
       "This only affects PIN/TAN mode, all other modes\n"
       "are not affected.\n"
       ""
       "<html>"
      "<p>"
      "Since version 1.0.3.9 AqHBCI no longer\n"
      "stores the PIN in your private logfiles when\n"
      "logging <b>PIN/TAN</b> connections.\n"
      "</p>"
       "<p>"
       "Because previous versions did, you should\n"
       "delete all logfiles in your local \n"
       "AqBanking Logfolder (usually something like\n"
       "<i>"
       "$HOME/.banking/backends/aqhbci/data/...\n"
       "</i>)"
       "</p>"
       "<p>"
       "Logfiles have the extension \".log\", \n"
       "<font color=red>please do only delete <b>those</b> files!</font>"
       "</p>"
       "<p>"
       "This only affects <b>PIN/TAN mode,</b> all other modes\n"
       "are not affected.\n"
       "</p>"
       "</html>"
                                ),
                            I18N("Continue"), 0, 0);
    }

    /* this is an update, set all UPD/BPD-Versions to zero */
    DBG_NOTICE(AQHBCI_LOGDOMAIN,
               "Updating from %d.%d.%d.%d",
               (oldVersion>>24) & 0xff,
               (oldVersion>>16) & 0xff,
               (oldVersion>>8) & 0xff,
               oldVersion & 0xff);
    b=AH_Bank_List_First(hbci->banks);
    while(b) {
      AH_USER_LIST2 *ul;

      ul=AH_Bank_GetUsers(b, "*");
      if (ul) {
        AH_USER_LIST2_ITERATOR *it;

        it=AH_User_List2_First(ul);
        if (it) {
          AH_USER *u;

          u=AH_User_List2Iterator_Data(it);
          while(u) {
            AH_CUSTOMER_LIST2 *cl;

            cl=AH_User_GetCustomers(u, "*");
            if (cl) {
              AH_CUSTOMER_LIST2_ITERATOR *cit;

              cit=AH_Customer_List2_First(cl);
              if (cit) {
                AH_CUSTOMER *cu;

                cu=AH_Customer_List2Iterator_Data(cit);
                while(cu) {
                  AH_Customer_SetUpdVersion(cu, 0);
                  AH_Customer_SetBpdVersion(cu, 0);
                  cu=AH_Customer_List2Iterator_Next(cit);
                }
                AH_Customer_List2Iterator_free(cit);
              }
              AH_Customer_List2_free(cl);
            }

            u=AH_User_List2Iterator_Next(it);
          }
          AH_User_List2Iterator_free(it);
        }
        AH_User_List2_free(ul);
      }
      b=AH_Bank_List_Next(b);
    }
  }



  return 0;
}



int AH_HBCI_Fini(AH_HBCI *hbci) {
  AH_BANK *b;
  GWEN_DB_NODE *db;
  GWEN_TYPE_UINT32 currentVersion;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Deinitializing AH_HBCI");
  assert(hbci);

  currentVersion=
    (AQHBCI_VERSION_MAJOR<<24) |
    (AQHBCI_VERSION_MINOR<<16) |
    (AQHBCI_VERSION_PATCHLEVEL<<8) |
    AQHBCI_VERSION_BUILD;

  /* close all connections */
  AH_HBCI_CloseAllConnections(hbci);

  /* clear dialogs */
  AH_Dialog_List_Clear(hbci->dialogs);

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

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "transferTimeout",
                      hbci->transferTimeout);

  GWEN_DB_SetIntValue(db,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "connectTimeout",
                      hbci->connectTimeout);

  b=AH_Bank_List_First(hbci->banks);
  if (b) {
    GWEN_DB_NODE *gr;

    gr=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "banks");
    assert(gr);
    while(b) {
      GWEN_DB_NODE *dbB;
      int rv;

      dbB=GWEN_DB_GetGroup(gr, GWEN_PATH_FLAGS_CREATE_GROUP, "bank");
      assert(dbB);
      rv=AH_Bank_toDb(b, dbB);
      if (rv) {
        return rv;
      }
      b=AH_Bank_List_Next(b);
    } /* while */
  }

  /* clear active banks */
  AH_Bank_List_Clear(hbci->banks);

  /* save all active media */
  AH_HBCI__SaveMedia(hbci, db);

  /* clear active media */
  DBG_ERROR(AQHBCI_LOGDOMAIN, "%d active media",
            AH_Medium_List_GetCount(hbci->activeMedia));
  AH_Medium_List_Clear(hbci->activeMedia);

  {
    GWEN_PLUGIN_MANAGER *pm;

    /* clear xmldata pluginmanager */
    pm=GWEN_PluginManager_FindPluginManager("xmldata");
    if (pm) {
      if (GWEN_PluginManager_Unregister(pm)) {
	DBG_ERROR(AQHBCI_LOGDOMAIN,
		  "Could not unregister xmldata plugin manager");
      }
      GWEN_PluginManager_free(pm);
    }
  }

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
  GWEN_PLUGIN_MANAGER *pm;
  const GWEN_STRINGLIST *slist;

  xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");

  /* New loading code -- use path list from PluginManager but don't
     use its loading code */
  pm=GWEN_PluginManager_FindPluginManager("xmldata");
  if (!pm) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Could not find plugin manager for \"%s\"",
              "xmldata");
    return 0;
  }
  slist = GWEN_PluginManager_GetPaths(pm);
  if (GWEN_XML_ReadFileSearch(xmlNode,
			      "hbci.xml",
			      GWEN_XML_FLAGS_DEFAULT |
			      GWEN_XML_FLAGS_HANDLE_HEADERS,
			      (GWEN_STRINGLIST*)slist) ) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not load XML file.\n");
    GWEN_XMLNode_free(xmlNode);
    return 0;
  }

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



GWEN_TYPE_UINT32 AH_HBCI_GetLibraryMark(const AH_HBCI *hbci){
  assert(hbci);
  return hbci->libId;
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




AH_CUSTOMER *AH_HBCI_FindCustomer(AH_HBCI *hbci,
                                  int country,
                                  const char *bankId,
                                  const char *userId,
				  const char *customerId) {
  AH_BANK *b;

  assert(hbci);
  b=AH_Bank_List_First(hbci->banks);
  while(b) {
    if (-1!=GWEN_Text_ComparePattern(AH_Bank_GetBankId(b), bankId, 0)) {
      AH_CUSTOMER *cu;

      cu=AH_Bank_FindCustomer(b, userId, customerId);
      if (cu)
        return cu;
    }

    b=AH_Bank_List_Next(b);
  } /* while */

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Customer not found");
  return 0;
}




AH_CUSTOMER_LIST2 *AH_HBCI_GetCustomers(AH_HBCI *hbci,
                                        int country,
                                        const char *bankId,
                                        const char *userId,
                                        const char *customerId){
  AH_BANK *b;
  AH_CUSTOMER_LIST2 *cl;

  assert(hbci);
  cl=AH_Customer_List2_new();
  b=AH_Bank_List_First(hbci->banks);
  while(b) {
    if (-1!=GWEN_Text_ComparePattern(AH_Bank_GetBankId(b), bankId, 0)) {
      AH_CUSTOMER_LIST2 *tcl;

      tcl=AH_Bank_GetCustomers(b, userId, customerId);
      if (tcl) {
	AH_CUSTOMER_LIST2_ITERATOR *it;
        AH_CUSTOMER *cu;

	it=AH_Customer_List2_First(tcl);
	assert(it);
	cu=AH_Customer_List2Iterator_Data(it);
	while(cu) {
          AH_Customer_List2_PushBack(cl, cu);
	  cu=AH_Customer_List2Iterator_Next(it);
	}
        AH_Customer_List2Iterator_free(it);
	AH_Customer_List2_free(tcl);
      }
    }

    b=AH_Bank_List_Next(b);
  } /* while */

  if (AH_Customer_List2_GetSize(cl)==0) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No customers found");
    AH_Customer_List2_free(cl);
    return 0;
  }

  return cl;
}




AH_ACCOUNT *AH_HBCI_FindAccount(AH_HBCI *hbci,
                                int country,
                                const char *bankId,
                                const char *accountId){
  AH_BANK *b;

  assert(hbci);
  b=AH_Bank_List_First(hbci->banks);
  while(b) {
    if (-1!=GWEN_Text_ComparePattern(AH_Bank_GetBankId(b), bankId, 0)) {
      AH_ACCOUNT *a;

      a=AH_Bank_FindAccount(b, accountId);
      if (a)
        return a;
    }

    b=AH_Bank_List_Next(b);
  } /* while */

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Account not found");
  return 0;
}



AH_ACCOUNT_LIST2 *AH_HBCI_GetAccounts(AH_HBCI *hbci,
                                      int country,
                                      const char *bankId,
                                      const char *accountId){
  AH_BANK *b;
  AH_ACCOUNT_LIST2 *al;

  assert(hbci);
  al=AH_Account_List2_new();
  b=AH_Bank_List_First(hbci->banks);
  while(b) {
    if (-1!=GWEN_Text_ComparePattern(AH_Bank_GetBankId(b), bankId, 0)) {
      AH_ACCOUNT_LIST2 *tal;

      tal=AH_Bank_GetAccounts(b, accountId);
      if (tal) {
	AH_ACCOUNT_LIST2_ITERATOR *it;
        AH_ACCOUNT *a;

	it=AH_Account_List2_First(tal);
	assert(it);
	a=AH_Account_List2Iterator_Data(it);
	while(a) {
          AH_Account_List2_PushBack(al, a);
	  a=AH_Account_List2Iterator_Next(it);
	}
        AH_Account_List2Iterator_free(it);
	AH_Account_List2_free(tal);
      }
    }

    b=AH_Bank_List_Next(b);
  } /* while */

  if (AH_Account_List2_GetSize(al)==0) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No account found");
    AH_Account_List2_free(al);
    return 0;
  }

  return al;
}



AH_USER *AH_HBCI_FindUser(AH_HBCI *hbci,
                          int country,
                          const char *bankId,
                          const char *userId){
  AH_BANK *b;

  assert(hbci);
  b=AH_Bank_List_First(hbci->banks);
  while(b) {
    if (-1!=GWEN_Text_ComparePattern(AH_Bank_GetBankId(b), bankId, 0)) {
      AH_USER *u;

      u=AH_Bank_FindUser(b, userId);
      if (u)
        return u;
    }

    b=AH_Bank_List_Next(b);
  } /* while */

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "User not found");
  return 0;
}



AH_USER_LIST2 *AH_HBCI_GetUsers(AH_HBCI *hbci,
                                int country,
                                const char *bankId,
                                const char *userId){
  AH_BANK *b;
  AH_USER_LIST2 *ul;

  assert(hbci);
  ul=AH_User_List2_new();
  b=AH_Bank_List_First(hbci->banks);
  while(b) {
    if (-1!=GWEN_Text_ComparePattern(AH_Bank_GetBankId(b), bankId, 0)) {
      AH_USER_LIST2 *tul;

      tul=AH_Bank_GetUsers(b, userId);
      if (tul) {
	AH_USER_LIST2_ITERATOR *it;
        AH_USER *u;

	it=AH_User_List2_First(tul);
	assert(it);
	u=AH_User_List2Iterator_Data(it);
	while(u) {
          AH_User_List2_PushBack(ul, u);
	  u=AH_User_List2Iterator_Next(it);
	}
        AH_User_List2Iterator_free(it);
	AH_User_List2_free(tul);
      }
    }

    b=AH_Bank_List_Next(b);
  } /* while */

  if (AH_User_List2_GetSize(ul)==0) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No user found");
    AH_User_List2_free(ul);
    return 0;
  }

  return ul;
}



AH_BANK *AH_HBCI_FindBank(const AH_HBCI *hbci,
                          int country,
                          const char *bankId){
  AH_BANK *b;

  assert(hbci);
  assert(bankId);

  b=AH_Bank_List_First(hbci->banks);

  while(b) {
    if ((!country || AH_Bank_GetCountry(b)==country) &&
        -1!=GWEN_Text_ComparePattern(AH_Bank_GetBankId(b), bankId, 0))
      return b;
    b=AH_Bank_List_Next(b);
  } /* while */

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Bank \"%d:%s\" not found", country, bankId);
  return 0;
}



AH_BANK_LIST2 *AH_HBCI_GetBanks(const AH_HBCI *hbci,
                                int country,
                                const char *bankId){
  AH_BANK *b;
  AH_BANK_LIST2 *bl;

  assert(hbci);
  assert(bankId);

  bl=AH_Bank_List2_new();
  b=AH_Bank_List_First(hbci->banks);

  while(b) {
    if ((!country || AH_Bank_GetCountry(b)==country) &&
        -1!=GWEN_Text_ComparePattern(AH_Bank_GetBankId(b), bankId, 0))
      AH_Bank_List2_PushBack(bl, b);
    b=AH_Bank_List_Next(b);
  } /* while */

  if (AH_Bank_List2_GetSize(bl)==0) {
    AH_Bank_List2_free(bl);
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Bank \"%d:%s\" not found", country, bankId);
    return 0;
  }

  return bl;
}




int AH_HBCI_AddBank(AH_HBCI *hbci, AH_BANK *b){
  if (AH_HBCI_FindBank(hbci,
                       AH_Bank_GetCountry(b),
                       AH_Bank_GetBankId(b))) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bank \"%d:%s\" already enlisted",
              AH_Bank_GetCountry(b),
              AH_Bank_GetBankId(b));
    return -1;
  }
  AH_Bank_List_Add(b, hbci->banks);
  return 0;
}



int AH_HBCI_RemoveBank(AH_HBCI *hbci, AH_BANK *b) {
  assert(hbci);
  assert(b);
  AH_Bank_List_Del(b);
  return 0;
}





AH_MEDIUM *AH_HBCI_GetMedium(AH_HBCI *hbci, AH_CUSTOMER *cu){
  AH_MEDIUM *m;
  AH_BANK *b;
  AH_USER *u;

  assert(hbci);
  assert(cu);
  u=AH_Customer_GetUser(cu);
  assert(u);
  b=AH_User_GetBank(u);
  assert(b);

  if (hbci->currentMedium==AH_User_GetMedium(u)) {
    /* return current medium if it is the wanted one, release it otherwise */
    if (AH_Medium_SelectContext(hbci->currentMedium,
                                AH_User_GetContextIdx(u))==0) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Returning current medium");
      return hbci->currentMedium;
    }
    if (AH_Medium_IsMounted(hbci->currentMedium)) {
      if (AH_Medium_Unmount(hbci->currentMedium, 0)) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Could not unmount medium");
        return 0;
      }
    }
    AH_Medium_free(hbci->currentMedium);
    hbci->currentMedium=0;
  }

  m=AH_User_GetMedium(u);
  assert(m);

  if (!AH_Medium_IsMounted(m)) {
    if (AH_Medium_Mount(m)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not mount medium");
      return 0;
    }
  }

  /* store current medium */
  hbci->currentMedium=m;
  AH_Medium_Attach(m);

  if (AH_Medium_SelectContext(hbci->currentMedium,
                              AH_User_GetContextIdx(u))){
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Error selecting context %d for \"%d:%s/%s\"",
              AH_User_GetContextIdx(u),
              AH_Bank_GetCountry(b),
              AH_Bank_GetBankId(b),
              AH_User_GetUserId(u));
    return 0;
  }

  return m;
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



int AH_HBCI_AddMedium(AH_HBCI *hbci, AH_MEDIUM *m) {
  assert(hbci);
  assert(m);

  if (AH_HBCI_FindMedium(hbci,
                         AH_Medium_GetMediumTypeName(m),
                         AH_Medium_GetMediumName(m))) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Medium already listed");
    return -1;
  }

  AH_Medium_List_Add(m, hbci->activeMedia);
  return 0;
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




















void AH_HBCI_AddConnection(AH_HBCI *hbci,
                           GWEN_NETCONNECTION *conn){
  assert(hbci);
  assert(conn);
  GWEN_Net_AddConnectionToPool(conn);
}



void AH_HBCI_AddDialog(AH_HBCI *hbci,
                       AH_DIALOG *dlg){
  assert(hbci);
  assert(dlg);
  AH_Dialog_List_Add(dlg, hbci->dialogs);
}



AH_DIALOG *AH_HBCI_FindDialog(const AH_HBCI *hbci,
                              const AH_CUSTOMER *cu){
  AH_DIALOG *dlg;

  assert(hbci);
  assert(cu);
  dlg=AH_Dialog_List_First(hbci->dialogs);
  while(dlg) {
    if (AH_Dialog_GetDialogOwner(dlg)==cu) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Dialog for customer \"%s\" found",
                AH_Customer_GetCustomerId(cu));
      return dlg;
    }
    dlg=AH_Dialog_List_Next(dlg);
  } /* while */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "No dialog found for customer \"%s\"",
            AH_Customer_GetCustomerId(cu));
  return 0;
}



AH_DIALOG*
AH_HBCI_FindDialogByConnection(const AH_HBCI *hbci,
                               const GWEN_NETCONNECTION *conn){
  AH_DIALOG *dlg;

  assert(hbci);
  assert(conn);
  dlg=AH_Dialog_List_First(hbci->dialogs);
  while(dlg) {
    if (AH_Dialog_GetConnection(dlg)==conn) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Dialog for connection found");
      return dlg;
    }
    dlg=AH_Dialog_List_Next(dlg);
  } /* while */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "No dialog found for given connection");
  return 0;
}





void AH_HBCI_EndDialog(AH_HBCI *hbci, AH_DIALOG *dlg){
  GWEN_NETCONNECTION *conn;
  AH_CUSTOMER *cu;

  assert(hbci);
  assert(dlg);

  cu=AH_Dialog_GetDialogOwner(dlg);
  if (cu) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Disconnecting connection for customer \"%s\"",
             AH_Customer_GetCustomerId(cu));
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Disconnecting");
  }
  conn=AH_Dialog_GetConnection(dlg);
  assert(conn);
  if (GWEN_NetConnection_StartDisconnect(conn)) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not start to disconnect connection for dialog");
  }
  else {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Successfully started to disconnect");
  }
  /* dialog is no longer open (if it ever was) */
  AH_Dialog_SubFlags(dlg, AH_DIALOG_FLAGS_OPEN);

  /* destroy dialog */
  AH_Dialog_free(dlg);
}



void AH_HBCI_CheckConnections(AH_HBCI *hbci) {
  GWEN_NETCONNECTION *conn;

  assert(hbci);
  conn=GWEN_NetConnection_List_First(GWEN_Net_GetConnectionPool());
  while(conn) {
    GWEN_NETCONNECTION *next;
    AH_DIALOG *dlg;

    next=GWEN_NetConnection_List_Next(conn);
    if (GWEN_NetConnection_GetLibraryMark(conn)==hbci->libId) {
      if (AH_Connection_IsDown(conn)) {
        /* connection is down, find matching dialog */
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found a connection which is down");
        dlg=AH_HBCI_FindDialogByConnection(hbci, conn);
        if (dlg) {
          /* found a dialog, unlink and free it */
          DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found dialog for this connection, freeing it");
          if (AH_Dialog_GetFlags(dlg) & AH_DIALOG_FLAGS_OPEN) {
            AH_Dialog_SubFlags(dlg, AH_DIALOG_FLAGS_OPEN);
            AH_HBCI_EmitDialogDown(hbci, dlg);
          }
          AH_Dialog_List_Del(dlg);
          AH_Dialog_free(dlg);
        }
        else {
          DBG_DEBUG(AQHBCI_LOGDOMAIN,
                   "Did not find a dialog for this connection");
        }
        /* unlink and free connection */
        GWEN_NetConnection_List_Del(conn);
        GWEN_NetConnection_free(conn);
      } /* if connection is down */
    } /* if connection belongs to us */
    conn=next;
  } /* while */
}



void AH_HBCI_CloseAllConnections(AH_HBCI *hbci) {
  GWEN_NETCONNECTION *conn;

  assert(hbci);
  conn=GWEN_NetConnection_List_First(GWEN_Net_GetConnectionPool());
  while(conn) {
    GWEN_NETCONNECTION *next;

    next=GWEN_NetConnection_List_Next(conn);
    if (GWEN_NetConnection_GetLibraryMark(conn)==hbci->libId) {
      if (!AH_Connection_IsDown(conn)) {
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Shutting down connection (waiting)");
        GWEN_NetConnection_Disconnect_Wait(conn, 5);
      } /* if connection is not down */
      /* unlink and free connection */
      GWEN_NetConnection_List_Del(conn);
      GWEN_NetConnection_free(conn);
    } /* if connection belongs to us */
    conn=next;
  } /* while */
}





void AH_HBCI_EmitDialogUp(AH_HBCI *hbci, AH_DIALOG *dlg){
  assert(hbci);
  assert(dlg);

  if (hbci->dialogUpFn)
    hbci->dialogUpFn(hbci, dlg);
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No dialog up function set");
  }
}



void AH_HBCI_EmitDialogDown(AH_HBCI *hbci, AH_DIALOG *dlg){
  assert(hbci);
  assert(dlg);

  if (hbci->dialogDownFn)
    hbci->dialogDownFn(hbci, dlg);
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No dialog down function set");
  }
}



void AH_HBCI_SetDialogUpFn(AH_HBCI *hbci, AH_HBCI_DIALOGUPFN fn){
  assert(hbci);
  hbci->dialogUpFn=fn;
}



void AH_HBCI_SetDialogDownFn(AH_HBCI *hbci, AH_HBCI_DIALOGUPFN fn){
  assert(hbci);
  hbci->dialogDownFn=fn;
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



int AH_HBCI_BeginDialog(AH_HBCI *hbci,
			AH_CUSTOMER *cu,
			AH_DIALOG **pdlg) {
  GWEN_NETTRANSPORT *tr;
  GWEN_NETCONNECTION *conn;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  AH_BANK *b;
  AH_USER *u;
  const char *bankAddr;
  int bankPort;
  AH_BPD_ADDR_TYPE addrType;
  const AH_BPD_ADDR *bpdAddr;
  AH_DIALOG *dlg;
  GWEN_ERRORCODE err;
  const char *p;
  int rv;

  assert(pdlg);
  u=AH_Customer_GetUser(cu);
  assert(u);
  b=AH_User_GetBank(u);
  assert(b);

  /* take bank addr from user */
  bpdAddr=AH_User_GetAddress(u);
  if (!bpdAddr) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User has no address settings");
    return AB_ERROR_INVALID;
  }
  addrType=AH_BpdAddr_GetType(bpdAddr);
  bankAddr=AH_BpdAddr_GetAddr(bpdAddr);
  if (!bankAddr) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User has no valid address settings");
    return AB_ERROR_INVALID;
  }
  bankPort=-1;
  p=AH_BpdAddr_GetSuffix(bpdAddr);
  if (p) {
    if (1!=sscanf(p, "%i", &bankPort)) {
      DBG_WARN(AQHBCI_LOGDOMAIN, "User has bad port settings");
      bankPort=-1;
    }
  }
  if (bankPort==-1) {
    switch(addrType) {

    case AH_BPD_AddrTypeTCP:
      bankPort=3000;
      break;

    case AH_BPD_AddrTypeBTX:
      DBG_WARN(AQHBCI_LOGDOMAIN,
               "Unsupported address type (%d), assuming TCP",
               addrType);
      bankPort=3000;
      break;

    case AH_BPD_AddrTypeSSL:
      bankPort=443;
      break;

    default:
      DBG_WARN(AQHBCI_LOGDOMAIN,
               "Unknown address type (%d), assuming TCP",
               addrType);
      bankPort=3000;
      break;
    } /* switch */
  }

  /* create connection */
  switch(addrType) {

  case AH_BPD_AddrTypeTCP:
    sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
    tr=GWEN_NetTransportSocket_new(sk, 1);
    addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
    err=GWEN_InetAddr_SetAddress(addr, bankAddr);
    if (!GWEN_Error_IsOk(err)) {
      char dbgbuf[256];

      snprintf(dbgbuf, sizeof(dbgbuf)-1,
               I18N("Resolving hostname \"%s\" ..."),
               bankAddr);
      dbgbuf[sizeof(dbgbuf)-1]=0;
      AB_Banking_ProgressLog(hbci->banking, 0,
                             AB_Banking_LogLevelNotice,
                             dbgbuf);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Resolving hostname \"%s\"",
               bankAddr);
      err=GWEN_InetAddr_SetName(addr, bankAddr);
      if (!GWEN_Error_IsOk(err)) {
        snprintf(dbgbuf, sizeof(dbgbuf)-1,
                 I18N("Unknown hostname \"%s\""),
                 bankAddr);
        dbgbuf[sizeof(dbgbuf)-1]=0;
        AB_Banking_ProgressLog(hbci->banking, 0,
                               AB_Banking_LogLevelError,
                               dbgbuf);
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Error resolving hostname \"%s\":",
                  bankAddr);
        DBG_ERROR_ERR(AQHBCI_LOGDOMAIN, err);
	return AB_ERROR_NETWORK;
      }
      else {
        char addrBuf[256];
        GWEN_ERRORCODE err;

        err=GWEN_InetAddr_GetAddress(addr, addrBuf, sizeof(addrBuf)-1);
        addrBuf[sizeof(addrBuf)-1]=0;
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(AQHBCI_LOGDOMAIN, err);
        }
        else {
          snprintf(dbgbuf, sizeof(dbgbuf)-1,
                   I18N("IP address is %s"),
                   addrBuf);
          dbgbuf[sizeof(dbgbuf)-1]=0;
          AB_Banking_ProgressLog(hbci->banking, 0,
                                 AB_Banking_LogLevelNotice,
                                 dbgbuf);
        }
      }
    }
    GWEN_InetAddr_SetPort(addr, bankPort);
    GWEN_NetTransport_SetPeerAddr(tr, addr);
    GWEN_InetAddr_free(addr);
    conn=AH_ConnectionHBCI_new(tr, 1, AH_HBCI_GetLibraryMark(hbci));
    assert(conn);
    AH_Connection_Extend(hbci, conn);
    GWEN_NetConnection_SetUserMark(conn, AH_HBCI_CONN_MARK_TCP);
    break;

  case AH_BPD_AddrTypeSSL: {
    int country;
    const char *bankId;
    GWEN_BUFFER *nbuf;
    char *p;

    country=AH_Bank_GetCountry(b);
    bankId=AH_Bank_GetBankId(b);
    nbuf=GWEN_Buffer_new(0, 64, 0, 1);
    AH_HBCI_AddBankCertFolder(hbci, b, nbuf);

    sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
    tr=GWEN_NetTransportSSL_new(sk,
                                GWEN_Buffer_GetStart(nbuf),
                                GWEN_Buffer_GetStart(nbuf),
                                0, 0, 0, 1);
    GWEN_Buffer_Reset(nbuf);
    GWEN_Buffer_AppendString(nbuf, bankAddr);
    p=strchr(GWEN_Buffer_GetStart(nbuf), '/');
    if (p)
      *p=0;
    if (AH_Customer_GetHttpHost(cu)==0)
      /* set HTTP host if it not already is */
      AH_Customer_SetHttpHost(cu, GWEN_Buffer_GetStart(nbuf));
    addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
    err=GWEN_InetAddr_SetAddress(addr, GWEN_Buffer_GetStart(nbuf));
    if (!GWEN_Error_IsOk(err)) {
      char dbgbuf[256];

      snprintf(dbgbuf, sizeof(dbgbuf)-1,
               I18N("Resolving hostname \"%s\" ..."),
               GWEN_Buffer_GetStart(nbuf));
      dbgbuf[sizeof(dbgbuf)-1]=0;
      AB_Banking_ProgressLog(hbci->banking, 0,
                             AB_Banking_LogLevelNotice,
                             dbgbuf);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Resolving hostname \"%s\"",
               GWEN_Buffer_GetStart(nbuf));
      err=GWEN_InetAddr_SetName(addr, GWEN_Buffer_GetStart(nbuf));
      if (!GWEN_Error_IsOk(err)) {
        snprintf(dbgbuf, sizeof(dbgbuf)-1,
                 I18N("Unknown hostname \"%s\""),
                 GWEN_Buffer_GetStart(nbuf));
        dbgbuf[sizeof(dbgbuf)-1]=0;
        AB_Banking_ProgressLog(hbci->banking, 0,
                               AB_Banking_LogLevelError,
                               dbgbuf);
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Error resolving hostname \"%s\":",
                  GWEN_Buffer_GetStart(nbuf));
        DBG_ERROR_ERR(AQHBCI_LOGDOMAIN, err);
        GWEN_Buffer_free(nbuf);
        return AB_ERROR_NETWORK;
      }
      else {
        char addrBuf[256];
        GWEN_ERRORCODE err;

        err=GWEN_InetAddr_GetAddress(addr, addrBuf, sizeof(addrBuf)-1);
        addrBuf[sizeof(addrBuf)-1]=0;
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(AQHBCI_LOGDOMAIN, err);
        }
        else {
          snprintf(dbgbuf, sizeof(dbgbuf)-1,
                   I18N("IP address is %s"),
                   addrBuf);
          dbgbuf[sizeof(dbgbuf)-1]=0;
          AB_Banking_ProgressLog(hbci->banking, 0,
                                 AB_Banking_LogLevelNotice,
                                 dbgbuf);
        }
      }
    }
    GWEN_InetAddr_SetPort(addr, bankPort);
    GWEN_NetTransport_SetPeerAddr(tr, addr);
    GWEN_InetAddr_free(addr);
    conn=GWEN_NetConnectionHTTP_new(tr, 1,
                                    AH_HBCI_GetLibraryMark(hbci),
                                    AH_Customer_GetHttpVMajor(cu),
                                    AH_Customer_GetHttpVMinor(cu));
    assert(conn);
    AH_Connection_Extend(hbci, conn);
    GWEN_NetConnection_SetUserMark(conn, AH_HBCI_CONN_MARK_SSL);
    if (p)
      *p='/';
    if (p)
      if (*p)
        GWEN_NetConnectionHTTP_SetDefaultURL(conn, p);
    GWEN_Buffer_free(nbuf);
    break;
  }

  case AH_BPD_AddrTypeBTX:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Address type \"BTX\" not supported");
    return AB_ERROR_INVALID;

  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unsupported address type (%d)", addrType);
    return AB_ERROR_INVALID;
  }

  rv=GWEN_NetConnection_Connect_Wait(conn, hbci->connectTimeout);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "Could not connect to bank (%d)",
	      rv);
    GWEN_NetConnection_free(conn);
    return AB_ERROR_NETWORK;
  }

  dlg=AH_Dialog_new(cu, conn);
  AH_Dialog_AddFlags(dlg, AH_DIALOG_FLAGS_INITIATOR);

  AH_HBCI_AddConnection(hbci, conn);
  AH_HBCI_AddDialog(hbci, dlg);

  *pdlg=dlg;

  return 0;
}



int AH_HBCI_AddBankCertFolder(AH_HBCI *hbci,
                              AH_BANK *b,
                              GWEN_BUFFER *nbuf) {
  AH_HBCI_AddObjectPath(hbci,
                        AH_Bank_GetCountry(b),
                        AH_Bank_GetBankId(b),
                        0, 0, 0, nbuf);
  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP "certs");
  return 0;
}



int AH_HBCI_RemoveAllBankCerts(AH_HBCI *hbci, AH_BANK *b) {
  GWEN_DIRECTORYDATA *d;
  GWEN_BUFFER *nbuf;
  char nbuffer[64];
  unsigned int pathLen;

  assert(hbci);

  /* create path */
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AH_HBCI_AddBankCertFolder(hbci, b, nbuf);
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

  /* file exists, load it */
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
  db=GWEN_DB_Group_new("bank");
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
                        const AH_CUSTOMER *cu,
                        GWEN_DB_NODE *dbMsg) {
  GWEN_BUFFER *nbuf;
  int rv;

  assert(hbci);
  assert(cu);

  /* create path */
  nbuf=GWEN_Buffer_new(0, 64, 0, 1);
  AH_HBCI_AddCustomerPath(hbci, cu, nbuf);

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
                        const AH_BANK *b,
                        GWEN_BUFFER *nbuf) {
  int country;
  const char *bankId;
  char numbuf[16];

  assert(hbci);
  assert(b);
  assert(nbuf);
  country=AH_Bank_GetCountry(b);
  bankId=AH_Bank_GetBankId(b);

  AB_Provider_GetUserDataDir(hbci->provider, nbuf);
  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP "banks" AH_PATH_SEP);

  snprintf(numbuf, sizeof(numbuf), "%d", country);
  GWEN_Buffer_AppendString(nbuf, numbuf);
  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP);
  GWEN_Buffer_AppendString(nbuf, bankId);
  return 0;
}



int AH_HBCI_AddUserPath(const AH_HBCI *hbci,
                        const AH_USER *u,
                        GWEN_BUFFER *nbuf) {
  AH_BANK *b;
  const char *userId;

  assert(hbci);
  assert(u);
  b=AH_User_GetBank(u);
  assert(b);
  if (AH_HBCI_AddBankPath(hbci, b, nbuf))
    return -1;

  userId=AH_User_GetUserId(u);
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
                            const AH_CUSTOMER *cu,
                            GWEN_BUFFER *nbuf) {
  AH_USER *u;
  const char *customerId;

  assert(hbci);
  assert(cu);
  u=AH_Customer_GetUser(cu);
  assert(u);
  if (AH_HBCI_AddUserPath(hbci, u, nbuf))
    return -1;
  GWEN_Buffer_AppendByte(nbuf, '/');

  /* escape and append customer name */
  customerId=AH_Customer_GetCustomerId(cu);
  if (GWEN_Path_Convert(customerId,
                        nbuf,
                        GWEN_PATH_FLAGS_ESCAPE |
                        GWEN_PATH_FLAGS_TOLERANT_ESCAPE)) {
    return -1;
  }
  return 0;
}



int AH_HBCI_AddAccountPath(const AH_HBCI *hbci,
                           const AH_ACCOUNT *a,
                           GWEN_BUFFER *nbuf) {
  AH_BANK *b;
  const char *accountId;

  assert(hbci);
  assert(a);
  b=AH_Account_GetBank(a);
  assert(b);
  if (AH_HBCI_AddBankPath(hbci, b, nbuf))
    return -1;

  accountId=AH_Account_GetAccountId(a);
  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP "accounts" AH_PATH_SEP);

  /* escape and append account id */
  if (GWEN_Path_Convert(accountId,
                        nbuf,
                        GWEN_PATH_FLAGS_ESCAPE |
                        GWEN_PATH_FLAGS_TOLERANT_ESCAPE)) {
    return -1;
  }
  return 0;
}




int AH_HBCI_AddObjectPath(const AH_HBCI *hbci,
                          int country,
                          const char *bankId,
                          const char *accountId,
                          const char *userId,
                          const char *customerId,
                          GWEN_BUFFER *nbuf) {
  char numbuf[16];

  assert(hbci);

  AB_Provider_GetUserDataDir(hbci->provider, nbuf);
  GWEN_Buffer_AppendString(nbuf, AH_PATH_SEP "banks" AH_PATH_SEP);

  if (!country)
    return 0;
  snprintf(numbuf, sizeof(numbuf), "%d", country);
  GWEN_Buffer_AppendString(nbuf, numbuf);

  if (!bankId)
    return 0;
  GWEN_Buffer_AppendByte(nbuf, '/');
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





