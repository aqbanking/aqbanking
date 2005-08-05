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

#ifndef AH_USER_P_H
#define AH_USER_P_H

#include "user_l.h"
#include "customer_l.h"
#include "bpd.h"


struct AH_USER {
  GWEN_LIST_ELEMENT(AH_USER);

  AH_BANK *bank;
  char *userId;

  AH_MEDIUM *medium;
  int contextIdx;

  GWEN_TYPE_UINT32 usage;

  AH_CUSTOMER_LIST *customers;

  /* adjustable vars */
  AH_USER_STATUS status;
  /* only from here the server addres is taken !! */
  AH_BPD_ADDR *serverAddress;

  AH_CRYPT_MODE cryptMode;

  /* new variables used with CryptTokens */
  char *peerId;


};



#endif /* AH_USER_P_H */


