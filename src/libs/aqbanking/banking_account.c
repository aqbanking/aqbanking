/***************************************************************************
 begin       : Wed Nov 28 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


/* This file is included by banking.c */


int AB_Banking_Read_AccountConfig(const AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE **pDb)
{
  int rv;

  rv=AB_Banking_ReadConfigGroup(ab, AB_CFG_GROUP_ACCOUNTS, uid, doLock, doUnlock, pDb);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}



int AB_Banking_Write_AccountConfig(AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE *db)
{
  int rv;

  rv=AB_Banking_WriteConfigGroup(ab, AB_CFG_GROUP_ACCOUNTS, uid, doLock, doUnlock, db);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_Delete_AccountConfig(AB_BANKING *ab, uint32_t uid)
{
  int rv;

  rv=AB_Banking_DeleteConfigGroup(ab, AB_CFG_GROUP_ACCOUNTS, uid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_Unlock_AccountConfig(AB_BANKING *ab, uint32_t uid)
{
  int rv;

  rv=AB_Banking_UnlockConfigGroup(ab, AB_CFG_GROUP_ACCOUNTS, uid);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}




