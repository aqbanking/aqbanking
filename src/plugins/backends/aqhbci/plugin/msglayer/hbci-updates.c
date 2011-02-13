/***************************************************************************
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

    if (oldVersion<((3<<24) | (1<<16) | (1<<8) | 2)) {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Updating user from pre 3.1.1.2");
      rv=AH_HBCI_UpdateUser_3_1_1_2(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }

    if (oldVersion<((5<<24) | (0<<16) | (3<<8) | 1)) {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Updating user from pre 5.0.3.1");
      rv=AH_HBCI_UpdateUser_5_0_3_1(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }

    /* insert more updates here */


    /* updated! */
    return 1;
  } /* if update */
  else
    /* not updated */
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


    /* updated! */
    return 1;
  } /* if update */
  else
    /* not updated */
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
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "tanMethods", "singleStep");
  return 0;
}



int AH_HBCI_UpdateUser_2_9_3_2(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  int i;

  /* adjust token settings */
  if (GWEN_DB_GetCharValue(db, "tokenType", 0, NULL)==NULL) {
    GWEN_DB_NODE *dbMedia;

    dbMedia=AH_HBCI_GetProviderDb(hbci);

    if (dbMedia)
      dbMedia=GWEN_DB_GetGroup(dbMedia, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "media");
    if (dbMedia) {
      int mediumId;

      mediumId=GWEN_DB_GetIntValue(db, "medium", 0, 0);
      if (mediumId) {
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
  }

  /* adjust context id */
  i=GWEN_DB_GetIntValue(db, "contextIdx", 0, 0);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "contextId", i);

  /* adjust rdh type */
  i=GWEN_DB_GetIntValue(db, "rdhType", 0, -1);
  if (i<1) {
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



int AH_HBCI_UpdateUser_3_1_1_2(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  int i;

  GWEN_DB_DeleteVar(db, "tanMethodList");
  for (i=0; ; i++) {
    const char *s;

    s=GWEN_DB_GetCharValue(db, "tanMethods", i, 0);
    if (!s)
      break;

    if (strcasecmp(s, "singleStep")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 999);
    else if (strcasecmp(s, "twoStep0")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 990);
    else if (strcasecmp(s, "twoStep1")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 991);
    else if (strcasecmp(s, "twoStep2")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 992);
    else if (strcasecmp(s, "twoStep3")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 993);
    else if (strcasecmp(s, "twoStep4")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 994);
    else if (strcasecmp(s, "twoStep5")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 995);
    else if (strcasecmp(s, "twoStep6")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 996);
    else if (strcasecmp(s, "twoStep7")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 997);
    else if (strcasecmp(s, "twoStep00")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 900);
    else if (strcasecmp(s, "twoStep01")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 901);
    else if (strcasecmp(s, "twoStep02")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 902);
    else if (strcasecmp(s, "twoStep03")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 903);
    else if (strcasecmp(s, "twoStep04")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 904);
    else if (strcasecmp(s, "twoStep05")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 905);
    else if (strcasecmp(s, "twoStep06")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 906);
    else if (strcasecmp(s, "twoStep07")==0)
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
			  "tanMethodList", 907);
  }
  return 0;
}



int AH_HBCI_UpdateUser_5_0_3_1(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  int tmn;

  /* update selectedTanMethod */
  tmn=GWEN_DB_GetIntValue(db, "selectedTanMethod", 0, 0);
  if (tmn>0 && tmn < 1000) {
    GWEN_DB_NODE *dbT;

    /* get first version group of "bpd/bpdjobs/HITANS" */
    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bpd");
    if (dbT)
      dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bpdjobs");
    if (dbT)
      dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "HITANS");
    if (dbT)
      dbT=GWEN_DB_GetFirstGroup(dbT);

    if (dbT) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Starting with group %s", GWEN_DB_GroupName(dbT));
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No group");
    }

    while(dbT) {
      int foundTm=0;
      int version;

      /* find tanMethod group in any of the tanMethod groups of every HITANS version */
      version=atoi(GWEN_DB_GroupName(dbT));
      if (version>0) {
        GWEN_DB_NODE *dbM;

        dbM=GWEN_DB_FindFirstGroup(dbT, "tanMethod");
        while(dbM) {
          int fn;

          fn=GWEN_DB_GetIntValue(dbM, "function", 0, 0);
          if (fn==tmn) {
            int newFn;

            newFn=(version*1000)+fn;
            DBG_WARN(AQHBCI_LOGDOMAIN, "Updating selectedTanMethod from %d to %d", tmn, newFn);
            GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                "selectedTanMethod", newFn);
            foundTm=1;
            break;
          }
          dbM=GWEN_DB_FindNextGroup(dbM, "tanMethod");
        }
      }
      if (foundTm)
        break;
      dbT=GWEN_DB_GetNextGroup(dbT);
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No selectedTanMethod");
  }

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












