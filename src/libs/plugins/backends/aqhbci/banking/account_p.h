/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_ACCOUNT_P_H
#define AH_ACCOUNT_P_H


#include "aqhbci/msglayer/hbci_l.h"
#include "aqhbci/banking/account_l.h"


typedef struct AH_ACCOUNT AH_ACCOUNT;
struct AH_ACCOUNT {
  AH_HBCI *hbci;
  uint32_t flags;
  GWEN_DB_NODE *dbTempUpd;

  AB_ACCOUNT_READFROMDB_FN readFromDbFn;
  AB_ACCOUNT_WRITETODB_FN writeToDbFn;
};

static void GWENHYWFAR_CB AH_Account_freeData(void *bp, void *p);


static int AH_Account_ReadFromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);
static int AH_Account_WriteToDb(const AB_ACCOUNT *a, GWEN_DB_NODE *db);


#endif /* AH_ACCOUNT_P_H */


