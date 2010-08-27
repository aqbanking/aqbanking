/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_ACCOUNT_P_H
#define AQBANKING_ACCOUNT_P_H


#include "account_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/db.h>


struct AB_ACCOUNT {
  GWEN_INHERIT_ELEMENT(AB_ACCOUNT)
  GWEN_LIST_ELEMENT(AB_ACCOUNT)
  uint32_t usage;

  uint32_t uniqueId;
  AB_ACCOUNT_TYPE accountType;

  AB_BANKING *banking;
  AB_PROVIDER *provider;
  char *backendName;

  char *subAccountId;
  char *accountNumber;
  char *bankCode;
  char *accountName;
  char *bankName;
  char *iban;
  char *bic;
  char *ownerName;
  char *currency;
  char *country;

  GWEN_STRINGLIST *userIds;
  GWEN_STRINGLIST *selectedUserIds;

  GWEN_DB_NODE *appData;
  GWEN_DB_NODE *providerData;

  char *dbId;
};

static AB_ACCOUNT *AB_Account__freeAll_cb(AB_ACCOUNT *a, void *userData);



#endif /* AQBANKING_ACCOUNT_P_H */
