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

#ifndef AD_JOB_H
#define AD_JOB_H

#include "provider_l.h"
#include "account_l.h"
#include <aqbanking/account_be.h>
#include <aqbanking/transaction.h>
#include <gwenhywfar/misc.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct AD_JOB AD_JOB;
GWEN_LIST_FUNCTION_DEFS(AD_JOB, AD_Job)

AD_JOB *AD_Job_new(AB_ACCOUNT *acc, int isDebitJob, uint32_t jid);
void AD_Job_free(AD_JOB *dj);

void AD_Job_AddTransfer(AD_JOB *dj, AB_TRANSACTION *t);
int AD_Job_GetTransferCount(const AD_JOB *dj);
AB_TRANSACTION_LIST2 *AD_Job_GetTransfers(const AD_JOB *dj);

int AD_Job_GetIsDebitNote(const AD_JOB *dj);

AB_ACCOUNT *AD_Job_GetAccount(const AD_JOB *dj);
uint32_t AD_Job_GetJobId(const AD_JOB *dj);

void AD_Job_SetResult(AD_JOB *dj, int code, const char *text);
int AD_Job_GetResultCode(const AD_JOB *dj);
const char *AD_Job_GetResultText(const AD_JOB *dj);

#ifdef __cplusplus
}
#endif


#endif
