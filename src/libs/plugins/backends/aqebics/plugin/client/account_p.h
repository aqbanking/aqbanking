/***************************************************************************
 $RCSfile: user_p.h,v $
                             -------------------
    cvs         : $Id: user_p.h,v 1.2 2006/01/13 13:59:59 cstim Exp $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
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
};

static void GWENHYWFAR_CB EBC_Account_freeData(void *bp, void *p);

static void EBC_Account_ReadDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);
static void EBC_Account_toDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);


#endif /* EBC_CLIENT_ACCOUNT_P_H */

