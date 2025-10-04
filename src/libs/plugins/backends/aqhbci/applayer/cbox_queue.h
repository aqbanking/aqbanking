/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_CBOX_QUEUE_H
#define AH_OUTBOX_CBOX_QUEUE_H


#include "aqhbci/applayer/cbox.h"

int AH_OutboxCBox_SendAndRecvBox(AH_OUTBOX_CBOX *cbox);

int AH_OutboxCBox_SendAndReceiveJob(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *j);
int AH_OutboxCBox_SendAndReceiveJobNoTan(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *j);

int AH_OutboxCBox_SendAndRecvQueue(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq);
int AH_OutboxCBox_SendAndRecvQueueNoTan(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq);
void AH_OutboxCBox_Finish(AH_OUTBOX_CBOX *cbox);


#endif

