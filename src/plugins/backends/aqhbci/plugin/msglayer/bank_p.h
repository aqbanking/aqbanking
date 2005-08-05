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


#ifndef AH_BANK_P_H
#define AH_BANK_P_H

#include "bank_l.h"
#include "account_l.h"
#include "user_l.h"


struct AH_BANK {
  GWEN_LIST_ELEMENT(AH_BANK);
  GWEN_INHERIT_ELEMENT(AH_BANK);

  GWEN_TYPE_UINT32 usage;

  AH_HBCI *hbci;
  int country;
  char *bankId;
  char *bankName;

  AH_USER_LIST *users;
  AH_ACCOUNT_LIST *accounts;

  GWEN_MSGENGINE *msgEngine;

};



#endif /* AH_BANK_P_H */


