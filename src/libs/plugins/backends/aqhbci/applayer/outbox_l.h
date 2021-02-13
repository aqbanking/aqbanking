/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_L_H
#define AH_OUTBOX_L_H

#define AH_OUTBOX_FLAGS_ENDDIALOG 0x00000001

#define AH_OUTBOX_FLAGS_DEFAULT \
  (\
  AH_OUTBOX_FLAGS_ENDDIALOG \
  )


typedef struct AH_OUTBOX AH_OUTBOX;

#include "aqhbci/joblayer/job_l.h"

#include "aqbanking/backendsupport/imexporter.h"
#include "aqbanking/backendsupport/provider.h"



AH_OUTBOX *AH_Outbox_new(AB_PROVIDER *pro);
void AH_Outbox_free(AH_OUTBOX *ob);
void AH_Outbox_Attach(AH_OUTBOX *ob);

AB_IMEXPORTER_CONTEXT *AH_Outbox_GetImExContext(const AH_OUTBOX *outbox);


void AH_Outbox_AddJob(AH_OUTBOX *ob, AH_JOB *j);

/* makes all jobs process their data */
void AH_Outbox_Process(AH_OUTBOX *ob);

int AH_Outbox_Execute(AH_OUTBOX *ob,
                      AB_IMEXPORTER_CONTEXT *ctx,
                      int withProgress, int nounmount, int doLock);


AH_JOB *AH_Outbox_FindTransferJob(AH_OUTBOX *ob, AB_USER *u, AB_ACCOUNT *a, const char *jobName);


AH_JOB_LIST *AH_Outbox_GetFinishedJobs(AH_OUTBOX *ob);


#endif /* AH_OUTBOX_L_H */

