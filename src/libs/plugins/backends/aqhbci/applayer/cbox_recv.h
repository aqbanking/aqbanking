/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_CBOX_RECV_H
#define AH_OUTBOX_CBOX_RECV_H


#include "aqhbci/applayer/cbox.h"



AH_MSG *AH_OutboxCBox_RecvMessage(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, GWEN_DB_NODE *dbRsp);
int AH_OutboxCBox_RecvQueue(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq);



#endif

