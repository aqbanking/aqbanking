/***************************************************************************
 begin       : Thu Dec 13 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/



int AB_Banking_SetAccountSpecAlias(AB_BANKING *ab, const AB_ACCOUNT_SPEC *as, const char *alias) {
  int rv;
  GWEN_DB_NODE *dbConfig=NULL;
  GWEN_DB_NODE *db;

  rv=AB_Banking_ReadNamedConfigGroup(ab, AB_CFG_GROUP_APPS, ab->appName, 1, 0, &dbConfig);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  db=GWEN_DB_GetGroup(dbConfig, GWEN_DB_FLAGS_DEFAULT, "banking/aliases");
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, alias, AB_AccountSpec_GetUniqueId(as));

  rv=AB_Banking_WriteNamedConfigGroup(ab, AB_CFG_GROUP_APPS, ab->appName, 0, 1, dbConfig);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbConfig);
    return rv;
  }
  GWEN_DB_Group_free(dbConfig);

  return 0;
}



AB_ACCOUNT_SPEC *AB_Banking_GetAccountSpecByAlias(AB_BANKING *ab, const char *alias){
  GWEN_DB_NODE *dbConfig=NULL;
  GWEN_DB_NODE *db;
  AB_ACCOUNT_SPEC *as=NULL;
  uint32_t aid;
  int rv;

  rv=AB_Banking_ReadNamedConfigGroup(ab, AB_CFG_GROUP_APPS, ab->appName, 1, 1, &dbConfig);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return NULL;
  }

  db=GWEN_DB_GetGroup(dbConfig, GWEN_DB_FLAGS_DEFAULT, "banking/aliases");
  aid=GWEN_DB_GetIntValue(db, alias, 0, 0);
  if (aid<1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid account id for alias \"%s\"", alias);
    GWEN_DB_Group_free(dbConfig);
    return NULL;
  }
  GWEN_DB_Group_free(dbConfig);

  rv=AB_Banking_GetAccountSpecByUniqueId(ab, aid, &as);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return NULL;
  }

  return as;
}



