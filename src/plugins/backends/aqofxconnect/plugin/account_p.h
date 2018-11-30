/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AO_ACCOUNT_P_H
#define AO_ACCOUNT_P_H

#include "account.h"

typedef struct AO_ACCOUNT AO_ACCOUNT;
struct AO_ACCOUNT {
  int maxPurposeLines;
  int debitAllowed;

  AB_ACCOUNT_READFROMDB_FN readFromDbFn;
  AB_ACCOUNT_WRITETODB_FN writeToDbFn;
};

static void GWENHYWFAR_CB AO_Account_freeData(void *bp, void *p);

static int AO_Account_ReadFromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);
static int AO_Account_WriteToDb(const AB_ACCOUNT *a, GWEN_DB_NODE *db);


#endif
