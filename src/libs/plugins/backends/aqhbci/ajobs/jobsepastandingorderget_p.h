/***************************************************************************
 begin       : Wed Jan 15 2014
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPASTANDINGORDERGET_P_H
#define AH_JOBSEPASTANDINGORDERGET_P_H


#include "jobsepastandingorderget_l.h"


static int AH_Job_SepaStandingOrderGet_Prepare(AH_JOB *j);
static int AH_Job_SepaStandingOrdersGet_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);

static AB_TRANSACTION *_readSto(AH_JOB *j, const char *docType, const uint8_t *ptr, uint32_t len);

static AB_TRANSACTION_PERIOD _getPeriod(const char *s);
static AB_TRANSACTION *_readTransactionFromResponse(AH_JOB *j, GWEN_DB_NODE *dbXA);



#endif /* AH_JOBSEPASTANDINGORDERGET_L_H */


