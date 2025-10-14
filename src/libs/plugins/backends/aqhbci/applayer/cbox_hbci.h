/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_CBOX_HBCI_H
#define AH_OUTBOX_CBOX_HBCI_H


#include "aqhbci/applayer/cbox.h"


int AH_OutboxCBox_OpenDialog_Hbci(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, uint32_t jqFlags);

int AH_OutboxCBox_OpenDialogWithJob_Hbci(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *jDlgOpen);


#endif

