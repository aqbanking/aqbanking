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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#define GWEN_EXTEND_NETCONNECTION

#include "connection_p.h"
#include "aqhbci_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(GWEN_NETCONNECTION, AH_CONNECTION);



void AH_Connection_Extend(AH_HBCI *hbci, GWEN_NETCONNECTION *conn) {
  AH_CONNECTION *hconn;

  assert(hbci);
  GWEN_NEW_OBJECT(AH_CONNECTION, hconn);
  hconn->hbci=hbci;
  GWEN_INHERIT_SETDATA(GWEN_NETCONNECTION, AH_CONNECTION,
                       conn, hconn,
                       AH_Connection_FreeData);
  GWEN_NetConnection_SetUpFn(conn, AH_Connection_Up);
  GWEN_NetConnection_SetDownFn(conn, AH_Connection_Down);
}



void AH_Connection_FreeData(void *bp, void *p){
  AH_CONNECTION *hconn;
  GWEN_NETCONNECTION *conn;

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Destroying AH_CONNECTION");
  conn=(GWEN_NETCONNECTION*)bp;
  assert(conn);
  hconn=(AH_CONNECTION*)p;
  assert(hconn);
  GWEN_FREE_OBJECT(hconn);
}



int AH_Connection_IsDown(const GWEN_NETCONNECTION *conn){
  AH_CONNECTION *hconn;

  assert(conn);
  hconn=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, AH_CONNECTION, conn);
  assert(hconn);

  return hconn->connectionDown;
}



void AH_Connection_SetIsDown(GWEN_NETCONNECTION *conn, int i){
  AH_CONNECTION *hconn;

  assert(conn);
  hconn=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, AH_CONNECTION, conn);
  assert(hconn);

  hconn->connectionDown=i;
}



void AH_Connection_Up(GWEN_NETCONNECTION *conn){
  AH_CONNECTION *hconn;
  const GWEN_INETADDRESS *peerAddr;

  assert(conn);
  hconn=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, AH_CONNECTION, conn);
  assert(hconn);

  peerAddr=GWEN_NetConnection_GetPeerAddr(conn);
  if (peerAddr) {
    char addrBuffer[128];
    char buffer[256];

    GWEN_InetAddr_GetAddress(peerAddr,
                             addrBuffer, sizeof(addrBuffer));
    buffer[0]=0;
    buffer[sizeof(buffer)-1]=0;
    snprintf(buffer, sizeof(buffer)-1,
             I18N("Connection established with %s (port %d)"),
             addrBuffer, GWEN_InetAddr_GetPort(peerAddr));
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Connection established with %s (port %d)",
               addrBuffer, GWEN_InetAddr_GetPort(peerAddr));
    AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(hconn->hbci),
                           0,
                           AB_Banking_LogLevelNotice,
                           buffer);
  }
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "HBCI connection up");
    AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(hconn->hbci),
                           0,
                           AB_Banking_LogLevelNotice,
                           I18N("Connection established"));
  }
}



void AH_Connection_Down(GWEN_NETCONNECTION *conn){
  AH_CONNECTION *hconn;
  const GWEN_INETADDRESS *peerAddr;

  assert(conn);
  hconn=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, AH_CONNECTION, conn);
  assert(hconn);

  peerAddr=GWEN_NetConnection_GetPeerAddr(conn);
  if (peerAddr) {
    char addrBuffer[128];
    char buffer[256];

    GWEN_InetAddr_GetAddress(peerAddr,
                             addrBuffer, sizeof(addrBuffer));
    buffer[0]=0;
    buffer[sizeof(buffer)-1]=0;
    snprintf(buffer, sizeof(buffer)-1,
             I18N("Connection closed to %s (port %d)"),
             addrBuffer, GWEN_InetAddr_GetPort(peerAddr));
    DBG_NOTICE(AQHBCI_LOGDOMAIN,
               "Connection closed to %s (port %d)",
               addrBuffer, GWEN_InetAddr_GetPort(peerAddr));
    AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(hconn->hbci),
                           0,
                           AB_Banking_LogLevelNotice,
                           buffer);
  }
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "HBCI connection down");
    AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(hconn->hbci),
                           0,
                           AB_Banking_LogLevelNotice,
                           I18N("Connection closed"));
  }
  hconn->connectionDown=1;
}












