/***************************************************************************
    begin       : Fri Oct 3 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_CBOX_VOP_H
#define AH_OUTBOX_CBOX_VOP_H


#include "aqhbci/applayer/cbox.h"



int AH_OutboxCBox_SendAndReceiveJobWithTanAndVpp(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *transactionJob);



#endif

