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


#ifndef AH_CONNECTION_HBCI_P_H
#define AH_CONNECTION_HBCI_P_H

#define AH_CONNECTION_HBCI_MSGBUFSIZE 512

#include <aqhbci/connectionhbci.h>


typedef struct AH_CONNECTION_HBCI AH_CONNECTION_HBCI;
struct AH_CONNECTION_HBCI {
  GWEN_NETMSG *currentInMsg;
  GWEN_NETMSG *currentOutMsg;
  int downAfterSend;
};


void AH_ConnectionHBCI_FreeData(void *bp, void *p);
GWEN_NETCONNECTION_WORKRESULT
  AH_ConnectionHBCI_Work(GWEN_NETCONNECTION *conn);

GWEN_NETCONNECTION_WORKRESULT
  AH_ConnectionHBCI_ReadWork(GWEN_NETCONNECTION *conn);
GWEN_NETCONNECTION_WORKRESULT
  AH_ConnectionHBCI_WriteWork(GWEN_NETCONNECTION *conn);


#endif /* AH_CONNECTION_HBCI_P_H */


