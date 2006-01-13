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

#include "account_l.h"

typedef struct AD_ACCOUNT AD_ACCOUNT;
struct AD_ACCOUNT {
  int dummy;
};

void AD_Account_FreeData(void *bp, void *p);



#endif
