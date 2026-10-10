/***************************************************************************
    begin       : Fri Oct 3 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_CBOX_VOPMSG_H
#define AH_OUTBOX_CBOX_VOPMSG_H


#include "aqhbci/applayer/cbox.h"
#include "aqhbci/admjobs/vop_result.h"



int AH_OutboxCBox_LetUserConfirmVopResult(AH_OUTBOX_CBOX *cbox, AH_JOB *workJob, AH_JOB *vppJob, const char *sMsg);
void AH_OutboxCBox_ApplyVopResultsToTransfers(AH_JOB *workJob, const AH_VOP_RESULT_LIST *vrList);



#endif

