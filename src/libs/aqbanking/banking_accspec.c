/***************************************************************************
 begin       : Sat Jun 30 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static const char *_nonEmptyString(const char *s, const char *altstring);
static void _logAccountSpec(const AB_ACCOUNT_SPEC *a, const char *logMessage);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */




int AB_Banking_ReadAccountSpec(const AB_BANKING *ab, uint32_t uniqueId, AB_ACCOUNT_SPEC **pAccountSpec)
{
  AB_ACCOUNT_SPEC *accountSpec;
  GWEN_DB_NODE *db=NULL;
  int rv;

  assert(ab);

  rv=AB_Banking_ReadConfigGroup(ab, AB_CFG_GROUP_ACCOUNTSPECS, uniqueId, 1, 1, &db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  accountSpec=AB_AccountSpec_new();
  AB_AccountSpec_ReadDb(accountSpec, db);
  AB_AccountSpec_SetUniqueId(accountSpec, uniqueId);

  if (1) {
    int i;

    i=AB_AccountSpec_GetType(accountSpec);
    if (i==AB_AccountType_Unknown)
      AB_AccountSpec_SetType(accountSpec, AB_AccountType_Unspecified);
  }


  GWEN_DB_Group_free(db);

  if (pAccountSpec)
    *pAccountSpec=accountSpec;
  else
    AB_AccountSpec_free(accountSpec);
  return 0;
}




int AB_Banking_WriteAccountSpec(AB_BANKING *ab, const AB_ACCOUNT_SPEC *accountSpec)
{
  GWEN_DB_NODE *db=NULL;
  int rv;
  uint32_t uniqueId;

  assert(ab);

  uniqueId=AB_AccountSpec_GetUniqueId(accountSpec);

  /* write account spec to DB */
  db=GWEN_DB_Group_new("accountSpec");
  AB_AccountSpec_toDb(accountSpec, db);

  rv=AB_Banking_WriteConfigGroup(ab, AB_CFG_GROUP_ACCOUNTSPECS, uniqueId, 1, 1, db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }
  GWEN_DB_Group_free(db);

  return 0;
}



int AB_Banking_DeleteAccountSpec(AB_BANKING *ab, uint32_t uid)
{
  int rv;

  rv=AB_Banking_DeleteConfigGroup(ab, AB_CFG_GROUP_ACCOUNTSPECS, uid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}






int AB_Banking_GetAccountSpecList(const AB_BANKING *ab, AB_ACCOUNT_SPEC_LIST **pAccountSpecList)
{
  GWEN_DB_NODE *dbAll=NULL;
  int rv;

  DBG_INFO(AQBANKING_LOGDOMAIN, "Reading account spec list");

  rv=AB_Banking_ReadConfigGroups(ab, AB_CFG_GROUP_ACCOUNTSPECS, "uniqueId", NULL, NULL, &dbAll);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else {
    AB_ACCOUNT_SPEC_LIST *accountSpecList;
    GWEN_DB_NODE *db;

    accountSpecList=AB_AccountSpec_List_new();

    db=GWEN_DB_GetFirstGroup(dbAll);
    while (db) {
      AB_ACCOUNT_SPEC *a=NULL;

      assert(db);
      a=AB_AccountSpec_fromDb(db);
      if (a) {
        _logAccountSpec(a, "Adding account spec");
        if (1) {
          int i;

          i=AB_AccountSpec_GetType(a);
          if (i==AB_AccountType_Unknown)
            AB_AccountSpec_SetType(a, AB_AccountType_Unspecified);
        }
        AB_AccountSpec_List_Add(a, accountSpecList);
      }

      db=GWEN_DB_GetNextGroup(db);
    }

    if (AB_AccountSpec_List_GetCount(accountSpecList)) {
      *pAccountSpecList=accountSpecList;
      GWEN_DB_Group_free(dbAll);
      return 0;
    }
    else {
      DBG_WARN(AQBANKING_LOGDOMAIN, "No valid account specs found");
      AB_AccountSpec_List_free(accountSpecList);
      GWEN_DB_Group_free(dbAll);
      return GWEN_ERROR_NOT_FOUND;
    }
  }
}



int AB_Banking_GetAccountSpecByUniqueId(const AB_BANKING *ab, uint32_t uniqueAccountId, AB_ACCOUNT_SPEC **pAccountSpec)
{
  int rv;

  rv=AB_Banking_ReadAccountSpec(ab, uniqueAccountId, pAccountSpec);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}



void _logAccountSpec(const AB_ACCOUNT_SPEC *a, const char *logMessage)
{
  const char *sBankCode;
  const char *sAccountNumber;
  const char *sIban;
  const char *sBic;

  sBankCode=_nonEmptyString(AB_AccountSpec_GetBankCode(a), "<empty>");
  sAccountNumber=_nonEmptyString(AB_AccountSpec_GetAccountNumber(a), "<empty>");
  sIban=_nonEmptyString(AB_AccountSpec_GetIban(a), "<empty>");
  sBic=_nonEmptyString(AB_AccountSpec_GetBic(a), "<empty>");

  if (logMessage && *logMessage) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "%s (id=%u, bank code=%s, acc num=%s, bic=%s, iban=%s)",
             logMessage,
             AB_AccountSpec_GetUniqueId(a),
             sBankCode, sAccountNumber, sBic, sIban);
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Account spec (id=%u, bank code=%s, acc num=%s, bic=%s, iban=%s)",
             AB_AccountSpec_GetUniqueId(a),
             sBankCode, sAccountNumber, sBic, sIban);
  }
}


const char *_nonEmptyString(const char *s, const char *altstring)
{
  if (s && *s)
    return s;
  return altstring;
}









