/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_ITAN_H
#define AH_OUTBOX_ITAN_H

#include "aqhbci/applayer/outbox_l.h"
#include "aqhbci/msglayer/message_l.h"
#include "aqhbci/joblayer/jobqueue_l.h"



int AH_Outbox__CBox__Hash(int mode, const uint8_t *p, unsigned int l, AH_MSG *msg);
int AH_Outbox__CBox_JobToMessage(AH_JOB *j, AH_MSG *msg, int doCopySigners);

int AH_Outbox__CBox_SendAndReceiveQueueWithTan(AH_OUTBOX__CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *qJob);

int AH_Outbox__CBox_SelectItanMode(AH_OUTBOX__CBOX *cbox, AH_DIALOG *dlg);
void AH_Outbox__CBox_CopyJobResultsToJobList(const AH_JOB *j, const AH_JOB_LIST *qjl);

int AH_Outbox__CBox_InputTanWithChallenge(AH_OUTBOX__CBOX *cbox,
                                          AH_DIALOG *dialog,
                                          const char *sChallenge,
                                          const char *sChallengeHhd,
                                          char *passwordBuffer,
                                          int passwordMinLen,
                                          int passwordMaxLen);




#endif

