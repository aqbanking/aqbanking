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

#include <aqdtaus/account.h>


struct AD_ACCOUNT {
  int maxTransfersPerJob;
  int maxPurposeLines;
  int debitAllowed;
  int useDisc;
  int mountAllowed;
  int printAllTransactions;
  char *folder;
  char *mountCommand;
  char *unmountCommand;
  char *filePath;
  GWEN_TYPE_UINT32 lastVol;
};

void AD_Account_FreeData(void *bp, void *p);



#endif
