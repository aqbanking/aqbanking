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

#ifndef AO_ACCOUNT_P_H
#define AO_ACCOUNT_P_H

#include "account.h"

typedef struct AO_ACCOUNT AO_ACCOUNT;
struct AO_ACCOUNT {
  int maxPurposeLines;
  int debitAllowed;
};

static void GWENHYWFAR_CB AO_Account_freeData(void *bp, void *p);


#endif
