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



struct AH_ACCOUNT {
  GWEN_LIST_ELEMENT(AH_ACCOUNT);
  GWEN_INHERIT_ELEMENT(AH_ACCOUNT);

  GWEN_TYPE_UINT32 usage;

  AH_BANK *bank;

  char *bankId;
  char *accountId;
  char *accountName;
  char *ownerName;

  char *suffix;

  GWEN_STRINGLIST *customers;

};


AH_ACCOUNT *AH_Account__freeAll_cb(AH_ACCOUNT *a, void *userData);


#endif /* AH_ACCOUNT_P_H */


