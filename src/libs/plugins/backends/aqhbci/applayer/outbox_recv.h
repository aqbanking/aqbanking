/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_RECV_H
#define AH_OUTBOX_RECV_H


#include "aqhbci/applayer/outbox_l.h"



AH_MSG *AH_Outbox__CBox_RecvMessage(AH_OUTBOX__CBOX *cbox, AH_DIALOG *dlg, GWEN_DB_NODE *dbRsp);
int AH_Outbox__CBox_RecvQueue(AH_OUTBOX__CBOX *cbox, AH_DIALOG *dlg, AH_JOBQUEUE *jq);



#endif

