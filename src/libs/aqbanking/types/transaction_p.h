/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Apr 05 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_TRANSACTION_P_H
#define AQBANKING_TRANSACTION_P_H

#include "transaction_l.h"
#include <gwenhywfar/db.h>
#include <gwenhywfar/misc.h>



struct AB_TRANSACTION {
  GWEN_LIST_ELEMENT(AB_TRANSACTION)

  int localCountryCode;
  char *localBankCode;
  char *localAccountId;
  char *localSuffix;
  char *localOwnerName;

  int remoteCountryCode;
  char *remoteBankCode;
  char *remoteAccountId;
  char *remoteSuffix;
  GWEN_STRINGLIST *remoteOwnerName;

  GWEN_TIME *valutaDate;
  GWEN_TIME *date;

  AB_VALUE *value;

  int textKey;
  char *transactionKey;
  char *customerReference;
  char *bankReference;
  int transactionCode;
  char *transactionText;
  char *primanota;

  GWEN_STRINGLIST *purpose;

};

AB_TRANSACTION *AB_Transaction__freeAll_cb(AB_TRANSACTION *t, void *userData);






#endif /* AQBANKING_TRANSACTION_P_H */


