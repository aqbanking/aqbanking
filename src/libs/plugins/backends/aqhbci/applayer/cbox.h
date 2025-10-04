/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_CBOX_H
#define AH_OUTBOX_CBOX_H


#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>


typedef struct AH_OUTBOX_CBOX AH_OUTBOX_CBOX;
GWEN_LIST_FUNCTION_DEFS(AH_OUTBOX_CBOX, AH_OutboxCBox);


#include "aqhbci/joblayer/jobqueue_l.h"
#include "aqhbci/applayer/outbox_l.h"
#include "aqhbci/applayer/cbox_itan.h"



AH_OUTBOX_CBOX *AH_OutboxCBox_new(AB_PROVIDER *pro, AB_USER *u, AH_OUTBOX *ob);
void AH_OutboxCBox_free(AH_OUTBOX_CBOX *cbox);

void AH_OutboxCBox_AddTodoJob(AH_OUTBOX_CBOX *cbox, AH_JOB *j);

AH_OUTBOX *AH_OutboxCBox_GetOutbox(const AH_OUTBOX_CBOX *cbox);
AB_PROVIDER *AH_OutboxCBox_GetProvider(const AH_OUTBOX_CBOX *cbox);
AB_USER *AH_OutboxCBox_GetUser(const AH_OUTBOX_CBOX *cbox);

AH_JOB_LIST *AH_OutboxCBox_GetTodoJobs(const AH_OUTBOX_CBOX *cbox);
AH_JOB_LIST *AH_OutboxCBox_GetFinishedJobs(const AH_OUTBOX_CBOX *cbox);
AH_JOB_LIST *AH_OutboxCBox_TakeFinishedJobs(AH_OUTBOX_CBOX *cbox);



#endif /* AH_OUTBOX_CBOX_P_H */





