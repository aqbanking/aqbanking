/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWHBCI_DIALOG_P_H
#define GWHBCI_DIALOG_P_H


#include "dialog_l.h"

#include <aqbanking/httpsession.h>

#include <gwenhywfar/inetaddr.h>


struct AH_DIALOG {
  uint32_t lastMsgNum;
  uint32_t lastReceivedMsgNum;
  char *dialogId;
  AB_USER *dialogOwner;

  GWEN_MSGENGINE *msgEngine;

  GWEN_SYNCIO *ioLayer;
  GWEN_HTTP_SESSION *httpSession;

  uint32_t flags;

  uint32_t usage;

  GWEN_DB_NODE *globalValues;
  char *logName;

  uint32_t itanMethod;
  int itanProcessType;
  int tanJobVersion;

  AH_TAN_METHOD *tanMethodDescription;
};


static int AH_Dialog_SendPacket(AH_DIALOG *dlg, const char *buf, int blen);


static int AH_Dialog_CreateIoLayer_Hbci(AH_DIALOG *dlg);
static int AH_Dialog_Connect_Hbci(AH_DIALOG *dlg);
static int AH_Dialog_Disconnect_Hbci(AH_DIALOG *dlg);
static int AH_Dialog_SendPacket_Hbci(AH_DIALOG *dlg,
				     const char *buf, int blen);
static int AH_Dialog_RecvMessage_Hbci(AH_DIALOG *dlg, AH_MSG **pMsg);

static int AH_Dialog_CreateIoLayer_Https(AH_DIALOG *dlg);
static int AH_Dialog_Connect_Https(AH_DIALOG *dlg);
static int AH_Dialog_Disconnect_Https(AH_DIALOG *dlg);
static int AH_Dialog_SendPacket_Https(AH_DIALOG *dlg,
				      const char *buf, int blen);
static int AH_Dialog_RecvMessage_Https(AH_DIALOG *dlg, AH_MSG **pMsg);






#endif /* GWHBCI_DIALOG_P_H */
