/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_CBOX_ITAN_H
#define AH_OUTBOX_CBOX_ITAN_H

#include "aqhbci/applayer/cbox.h"
#include "aqhbci/msglayer/message_l.h"
#include "aqhbci/joblayer/jobqueue_l.h"



int AH_OutboxCBox_JobToMessage(AH_JOB *j, AH_MSG *msg, int doCopySigners);

int AH_OutboxCBox_SelectItanMode(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg);
void AH_OutboxCBox_CopyJobResultsToJobList(const AH_JOB *j, const AH_JOB_LIST *qjl);

int AH_OutboxCBox_InputTanWithChallenge(AH_OUTBOX_CBOX *cbox,
                                        AH_DIALOG *dialog,
                                        const char *sChallenge,
                                        const char *sChallengeHhd,
                                        char *passwordBuffer,
                                        int passwordMinLen,
                                        int passwordMaxLen);




#endif

