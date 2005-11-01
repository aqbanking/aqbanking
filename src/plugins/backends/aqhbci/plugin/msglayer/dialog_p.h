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

#include <gwenhywfar/inetaddr.h>
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

  GWEN_NETLAYER *netLayer;

  GWEN_TYPE_UINT32 flags;

  GWEN_TYPE_UINT32 usage;

  GWEN_DB_NODE *globalValues;
  char *logName;
};



int AH_Dialog__SetAddress(AH_DIALOG *dlg,
                          GWEN_INETADDRESS *addr,
                          const char *bankAddr);
int AH_Dialog__CreateNetLayer(AH_DIALOG *dlg);





#endif /* GWHBCI_DIALOG_P_H */
