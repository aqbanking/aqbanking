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

#ifndef AH_CUSTOMER_P_H
#define AH_CUSTOMER_P_H

#include "customer_l.h"



struct AH_CUSTOMER {
  GWEN_LIST_ELEMENT(AH_CUSTOMER);

  AH_USER *user;

  int ignoreUPD;

  char *customerId;
  char *fullName;

  char *systemId;

  int updVersion;
  GWEN_DB_NODE *upd;

  AH_BPD *bpd;

  GWEN_MSGENGINE *msgEngine;

  GWEN_TYPE_UINT32 usage;


  /* adjustable vars */
  int hbciVersion;

  int bankDoesntSign;
  int bankUsesSignSeq;

  int httpVMajor;
  int httpVMinor;
  char *httpUserAgent;
  int preferSingleTransfer;
  int preferSingleDebitNote;
  int keepAlive;

};



#endif /* AH_CUSTOMER_P_H */


