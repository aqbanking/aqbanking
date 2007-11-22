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


#include "i18n_l.h"
#include "hbci-updates_p.h"
#include "user_l.h"
#include "account_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



int AH_HBCI_UpdateDbUser(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  int rv;
  uint32_t oldVersion;
  uint32_t currentVersion;

  oldVersion=AH_HBCI_GetLastVersion(hbci);

  currentVersion=
    (AQHBCI_VERSION_MAJOR<<24) |
    (AQHBCI_VERSION_MINOR<<16) |
    (AQHBCI_VERSION_PATCHLEVEL<<8) |
    AQHBCI_VERSION_BUILD;

  if (currentVersion>oldVersion) {
    DBG_WARN(AQHBCI_LOGDOMAIN,
             "Updating user from %d.%d.%d.%d",
             (oldVersion>>24) & 0xff,
             (oldVersion>>16) & 0xff,
             (oldVersion>>8) & 0xff,
             oldVersion & 0xff);

    if (oldVersion<((1<<24) | (9<<16) | (7<<8) | 7)) {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Updating user from pre 1.9.7.7");
      rv=AH_HBCI_UpdateUser_1_9_7_7(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }

    if (oldVersion<((2<<24) | (1<<16) | (1<<8) | 1)) {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Updating user from pre 2.1.1.1");
      rv=AH_HBCI_UpdateUser_2_1_1_1(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }

    if (oldVersion<((2<<24) | (9<<16) | (3<<8) | 2)) {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Updating user from pre 2.9.3.2");
      rv=AH_HBCI_UpdateUser_2_9_3_2(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }

    if (oldVersion<((2<<24) | (9<<16) | (3<<8) | 3)) {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Updating user from pre 2.9.3.3");
      rv=AH_HBCI_UpdateUser_2_9_3_3(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }

    /* insert more updates here */


  } /* if update */

  return 0;
}



int AH_HBCI_UpdateDbAccount(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  uint32_t oldVersion;
  uint32_t currentVersion;
  int rv;

  oldVersion=AH_HBCI_GetLastVersion(hbci);

  currentVersion=
    (AQHBCI_VERSION_MAJOR<<24) |
    (AQHBCI_VERSION_MINOR<<16) |
    (AQHBCI_VERSION_PATCHLEVEL<<8) |
    AQHBCI_VERSION_BUILD;

  if (currentVersion>oldVersion) {
    DBG_WARN(AQHBCI_LOGDOMAIN,
             "Updating account from %d.%d.%d.%d",
             (oldVersion>>24) & 0xff,
             (oldVersion>>16) & 0xff,
             (oldVersion>>8) & 0xff,
             oldVersion & 0xff);

    if (oldVersion<((1<<24) | (9<<16) | (7<<8) | 9)) {
      rv=AH_HBCI_UpdateAccount_1_9_7_9(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
    /* insert more updates here */


  } /* if update */

  return 0;
}



int AH_HBCI_Update2(AH_HBCI *hbci,
                    GWEN_DB_NODE *db,
                    uint32_t oldVersion,
                    uint32_t currentVersion) {
  int rv;

  if (0==GWEN_DB_Groups_Count(db) &&
      0==GWEN_DB_Variables_Count(db)) {
    DBG_WARN(AQHBCI_LOGDOMAIN,
             "Initial setup, nothing to upgrade");
    return 0;
  }

  if (currentVersion>oldVersion) {
    DBG_WARN(AQHBCI_LOGDOMAIN,
             "Updating from %d.%d.%d.%d",
             (oldVersion>>24) & 0xff,
             (oldVersion>>16) & 0xff,
             (oldVersion>>8) & 0xff,
             oldVersion & 0xff);

    if (oldVersion < ((1<<24) | (8<<16) | (1<<8) | 3)) {
      rv=AH_HBCI_Update2_1_8_1_3(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }

    if (oldVersion < ((2<<24) | (9<<16) | (3<<8) | 3)) {
      rv=AH_HBCI_Update2_2_9_3_3(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }

    /* insert more updates here */


    /* this should follow any version-specific update */
    rv=AH_HBCI_Update_Any(hbci, db);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  } /* if update */

  return 0;
}



int AH_HBCI_Update_Any(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  GWEN_DB_NODE *dbBanks;

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
          GWEN_DB_NODE *dbCustomers;
    
          dbCustomers=GWEN_DB_GetGroup(dbUser,
                                       GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                       "customers");
          if (dbCustomers) {
            GWEN_DB_NODE *dbCustomer;

            dbCustomer=GWEN_DB_FindFirstGroup(dbCustomers,
                                              "customer");
            while(dbCustomer) {
              GWEN_DB_NODE *dbBpd;

              GWEN_DB_SetIntValue(dbCustomer, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                  "updVersion", 0);

              dbBpd=GWEN_DB_GetGroup(dbCustomer,
                                     GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                     "bpd");
              if (dbBpd) {
                GWEN_DB_SetIntValue(dbBpd,
                                    GWEN_DB_FLAGS_OVERWRITE_VARS,
                                    "bpdVersion", 0);
              }
              dbCustomer=GWEN_DB_FindNextGroup(dbCustomer,
                                               "customer");
            } /* if customer */
          } /* if customers */

          dbUser=GWEN_DB_FindNextGroup(dbUser, "user");
        } /* if user */
      } /* if users */
      dbBank=GWEN_DB_FindNextGroup(dbBank, "bank");
    } /* if bank */
  } /* if banks */

  return 0;
}



int AH_HBCI_Update2_1_8_1_3(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  DBG_ERROR(AQHBCI_LOGDOMAIN, "Version is too old, can't autoupgrade");

  GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO |
		      GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
		      GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL,
		      I18N("AqHBCI-Notice"),
		      I18N(
  "The version of AqBanking/AqHBCI previously used is too old to be\n"
  "upgraded automatically.\n"
  "Therefore you should delete the settings file and setup AqBanking\n"
  "completely from scratch.\n"
  "The settings file usually is\n"
   "  $HOME/.banking/settings.conf\n"
   "<html>"
  "<p>"
  "The version of AqBanking/AqHBCI previously used is too old to be\n"
  "upgraded automatically.\n"
  "</p>"
  "<p>"
  "Therefore you should delete the settings file and setup AqBanking\n"
  "completely from scratch.\n"
  "</p>"
  "<p>"
  "The settings file usually is: \n"
  "<i>"
  "$HOME/.banking/settings.conf\n"
  "</i>.\n"
   "</p>"
   "</html>"
			  ),
		      I18N("Continue"), 0, 0, 0);
  return GWEN_ERROR_INTERNAL;
}



int AH_HBCI_Update2_2_9_3_3(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  GWEN_DB_ClearGroup(db, "media");

  return 0;
}






int AH_HBCI_UpdateUser_1_9_7_7(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  GWEN_DB_NODE *dbT;
  
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "server");
  if (dbT) {
    const char *s_addr;
    const char *s_port;
    const char *s_type;
  
    s_addr=GWEN_DB_GetCharValue(dbT, "address", 0, 0);
    s_port=GWEN_DB_GetCharValue(dbT, "suffix", 0, 0);
    s_type=GWEN_DB_GetCharValue(dbT, "type", 0, "tcp");
    if (s_addr) {
      GWEN_URL *url;
      int bankPort=0;
      GWEN_BUFFER *ubuf;

      if (s_port)
        bankPort=atoi(s_port);
      url=GWEN_Url_fromString(s_addr);
      assert(url);
      if (s_type && strcasecmp(s_type, "ssl")==0) {
        GWEN_Url_SetProtocol(url, "https");
        if (bankPort==0)
          bankPort=443;
        GWEN_Url_SetPort(url, bankPort);
      }
      else {
        GWEN_Url_SetProtocol(url, "hbci");
        if (bankPort==0)
          bankPort=3000;
        GWEN_Url_SetPort(url, bankPort);
      }

      GWEN_DB_UnlinkGroup(dbT);
      GWEN_DB_Group_free(dbT);

      ubuf=GWEN_Buffer_new(0, 256, 0, 1);
      if (GWEN_Url_toString(url, ubuf)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not store url");
        GWEN_Url_free(url);
        return -1;
      }
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "server", GWEN_Buffer_GetStart(ubuf));
      GWEN_Buffer_free(ubuf);
      GWEN_Url_free(url);
    }
  }

  return 0;
}



int AH_HBCI_UpdateUser_2_1_1_1(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  uint32_t tm;

  tm=AH_USER_TANMETHOD_SINGLE_STEP;
  AH_User_TanMethods_toDb(db, "tanMethods", tm);
  return 0;
}



int AH_HBCI_UpdateUser_2_9_3_2(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  int mediumId;
  int i;

  mediumId=GWEN_DB_GetIntValue(db, "medium", 0, 0);
  if (mediumId) {
    AB_PROVIDER *pro;
    GWEN_DB_NODE *dbPro;
    GWEN_DB_NODE *dbMedia;

    pro=AH_HBCI_GetProvider(hbci);
    assert(pro);
    dbPro=AB_Provider_GetData(pro);
    assert(dbPro);

    dbMedia=GWEN_DB_GetGroup(dbPro, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			     "media");
    if (dbMedia) {
      GWEN_DB_NODE *dbT;

      dbT=GWEN_DB_FindFirstGroup(dbMedia, "medium");
      while(dbT) {
	i=GWEN_DB_GetIntValue(dbT, "uniqueId", 0, 0);
	if (i) {
	  if (i==mediumId) {
	    const char *typeName;
	    const char *name;

	    name=GWEN_DB_GetCharValue(dbT, "mediumName", 0, 0);
	    assert(name);
	    typeName=GWEN_DB_GetCharValue(dbT, "mediumTypeName", 0, 0);
	    assert(typeName);

	    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
				 "tokenType", typeName);
	    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
				 "tokenName", name);
	    break;
	  }
	}
	dbT=GWEN_DB_FindNextGroup(dbT, "medium");
      }
    }
  }

  /* adjust context id */
  i=GWEN_DB_GetIntValue(db, "contextIdx", 0, 0);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "contextId", i);

  /* adjust rdh type */
  i=GWEN_DB_GetIntValue(db, "rdhType", 0, -1);
  if (i==-1) {
    const char *s;

    s=GWEN_DB_GetCharValue(db, "cryptMode", 0, 0);
    if (s && strcasecmp(s, "rdh")==0)
      i=1; /* default is 1 if no type was set in RDH mode */
    else
      i=0; /* default is 0 in any mode other than rdh */
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"rdhType", i);
  }

  return 0;
}



int AH_HBCI_UpdateUser_2_9_3_3(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  int i;

  /* create tokenContextId from medium context id */
  i=GWEN_DB_GetIntValue(db, "contextId", 0, 0);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "tokenContextId", i+1);

  return 0;
}









int AH_HBCI_UpdateAccount_1_9_7_9(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  uint32_t flags;

  flags=AH_Account_Flags_fromDb(db, "accountFlags");
  if (flags==0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN,
               "Setting account flags to default");
    flags=AH_BANK_FLAGS_DEFAULT;
    AH_Account_Flags_toDb(db, "accountFlags", flags);
  }

  return 0;
}












