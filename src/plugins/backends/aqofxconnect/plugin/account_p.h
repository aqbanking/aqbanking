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

#include <aqofxconnect/account.h>


struct AO_ACCOUNT {
  int maxPurposeLines;
  int debitAllowed;
  char *userId;
};

void AO_Account_FreeData(void *bp, void *p);



#endif
