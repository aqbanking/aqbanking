/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2014 by Martin Preuss
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

#define AH_OUTBOX_EXECUTE_WCB_ID "AH_OUTBOX_EXECUTE_WCB_ID"


typedef struct AH_OUTBOX AH_OUTBOX;

#include <aqbanking/imexporter.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/gwentime.h>

#include "hbci_l.h"
#include "job_l.h"
#include <aqhbci/user.h>


AH_OUTBOX *AH_Outbox_new(AH_HBCI *hbci);
void AH_Outbox_free(AH_OUTBOX *ob);
void AH_Outbox_Attach(AH_OUTBOX *ob);

void AH_Outbox_AddJob(AH_OUTBOX *ob, AH_JOB *j);
void AH_Outbox_AddPendingJob(AH_OUTBOX *ob, AB_JOB *bj);

/* makes all jobs commit their data */
void AH_Outbox_Commit(AH_OUTBOX *ob, int doLock);


/* makes all jobs process their data */
void AH_Outbox_Process(AH_OUTBOX *ob);

/* makes all jobs commit their system data (only calls
 * @ref AH_Job_DefaultCommitHandler which only commits system data
 * like account data, bank parameter data etc according to the flags in
 * @ref AH_HBCIClient).
 */
void AH_Outbox_CommitSystemData(AH_OUTBOX *ob, int doLock);


unsigned int AH_Outbox_CountTodoJobs(AH_OUTBOX *ob);
unsigned int AH_Outbox_CountFinishedJobs(AH_OUTBOX *ob);


int AH_Outbox_Execute(AH_OUTBOX *ob,
                      AB_IMEXPORTER_CONTEXT *ctx,
		      int withProgress, int nounmount, int doLock);


AH_JOB *AH_Outbox_FindTransferJob(AH_OUTBOX *ob,
                                  AB_USER *u,
                                  AB_ACCOUNT *a,
                                  const char *jobName);


AH_JOB_LIST *AH_Outbox_GetFinishedJobs(AH_OUTBOX *ob);


#endif /* AH_OUTBOX_L_H */

