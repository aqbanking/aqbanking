/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_CBOX_SEND_H
#define AH_OUTBOX_CBOX_SEND_H


#include "aqhbci/applayer/cbox.h"



int AH_OutboxCBox_SendMessage(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_MSG *msg);
int AH_OutboxCBox_SendQueue(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq);


#endif

