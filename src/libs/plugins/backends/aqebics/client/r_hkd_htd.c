/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "r_hkd_htd_l.h"
#include "provider_l.h"
#include "aqebics_l.h"
#include "account_l.h"

#include "msg/msg.h"
#include "msg/keys.h"
#include "msg/zip.h"
#include "msg/xml.h"
#include "user_l.h"

#include <gwenhywfar/base64.h>
#include <gwenhywfar/gui.h>


static int _xchgHkdRequest(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u, const char *requestName);

static void _sampleAccounts(xmlNodePtr node, GWEN_DB_NODE *dbAll);
static AB_ACCOUNT_LIST *_readAccounts(AB_PROVIDER *pro, GWEN_DB_NODE *dbAll);
static AB_ACCOUNT *_readAccount(AB_PROVIDER *pro, GWEN_DB_NODE *db);
static void _removeEmptyAccountsFromList(AB_ACCOUNT_LIST *accList);
static void _assignIdsOfStoredAccounts(AB_PROVIDER *pro, AB_ACCOUNT_LIST *accountList);
static void _addOrModifyAccounts(AB_PROVIDER *pro, AB_USER *user, AB_ACCOUNT_LIST *accountList);
static int _modifyExistingAccount(AB_PROVIDER *pro, AB_USER *user, AB_ACCOUNT *account);
static int _addAccount(AB_PROVIDER *pro, AB_USER *user, AB_ACCOUNT *account);



int EBC_Provider_XchgHkdRequest(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u)
{
  int rv;

  rv=_xchgHkdRequest(pro, sess, u, "HKD");
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int EBC_Provider_XchgHtdRequest(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u)
{
  int rv;

  rv=_xchgHkdRequest(pro, sess, u, "HTD");
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}




int _xchgHkdRequest(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u, const char *requestName)
{
  int rv;
  GWEN_BUFFER *buf;

  buf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=EBC_Provider_XchgDownloadRequest(pro, sess, u, requestName, buf, 0, NULL, NULL);
  if (rv<0 || rv>=300) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(buf);
    return rv;
  }
  else {
    xmlDocPtr orderDoc=NULL;
    xmlNodePtr root_node=NULL;
    xmlNodePtr node=NULL;
    GWEN_DB_NODE *dbAll;
    AB_ACCOUNT_LIST *accountList;

    /* parse XML document */
    rv=EB_Xml_DocFromBuffer(GWEN_Buffer_GetStart(buf), GWEN_Buffer_GetUsedBytes(buf), &orderDoc);
    GWEN_Buffer_free(buf);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    root_node=xmlDocGetRootElement(orderDoc);

    node=EB_Xml_GetNode(root_node, "PartnerInfo", GWEN_PATH_FLAGS_NAMEMUSTEXIST);
    if (node==NULL) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "No PartnerInfo found");
      xmlFreeDoc(orderDoc);
      return GWEN_ERROR_BAD_DATA;
    }

    dbAll=GWEN_DB_Group_new("Response");

    _sampleAccounts(node, dbAll);
    accountList=_readAccounts(pro, dbAll);
    if (accountList) {
      _removeEmptyAccountsFromList(accountList);
      _assignIdsOfStoredAccounts(pro, accountList);
      _addOrModifyAccounts(pro, u, accountList);

      AB_Account_List_free(accountList);
    }

    GWEN_DB_Group_free(dbAll);
    xmlFreeDoc(orderDoc);
    return 0;
  }
}



void _sampleAccounts(xmlNodePtr node, GWEN_DB_NODE *dbAll)
{
  xmlNodePtr nodeX;

  /* sample accounts */
  nodeX=node->children;
  while (nodeX) {
    if (nodeX->type==XML_ELEMENT_NODE) {
      if (nodeX->name && strcmp((const char *)nodeX->name, "AccountInfo")==0) {
        GWEN_DB_NODE *db;
        xmlChar *xs;
        xmlNodePtr nodeXX;

        DBG_DEBUG(AQEBICS_LOGDOMAIN, "Reading AccountInfo node");

        db=GWEN_DB_GetGroup(dbAll, GWEN_PATH_FLAGS_CREATE_GROUP, "Account");

        xs=xmlGetProp(nodeX, BAD_CAST "ID");
        if (xs) {
          GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "EbicsId", (const char *)xs);
          xmlFree(xs);
        }

        xs=xmlGetProp(nodeX, BAD_CAST "Currency");
        if (xs) {
          GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Currency", (const char *)xs);
          xmlFree(xs);
        }

        xs=xmlGetProp(nodeX, BAD_CAST "Description");
        if (xs) {
          GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "AccountName", (const char *)xs);
          xmlFree(xs);
        }

        nodeXX=nodeX->children;
        while (nodeXX) {
          if (nodeXX->type==XML_ELEMENT_NODE &&
              nodeXX->name) {
            if (strcmp((const char *)nodeXX->name, "AccountNumber")==0) {
              xs=xmlGetProp(nodeXX, BAD_CAST "international");
              if (xs) {
                xmlNodePtr nodeXXX=NULL;

                if (strcasecmp((const char *)xs, "false")==0) {
                  nodeXXX=nodeXX->children;
                  if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
                    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "AccountNumber", (const char *)nodeXXX->content);
                  }
                }
                else {
                  nodeXXX=nodeXX->children;
                  if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
                    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "IBAN", (const char *)nodeXXX->content);
                  }
                }
                xmlFree(xs);
              }
            }
            else if (strcmp((const char *)nodeXX->name, "NationalAccountNumber")==0) {
              xmlNodePtr nodeXXX=NULL;

              nodeXXX=nodeXX->children;
              if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
                GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "AccountNumber", (const char *)nodeXXX->content);
              }
            }
            else if (strcmp((const char *)nodeXX->name, "BankCode")==0) {
              xs=xmlGetProp(nodeXX, BAD_CAST "international");
              if (xs) {
                xmlNodePtr nodeXXX=NULL;

                if (strcasecmp((const char *)xs, "false")==0) {
                  nodeXXX=nodeXX->children;
                  if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
                    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "BankCode", (const char *)nodeXXX->content);
                  }
                }
                else {
                  nodeXXX=nodeXX->children;
                  if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
                    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "BIC", (const char *)nodeXXX->content);
                  }
                }
                xmlFree(xs);
              }
            }
            else if (strcmp((const char *)nodeXX->name, "NationalBankCode")==0) {
              xmlNodePtr nodeXXX=NULL;

              nodeXXX=nodeXX->children;
              if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
                GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "BankCode", (const char *)nodeXXX->content);
              }
            }
            else if (strcmp((const char *)nodeXX->name, "AccountHolder")==0) {
              xmlNodePtr nodeXXX=NULL;

              nodeXXX=nodeXX->children;
              if (nodeXXX->type==XML_TEXT_NODE && nodeXXX->content) {
                GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "owner", (const char *)nodeXXX->content);
              }
            }
          }
          nodeXX=nodeXX->next;
        }
      }
    }
    nodeX=nodeX->next;
  }
}



AB_ACCOUNT_LIST *_readAccounts(AB_PROVIDER *pro, GWEN_DB_NODE *dbAll)
{
  AB_ACCOUNT_LIST *accountList;
  GWEN_DB_NODE *db;

  accountList=AB_Account_List_new();

  db=GWEN_DB_FindFirstGroup(dbAll, "Account");
  while (db) {
    AB_ACCOUNT *account;

    account=_readAccount(pro, db);
    if (account)
      AB_Account_List_Add(account, accountList);

    db=GWEN_DB_FindNextGroup(db, "Account");
  }

  if (AB_Account_List_GetCount(accountList)==0) {
    AB_Account_List_free(accountList);
    return NULL;
  }

  return accountList;
}



AB_ACCOUNT *_readAccount(AB_PROVIDER *pro, GWEN_DB_NODE *db)
{
  AB_ACCOUNT *a;
  const char *s;

  a=AB_Provider_CreateAccountObject(pro);
  assert(a);

  AB_Account_SetAccountType(a, AB_AccountType_Bank);

  AB_Account_SetCountry(a, "de");

  s=GWEN_DB_GetCharValue(db, "ebicsId", 0, NULL);
  if (s && *s)
    EBC_Account_SetEbicsId(a, s);

  s=GWEN_DB_GetCharValue(db, "bankCode", 0, NULL);
  if (s && *s)
    AB_Account_SetBankCode(a, s);

  s=GWEN_DB_GetCharValue(db, "accountNumber", 0, NULL);
  if (s && *s)
    AB_Account_SetAccountNumber(a, s);

  s=GWEN_DB_GetCharValue(db, "owner", 0, NULL);
  if (s && *s)
    AB_Account_SetOwnerName(a, s);

#if 0
  s=GWEN_DB_GetCharValue(db, "accountName", 0, NULL);
  if (s && *s)
    AB_Account_SetAccountName(a, s);
#endif

  s=GWEN_DB_GetCharValue(db, "currency", 0, NULL);
  if (s && *s)
    AB_Account_SetCurrency(a, s);
  else
    AB_Account_SetCurrency(a, "EUR");

  s=GWEN_DB_GetCharValue(db, "IBAN", 0, NULL);
  if (s && *s)
    AB_Account_SetIban(a, s);

  s=GWEN_DB_GetCharValue(db, "BIC", 0, NULL);
  if (s && *s)
    AB_Account_SetBic(a, s);

  return a;
}



void _removeEmptyAccountsFromList(AB_ACCOUNT_LIST *accList)
{
  /* only keep accounts which have at least IBAN or bankcode and account number */
  DBG_INFO(AQEBICS_LOGDOMAIN, "Checking for empty accounts");
  if (AB_Account_List_GetCount(accList)) {
    AB_ACCOUNT *acc;

    acc=AB_Account_List_First(accList);
    while (acc) {
      AB_ACCOUNT *accNext;
      const char *accountNum;
      const char *bankCode;
      const char *iban;

      accNext=AB_Account_List_Next(acc);
      accountNum=AB_Account_GetAccountNumber(acc);
      bankCode=AB_Account_GetBankCode(acc);
      iban=AB_Account_GetIban(acc);

      if (!((iban && *iban) || (accountNum && *accountNum && bankCode && *bankCode))) {
        DBG_INFO(AQEBICS_LOGDOMAIN, "Removing empty account from import list");
        AB_Account_List_Del(acc);
        AB_Account_free(acc);
      }
      acc=accNext;
    } /* while(acc) */
  } /* if (AB_Account_List_GetCount(accList)) */
}



void _assignIdsOfStoredAccounts(AB_PROVIDER *pro, AB_ACCOUNT_LIST *accountList)
{
  AB_ACCOUNT_SPEC_LIST *accountSpecList=NULL;
  int rv;

  accountSpecList=AB_AccountSpec_List_new();
  rv=AB_Banking_GetAccountSpecList(AB_Provider_GetBanking(pro), &accountSpecList);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "No account spec list");
  }
  else {
    AB_ACCOUNT *account;

    account=AB_Account_List_First(accountList);
    while (account) {
      AB_ACCOUNT_SPEC *accountSpec;

      accountSpec=AB_Provider_FindMatchingAccountSpec(pro, account, accountSpecList);
      if (accountSpec) {
        uint32_t uniqueId;

        uniqueId=AB_AccountSpec_GetUniqueId(accountSpec);
        DBG_INFO(AQEBICS_LOGDOMAIN, "Found a matching account (%x)", uniqueId);
        AB_Account_SetUniqueId(account, uniqueId);
      }

      account=AB_Account_List_Next(account);
    }
  }
  AB_AccountSpec_List_free(accountSpecList);
}



void _addOrModifyAccounts(AB_PROVIDER *pro, AB_USER *user, AB_ACCOUNT_LIST *accountList)
{
  AB_ACCOUNT *account;

  account=AB_Account_List_First(accountList);
  while (account) {
    uint32_t uniqueId;

    uniqueId=AB_Account_GetUniqueId(account);
    if (uniqueId) {
      int rv;

      rv=_modifyExistingAccount(pro, user, account);
      if (rv<0) {
        DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      }
    }
    else {
      int rv;

      rv=_addAccount(pro, user, account);
      if (rv<0) {
        DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      }
    }
    account=AB_Account_List_Next(account);
  }
}



int _modifyExistingAccount(AB_PROVIDER *pro, AB_USER *user, AB_ACCOUNT *account)
{
  int rv;
  AB_ACCOUNT *storedAccount=NULL;
  uint32_t uniqueId;

  uniqueId=AB_Account_GetUniqueId(account);

  /* account already exists, needs update */
  DBG_ERROR(AQEBICS_LOGDOMAIN, "Account exists, modifying");
  rv=AB_Provider_GetAccount(pro, uniqueId, 1, 0, &storedAccount); /* lock, don't unlock */
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error getting referenced account (%d)", rv);
    return rv;
  }
  else {
    const char *s;

    /* account is locked now, apply changes */
    assert(storedAccount);

    s=EBC_Account_GetEbicsId(account);
    if (s && *s)
      EBC_Account_SetEbicsId(storedAccount, s);

    s=AB_Account_GetCountry(account);
    if (s && *s)
      AB_Account_SetCountry(storedAccount, s);

    s=AB_Account_GetBankCode(account);
    if (s && *s)
      AB_Account_SetBankCode(storedAccount, s);

    s=AB_Account_GetBankName(account);
    if (s && *s)
      AB_Account_SetBankName(storedAccount, s);

    s=AB_Account_GetAccountNumber(account);
    if (s && *s)
      AB_Account_SetAccountNumber(storedAccount, s);

    s=AB_Account_GetSubAccountId(account);
    if (s && *s)
      AB_Account_SetSubAccountId(storedAccount, s);

    s=AB_Account_GetIban(account);
    if (s && *s)
      AB_Account_SetIban(storedAccount, s);

    s=AB_Account_GetBic(account);
    if (s && *s)
      AB_Account_SetBic(storedAccount, s);

    s=AB_Account_GetOwnerName(account);
    if (s && *s)
      AB_Account_SetOwnerName(storedAccount, s);

    s=AB_Account_GetCurrency(account);
    if (s && *s)
      AB_Account_SetCurrency(storedAccount, s);

    AB_Account_SetAccountType(storedAccount, AB_Account_GetAccountType(account));

    /* add flags from new account */
    EBC_Account_AddFlags(storedAccount, EBC_Account_GetFlags(account));

    /* handle users */
    AB_Account_SetUserId(storedAccount, AB_User_GetUniqueId(user));

    /* unlock account */
    rv=AB_Provider_EndExclUseAccount(pro, storedAccount, 0);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      AB_Provider_EndExclUseAccount(pro, storedAccount, 1); /* abort */
      return rv;
    }

    return 0;
  }
}



int _addAccount(AB_PROVIDER *pro, AB_USER *user, AB_ACCOUNT *account)
{
  int rv;

  /* account is new, add it */
  DBG_ERROR(AQEBICS_LOGDOMAIN, "Account is new, adding");
  AB_Account_SetUserId(account, AB_User_GetUniqueId(user));
  rv=AB_Provider_AddAccount(pro, account, 0); /* do not lock corresponding user, it might already be locked */
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Coud not add new account (%d)", rv);
    return rv;
  }

  return 0;
}




