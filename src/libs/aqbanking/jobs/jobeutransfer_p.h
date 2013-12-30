/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004-2013 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBEUTRANSFER_P_H
#define AQBANKING_JOBEUTRANSFER_P_H


#include <aqbanking/job.h>
#include <aqbanking/transaction.h>
#include "jobeutransfer_l.h"

typedef struct AB_JOBEUTRANSFER AB_JOBEUTRANSFER;
struct AB_JOBEUTRANSFER {
  AB_TRANSACTION *transaction;
  AB_EUTRANSFER_INFO_LIST *countryInfoList;
  int ibanAllowed;
  AB_JOBEUTRANSFER_CHARGE_WHOM chargeWhom;
};
static void GWENHYWFAR_CB AB_JobEuTransfer_FreeData(void *bp, void *p);


#endif

