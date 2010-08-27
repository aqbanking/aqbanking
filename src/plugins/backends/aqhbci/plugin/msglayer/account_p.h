/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_ACCOUNT_P_H
#define AH_ACCOUNT_P_H


#include "hbci_l.h"
#include "account_l.h"


typedef struct AH_ACCOUNT AH_ACCOUNT;
struct AH_ACCOUNT {
  AH_HBCI *hbci;
  uint32_t flags;
};

static void GWENHYWFAR_CB AH_Account_freeData(void *bp, void *p);

static void AH_Account_ReadDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);
static void AH_Account_toDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);


#endif /* AH_ACCOUNT_P_H */


