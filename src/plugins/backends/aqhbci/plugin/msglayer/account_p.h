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


#ifndef AH_ACCOUNT_P_H
#define AH_ACCOUNT_P_H


#include "hbci_l.h"
#include "account_l.h"


typedef struct AH_ACCOUNT AH_ACCOUNT;
struct AH_ACCOUNT {
  AH_HBCI *hbci;
  GWEN_TYPE_UINT32 flags;
};

static void GWENHYWFAR_CB AH_Account_freeData(void *bp, void *p);


#endif /* AH_ACCOUNT_P_H */


