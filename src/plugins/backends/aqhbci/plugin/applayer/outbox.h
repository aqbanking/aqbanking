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

#ifndef AH_OUTBOX_H
#define AH_OUTBOX_H


#define AH_OUTBOX_FLAGS_ENDDIALOG 0x00000001

#define AH_OUTBOX_FLAGS_DEFAULT \
  (\
  AH_OUTBOX_FLAGS_ENDDIALOG \
  )

#define AH_OUTBOX_EXECUTE_WCB_ID "AH_OUTBOX_EXECUTE_WCB_ID"

//#define AH_OUTBOX_EXECUTE_SEND_TIMEOUT 30
//#define AH_OUTBOX_EXECUTE_RECV_TIMEOUT 30


#ifdef __cplusplus
extern "C" {
#endif
typedef struct AH_OUTBOX AH_OUTBOX;
#ifdef __cplusplus
}
#endif

#include <gwenhywfar/inherit.h>
#include <aqhbci/hbci.h>
#include <aqhbci/customer.h>
#include <aqhbci/job.h>


#ifdef __cplusplus
extern "C" {
#endif


AH_OUTBOX *AH_Outbox_new(AH_HBCI *hbci);
void AH_Outbox_free(AH_OUTBOX *ob);
void AH_Outbox_Attach(AH_OUTBOX *ob);

void AH_Outbox_AddJob(AH_OUTBOX *ob, AH_JOB *j);
void AH_Outbox_AddPendingJob(AH_OUTBOX *ob, AB_JOB *bj);

/* makes all jobs commit their data */
void AH_Outbox_Commit(AH_OUTBOX *ob);


/* makes all jobs process their data */
void AH_Outbox_Process(AH_OUTBOX *ob);

/* makes all jobs commit their system data (only calls
 * @ref AH_Job_DefaultCommitHandler which only commits system data
 * like account data, bank parameter data etc according to the flags in
 * @ref AH_HBCIClient).
 */
void AH_Outbox_CommitSystemData(AH_OUTBOX *ob);


unsigned int AH_Outbox_CountTodoJobs(AH_OUTBOX *ob);
unsigned int AH_Outbox_CountFinishedJobs(AH_OUTBOX *ob);


int AH_Outbox_Execute(AH_OUTBOX *ob, int withProgress, int nounmount);


AH_JOB *AH_Outbox_FindTransferJob(AH_OUTBOX *ob,
                                  AH_CUSTOMER *cu,
                                  AH_ACCOUNT *a,
                                  int isTransfer);

#ifdef __cplusplus
}
#endif


#endif /* AH_OUTBOX_H */

