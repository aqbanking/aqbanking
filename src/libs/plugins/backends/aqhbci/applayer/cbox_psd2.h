/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_CBOX_PSD2_H
#define AH_OUTBOX_CBOX_PSD2_H


#include "aqhbci/applayer/cbox.h"


int AH_OutboxCBox_OpenDialogPsd2_Proc2(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg);

int AH_OutboxCBox_OpenDialogPsd2WithJob_Proc2(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *jDlgOpen);


#endif

