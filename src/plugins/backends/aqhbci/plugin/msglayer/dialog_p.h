/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWHBCI_DIALOG_P_H
#define GWHBCI_DIALOG_P_H

#include <aqhbci/dialog.h>


struct AH_DIALOG {
  GWEN_LIST_ELEMENT(AH_DIALOG);
  GWEN_INHERIT_ELEMENT(AH_DIALOG);

  AH_BANK *bank;
  GWEN_TYPE_UINT32 lastMsgNum;
  GWEN_TYPE_UINT32 lastReceivedMsgNum;
  char *dialogId;
  AH_CUSTOMER *dialogOwner;

  GWEN_MSGENGINE *msgEngine;

  GWEN_NETCONNECTION *connection;

  GWEN_TYPE_UINT32 flags;

  GWEN_TYPE_UINT32 usage;

  GWEN_DB_NODE *globalValues;
  char *logName;
};



int AH_Dialog__SendMessageSSL(AH_DIALOG *dlg, AH_MSG *msg);
int AH_Dialog__SendMessageHBCI(AH_DIALOG *dlg, AH_MSG *msg);

AH_MSG *AH_Dialog__MsgFromNetMsg_SSL(AH_DIALOG *dlg, GWEN_NETMSG *netmsg);
AH_MSG *AH_Dialog__MsgFromNetMsg_HBCI(AH_DIALOG *dlg, GWEN_NETMSG *netmsg);
AH_MSG *AH_Dialog__MsgFromNetMsg(AH_DIALOG *dlg, GWEN_NETMSG *netmsg);




#endif /* GWHBCI_DIALOG_P_H */
