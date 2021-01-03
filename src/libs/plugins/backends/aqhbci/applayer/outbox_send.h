/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_SEND_H
#define AH_OUTBOX_SEND_H


#include "aqhbci/applayer/outbox_l.h"



int AH_Outbox__CBox_SendMessage(AH_OUTBOX__CBOX *cbox, AH_DIALOG *dlg, AH_MSG *msg);
int AH_Outbox__CBox_SendQueue(AH_OUTBOX__CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq);


#endif

