/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_ITAN2_H
#define AH_OUTBOX_ITAN2_H


#include "aqhbci/applayer/itan.h"



int AH_Outbox__CBox_SendAndReceiveQueueWithTan2(AH_OUTBOX__CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *qJob);
int AH_Outbox__CBox_SendAndReceiveJobWithTan2(AH_OUTBOX__CBOX *cbox, AH_DIALOG *dlg, AH_JOB *job);


#endif

