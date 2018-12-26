/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef EBC_CLIENT_ACCOUNT_P_H
#define EBC_CLIENT_ACCOUNT_P_H

#include "account_l.h"


typedef struct EBC_ACCOUNT EBC_ACCOUNT;
struct EBC_ACCOUNT {
  uint32_t flags;
  char *ebicsId;

  AB_ACCOUNT_READFROMDB_FN readFromDbFn;
  AB_ACCOUNT_WRITETODB_FN writeToDbFn;
};

static void GWENHYWFAR_CB EBC_Account_freeData(void *bp, void *p);

static int EBC_Account_ReadFromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);
static int EBC_Account_WriteToDb(const AB_ACCOUNT *a, GWEN_DB_NODE *db);



#endif /* EBC_CLIENT_ACCOUNT_P_H */

