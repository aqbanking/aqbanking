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

#include <gwenhywfar/debug.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



int AH_HBCI_UpdateDb(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  int rv;
  GWEN_TYPE_UINT32 oldVersion;
  GWEN_TYPE_UINT32 currentVersion;

  if (0==GWEN_DB_Groups_Count(db) &&
      0==GWEN_DB_Variables_Count(db)) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN,
              "Initial setup, nothing to upgrade");
    return 0;
  }

  oldVersion=GWEN_DB_GetIntValue(db, "lastVersion", 0, 0);

  currentVersion=
    (AQHBCI_VERSION_MAJOR<<24) |
    (AQHBCI_VERSION_MINOR<<16) |
    (AQHBCI_VERSION_PATCHLEVEL<<8) |
    AQHBCI_VERSION_BUILD;

  if (currentVersion>oldVersion) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN,
               "Updating from %d.%d.%d.%d",
               (oldVersion>>24) & 0xff,
               (oldVersion>>16) & 0xff,
               (oldVersion>>8) & 0xff,
               oldVersion & 0xff);

    if (oldVersion < ((1<<24) | (0<<16) | (3<<8) | 9)) {
      rv=AH_HBCI_Update_1_0_3_9(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }

    if (oldVersion<((1<<24) | (2<<16) | (0<<8) | 3)) {
      rv=AH_HBCI_Update_1_2_0_3(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }

    if (oldVersion<((1<<24) | (4<<16) | (1<<8) | 2)) {
      rv=AH_HBCI_Update_1_4_1_2(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }

    if (oldVersion<((1<<24) | (8<<16) | (1<<8) | 3)) {
      rv=AH_HBCI_Update_1_8_1_3(hbci, db);
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



int AH_HBCI_Update2(AH_HBCI *hbci,
                    GWEN_DB_NODE *db,
                    GWEN_TYPE_UINT32 oldVersion,
                    GWEN_TYPE_UINT32 currentVersion) {
  int rv;

  if (0==GWEN_DB_Groups_Count(db) &&
      0==GWEN_DB_Variables_Count(db)) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN,
              "Initial setup, nothing to upgrade");
    return 0;
  }

  if (currentVersion>oldVersion) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN,
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



int AH_HBCI_Update_1_0_3_9(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  AB_Banking_MessageBox(AH_HBCI_GetBankingApi(hbci),
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
  return 0;
}



int AH_HBCI_Update_1_2_0_3(AH_HBCI *hbci, GWEN_DB_NODE *db) {
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

  return 0;
}



int AH_HBCI_Update_1_4_1_2(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  GWEN_DB_NODE *dbMedia;

  DBG_WARN(AQHBCI_LOGDOMAIN,
           "Updating from version prior to 1.4.1.2");

  dbMedia=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "media");
  if (dbMedia) {
    GWEN_DB_NODE *dbMedium;

    dbMedium=GWEN_DB_FindFirstGroup(dbMedia, "medium");
    while(dbMedium) {
      GWEN_DB_NODE *dbCtxList;

      dbCtxList=GWEN_DB_GetGroup(dbMedium, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                 "contextList");
      if (dbCtxList) {
        GWEN_DB_UnlinkGroup(dbCtxList);
        GWEN_DB_Group_free(dbCtxList);
      }
      dbMedium=GWEN_DB_FindNextGroup(dbMedium, "medium");
    }
  }

  return 0;
}



int AH_HBCI_Update_1_8_1_3(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  GWEN_DB_NODE *dbMedia;

  DBG_WARN(AQHBCI_LOGDOMAIN,
           "Updating from version prior to 1.8.1.3");

  dbMedia=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "media");
  if (dbMedia) {
    GWEN_DB_NODE *dbMedium;

    dbMedium=GWEN_DB_FindFirstGroup(dbMedia, "medium");
    while(dbMedium) {
      GWEN_TYPE_UINT32 uid;

      uid=GWEN_DB_GetIntValue(dbMedium, "uniqueId", 0, 0);
      if (uid==0) {
        uid=AH_HBCI_GetNextMediumId(hbci);

        DBG_NOTICE(AQHBCI_LOGDOMAIN,
                   "Assigning new unique id %08x to medium", uid);
        GWEN_DB_SetIntValue(dbMedium, GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "uniqueId", uid);
      }

      dbMedium=GWEN_DB_FindNextGroup(dbMedium, "medium");
    }
  }

  return 0;
}
















int AH_HBCI_Update2_1_8_1_3(AH_HBCI *hbci, GWEN_DB_NODE *db) {
  GWEN_DB_NODE *dbBanks;
  AB_BANKING *ab;

  ab=AH_HBCI_GetBankingApi(hbci);
  assert(ab);
  dbBanks=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "banks");
  if (dbBanks) {
    GWEN_DB_NODE *dbBank;

    dbBank=GWEN_DB_FindFirstGroup(dbBanks, "bank");
    while(dbBank) {
      GWEN_DB_NODE *dbUsers;
      GWEN_DB_NODE *dbAccounts;
      const char *bankId;

      bankId=GWEN_DB_GetCharValue(dbBank, "bankId", 0, 0);
      dbUsers=GWEN_DB_GetGroup(dbBank,
                               GWEN_PATH_FLAGS_NAMEMUSTEXIST, "users");
      if (bankId && dbUsers) {
        GWEN_DB_NODE *dbUser;

        dbUser=GWEN_DB_FindFirstGroup(dbUsers, "user");
        while(dbUser) {
          GWEN_DB_NODE *dbCustomers;
          const char *userId;

          userId=GWEN_DB_GetCharValue(dbUser, "userId", 0, 0);
          dbCustomers=GWEN_DB_GetGroup(dbUser,
                                       GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                       "customers");
          if (userId && dbCustomers) {
            GWEN_DB_NODE *dbCustomer;

            dbCustomer=GWEN_DB_FindFirstGroup(dbCustomers, "customer");
            while(dbCustomer) {
              const char *customerId;

              customerId=GWEN_DB_GetCharValue(dbCustomer, "customerId", 0, 0);
              if (customerId) {
                AB_USER *u;

                u=AB_Banking_FindUser(ab,
                                      AH_PROVIDER_NAME,
                                      "*",
                                      bankId, userId, customerId);
                if (!u) {
                  GWEN_DB_NODE *dbT;
                  const char *mediumTypeName;
                  const char *mediumName;
                  int contextIdx;
                  AH_MEDIUM *m;
                  const char *s;

                  /* create user */
                  u=AB_Banking_CreateUser(ab, AH_PROVIDER_NAME);
                  assert(u);

                  /* set basic data */
                  AB_User_SetCountry(u, "de");
                  AB_User_SetBankCode(u, bankId);
                  AB_User_SetUserId(u, userId);
                  AB_User_SetCustomerId(u, customerId);

                  /* set medium and context idx */
                  mediumTypeName=GWEN_DB_GetCharValue(dbUser,
                                                      "medium/mediumTypeName",
                                                      0, 0);
                  assert(mediumTypeName);
                  mediumName=GWEN_DB_GetCharValue(dbUser,
                                                  "medium/mediumName",
                                                  0, 0);
                  contextIdx=GWEN_DB_GetIntValue(dbUser,
                                                 "contextIdx", 0, 0);

                  m=AH_HBCI_FindMedium(hbci, mediumTypeName,
                                       mediumName);
                  assert(m);
                  AH_User_SetMedium(u, m);
                  AH_User_SetContextIdx(u, contextIdx);

                  /* medium type -> cryptMode */
                  s=GWEN_DB_GetCharValue(dbUser, "cryptMode", 0, 0);
                  assert(s);
                  AH_User_SetCryptMode(u, AH_CryptMode_fromString(s));

                  /* set system id */
                  s=GWEN_DB_GetCharValue(dbCustomer, "systemId", 0, 0);
                  if (s)
                    AH_User_SetSystemId(u, s);

                  /* set bpd addr */
                  dbT=GWEN_DB_GetGroup(dbCustomer,
                                       GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                       "server");
                  if (dbT) {
                    AH_BPD_ADDR *addr;

                    addr=AH_BpdAddr_FromDb(dbT);
                    assert(addr);
                    AH_User_SetAddress(u, addr);
                    AH_BpdAddr_free(addr);
                  }

                  /* set BPD */
                  dbT=GWEN_DB_GetGroup(dbCustomer,
                                       GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                       "bpd");
                  if (dbT) {
                    AH_BPD *bpd;

                    bpd=AH_Bpd_FromDb(dbT);
                    assert(bpd);
                    AH_User_SetBpd(u, bpd);
                  }

                  /* enforce update */
                  AH_User_SetUpdVersion(u, 0);
                  AH_User_SetBpdVersion(u, 0);

                  /* add user */
                  DBG_NOTICE(AQHBCI_LOGDOMAIN,
                             "Adding user %s/%s/%s",
                             bankId, userId, customerId);
                  AB_Banking_AddUser(ab, u);
                }
              } /* if customerId */
              dbCustomer=GWEN_DB_FindNextGroup(dbCustomer, "customer");
            } /* while customer */
          } /* if customers */

          dbUser=GWEN_DB_FindNextGroup(dbUser, "user");
        } /* while user */
      } /* if users */

      dbAccounts=GWEN_DB_FindFirstGroup(dbBank, "accounts");
      if (bankId && dbAccounts) {
        GWEN_DB_NODE *dbAccount;

        dbAccount=GWEN_DB_FindFirstGroup(dbAccounts, "account");
        while(dbAccount) {
          const char *bankCode;
          const char *accountNr;

          bankCode=GWEN_DB_GetCharValue(dbAccount, "bankId", 0, 0);
          accountNr=GWEN_DB_GetCharValue(dbAccount, "accountId", 0, 0);
          if (bankCode && accountNr) {
            AB_ACCOUNT *a;
            const char *custId;

            custId=GWEN_DB_GetCharValue(dbAccount, "customer", 0, 0);
            assert(custId);
            a=AB_Banking_FindAccount(ab, AH_PROVIDER_NAME,
                                     "*", bankCode, accountNr);
            if (!a) {
              const char *accName;
              const char *bankName;
              const char *ownerName;
              AB_USER *u;

              accName=GWEN_DB_GetCharValue(dbAccount, "accountName", 0, 0);
              bankName=GWEN_DB_GetCharValue(dbAccount, "bankName", 0, 0);
              ownerName=GWEN_DB_GetCharValue(dbAccount, "ownerName", 0, 0);
              assert(ownerName);
              a=AB_Banking_CreateAccount(ab, AH_PROVIDER_NAME);
              assert(a);
              AB_Account_SetBankCode(a, bankCode);
              AB_Account_SetAccountNumber(a, accountNr);
              AB_Account_SetCountry(a, "de");
              if (accName)
                AB_Account_SetAccountName(a, accName);
              if (bankName)
                AB_Account_SetBankName(a, bankName);
              AB_Account_SetOwnerName(a, ownerName);

              u=AB_Banking_FindUser(ab, AH_PROVIDER_NAME,
                                    "*", bankId, "*", custId);
              assert(u);
              AB_Account_SetUser(a, u);
              AB_Account_SetSelectedUser(a, u);

              DBG_NOTICE(AQHBCI_LOGDOMAIN,
                         "Adding account %s/%s",
                         bankId, accountNr);
              AB_Banking_AddAccount(ab, a);
            }
            else {
              if (AB_Account_GetFirstUser(a)==0) {
                AB_USER *u;

                DBG_NOTICE(AQHBCI_LOGDOMAIN,
                           "Setting user \"%s\" in account %s/%s",
                           custId, bankId, accountNr);
                u=AB_Banking_FindUser(ab, AH_PROVIDER_NAME,
                                      "*", bankId, "*", custId);
                if (u) {
                  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Setting user in account");
                  AB_Account_SetUser(a, u);
                  AB_Account_SetSelectedUser(a, u);
                }
                else {
                  DBG_NOTICE(AQHBCI_LOGDOMAIN,
                             "User \"%s\" for account %s/%s not found",
                             custId, bankId, accountNr);
                }
              }
            }
          }
          dbAccount=GWEN_DB_FindNextGroup(dbAccount, "account");
        }
      }

      dbBank=GWEN_DB_FindNextGroup(dbBank, "bank");
    } /* while bank */
  } /* if banks */

  return 0;
}





