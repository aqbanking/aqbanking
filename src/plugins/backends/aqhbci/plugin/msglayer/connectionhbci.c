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

#include "connectionhbci_p.h"
#include "aqhbci_l.h"
#include <aqhbci/aqhbci.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(GWEN_NETCONNECTION, AH_CONNECTION_HBCI);



GWEN_NETCONNECTION *AH_ConnectionHBCI_new(GWEN_NETTRANSPORT *tr,
                                          int take,
                                          GWEN_TYPE_UINT32 libId){
  GWEN_NETCONNECTION *conn;
  AH_CONNECTION_HBCI *hconn;

  conn=GWEN_NetConnection_new(tr, take, libId);
  assert(conn);
  GWEN_NEW_OBJECT(AH_CONNECTION_HBCI, hconn);
  GWEN_INHERIT_SETDATA(GWEN_NETCONNECTION, AH_CONNECTION_HBCI,
                       conn, hconn,
                       AH_ConnectionHBCI_FreeData);
  GWEN_NetConnection_SetWorkFn(conn, AH_ConnectionHBCI_Work);

  return conn;
}




void AH_ConnectionHBCI_FreeData(void *bp, void *p){
  AH_CONNECTION_HBCI *hconn;
  GWEN_NETCONNECTION *conn;

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Destroying AH_CONNECTION_HBCI");
  conn=(GWEN_NETCONNECTION*)bp;
  assert(conn);
  hconn=(AH_CONNECTION_HBCI*)p;
  assert(hconn);
  GWEN_NetMsg_free(hconn->currentInMsg);
  GWEN_NetMsg_free(hconn->currentOutMsg);
  GWEN_FREE_OBJECT(hconn);
}



GWEN_NETCONNECTION_WORKRESULT
AH_ConnectionHBCI_ReadWork(GWEN_NETCONNECTION *conn){
  AH_CONNECTION_HBCI *hconn;
  GWEN_BUFFER *mbuf;
  GWEN_TYPE_UINT32 i, j, k, l;
  GWEN_RINGBUFFER *rbuf;
  const char *p;
  int size;

  assert(conn);
  hconn=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, AH_CONNECTION_HBCI, conn);
  assert(hconn);

  if (!hconn->currentInMsg)
    hconn->currentInMsg=GWEN_NetMsg_new(AH_CONNECTION_HBCI_MSGBUFSIZE);

  mbuf=GWEN_NetMsg_GetBuffer(hconn->currentInMsg);
  rbuf=GWEN_NetConnection_GetReadBuffer(conn);

  /* complete header if necessary */
  k=30;
  while((i=GWEN_Buffer_GetUsedBytes(mbuf))<k) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Reading header...");
    j=GWEN_RingBuffer_GetMaxUnsegmentedRead(rbuf);
    if (!j) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Readbuffer empty");
      return GWEN_NetConnectionWorkResult_NoChange;
    }
    l=k-i;
    if (l>j)
      l=j;
    GWEN_Buffer_AppendBytes(mbuf, GWEN_RingBuffer_GetReadPointer(rbuf), l);
    GWEN_RingBuffer_SkipBytesRead(rbuf, l);
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Read %d bytes of the message", l);
  } /* while */

  /* size of complete message already known ? */
  k=GWEN_NetMsg_GetSize(hconn->currentInMsg);
  if (!k) {
    /* no, get it */
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Trying to determine message size");
    i=GWEN_Buffer_GetUsedBytes(mbuf);
    p=GWEN_Buffer_GetStart(mbuf);
    /* try to find the first occurrence of "+" */
    for (j=0; j<i; j++) {
      if (p[j]=='+')
        break;
    } /* for */
    if (j>29) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad message, disconnecting");
      GWEN_NetConnection_StartDisconnect(conn);
      return GWEN_NetConnectionWorkResult_Error;
    }
    j++;
    /* pseudo-append a NULL to buffer for sscanf */
    GWEN_Buffer_AllocRoom(mbuf, 1);
    GWEN_Buffer_GetPosPointer(mbuf)[0]=0;
    if (sscanf(p+j, "%d", &size)!=1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad message, disconnecting");
      GWEN_NetConnection_StartDisconnect(conn);
      return GWEN_NetConnectionWorkResult_Error;
    }
    GWEN_NetMsg_SetSize(hconn->currentInMsg, size);
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Size of incoming message is %d bytes", size);
    k=size;
  }

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Rest of message is %d", k);

  /* read rest of the message */
  while((i=GWEN_Buffer_GetUsedBytes(mbuf))<k) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Reading rest of the message...");
    j=GWEN_RingBuffer_GetMaxUnsegmentedRead(rbuf);
    if (!j) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Readbuffer empty");
      return GWEN_NetConnectionWorkResult_NoChange;
    }
    l=k-i;
    if (l>j)
      l=j;
    GWEN_Buffer_AppendBytes(mbuf, GWEN_RingBuffer_GetReadPointer(rbuf), l);
    GWEN_RingBuffer_SkipBytesRead(rbuf, l);
  } /* while */

  /* append message to connection's queue */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Got a message");
  GWEN_NetConnection_AddInMsg(conn, hconn->currentInMsg);
  hconn->currentInMsg=0;
  return GWEN_NetConnectionWorkResult_Change;
}



GWEN_NETCONNECTION_WORKRESULT
AH_ConnectionHBCI_WriteWork(GWEN_NETCONNECTION *conn){
  AH_CONNECTION_HBCI *hconn;
  GWEN_BUFFER *mbuf;
  GWEN_TYPE_UINT32 i, j;
  GWEN_RINGBUFFER *wbuf;

  assert(conn);
  hconn=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, AH_CONNECTION_HBCI, conn);
  assert(hconn);

  /* check whether we are currently working on a message */
  if (!hconn->currentOutMsg) {
    /* no, we aren't, get the next from the queue */
    hconn->currentOutMsg=GWEN_NetConnection_GetOutMsg(conn);
    /* still no message in work ? */
    if (!hconn->currentOutMsg) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Nothing to write");
      if (hconn->downAfterSend) {
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Closing connections as instructed");
        if (GWEN_NetConnection_StartDisconnect(conn)) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Could not start to disconnect");
          return GWEN_NetConnectionWorkResult_Error;
        }
        return GWEN_NetConnectionWorkResult_Change;
      }
      return GWEN_NetConnectionWorkResult_NoChange;
    }
    mbuf=GWEN_NetMsg_GetBuffer(hconn->currentOutMsg);
    GWEN_Buffer_Rewind(mbuf);
  }
  else
    mbuf=GWEN_NetMsg_GetBuffer(hconn->currentOutMsg);

  wbuf=GWEN_NetConnection_GetWriteBuffer(conn);

  /* complete header if necessary */
  while((i=GWEN_Buffer_GetBytesLeft(mbuf))) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Writing message...");
    j=GWEN_RingBuffer_GetMaxUnsegmentedWrite(wbuf);
    if (!j) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Writebuffer full");
      return GWEN_NetConnectionWorkResult_NoChange;
    }
    if (j>i)
      j=i;

    memmove(GWEN_RingBuffer_GetWritePointer(wbuf),
            GWEN_Buffer_GetPosPointer(mbuf),
            j);
    GWEN_RingBuffer_SkipBytesWrite(wbuf, j);
    GWEN_Buffer_IncrementPos(mbuf, j);
  } /* while */

  /* free message */
  GWEN_NetMsg_free(hconn->currentOutMsg);
  hconn->currentOutMsg=0;

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message written");
  return GWEN_NetConnectionWorkResult_Change;
}



GWEN_NETCONNECTION_WORKRESULT
AH_ConnectionHBCI_Work(GWEN_NETCONNECTION *conn){
  GWEN_NETCONNECTION_WORKRESULT rv1;
  GWEN_NETCONNECTION_WORKRESULT rv2;
  GWEN_NETCONNECTION_WORKRESULT rv3;
  int changes;
  AH_CONNECTION_HBCI *hconn;

  assert(conn);
  hconn=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, AH_CONNECTION_HBCI, conn);
  assert(hconn);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Working on HBCI connection");

  changes=0;

  while(1) {
    while(1) {
      int lchanges=0;

      /* do all write work */
      while(1) {
	rv1=AH_ConnectionHBCI_WriteWork(conn);
        if (rv1==GWEN_NetConnectionWorkResult_Change) {
	  /* FIXME: Should this really be GWEN_LOGDOMAIN? Not AQHBCI_LOGDOMAIN? */
          DBG_NOTICE(GWEN_LOGDOMAIN, "Change while writing");
          lchanges++;
        }
        else if (rv1==GWEN_NetConnectionWorkResult_Error) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Error on writing");
          return rv1;
        }
        else {
          DBG_DEBUG(GWEN_LOGDOMAIN, "No change while writing");
          break;
        }
      }

      /* do all read work */
      while(1) {
	rv3=AH_ConnectionHBCI_ReadWork(conn);
        if (rv3==GWEN_NetConnectionWorkResult_Change) {
          DBG_NOTICE(GWEN_LOGDOMAIN, "Change while reading");
          lchanges++;
        }
        else if (rv3==GWEN_NetConnectionWorkResult_Error) {
          DBG_ERROR(GWEN_LOGDOMAIN, "Error on reading");
          return rv3;
        }
        else {
          DBG_DEBUG(GWEN_LOGDOMAIN, "No change while reading");
          break;
        }
      }
      changes+=lchanges;
      if (!lchanges)
        break;
    } /* while */

    if (changes) {
      return GWEN_NetConnectionWorkResult_Change;
    }

    rv2=GWEN_NetConnection_WorkIO(conn);
    if (rv2==GWEN_NetConnectionWorkResult_Change) {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Change on WorkIO");
      changes++;
    }
    else if (rv2==GWEN_NetConnectionWorkResult_Error) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error on WorkIO");
      return rv2;
    }
    else if (rv2==GWEN_NetConnectionWorkResult_NoChange) {
      break;
    }
  } /* while */

  if (changes) {
    DBG_DEBUG(GWEN_LOGDOMAIN, "There were some changes (%d)", changes);
    return GWEN_NetConnectionWorkResult_Change;
  }

  DBG_DEBUG(GWEN_LOGDOMAIN, "There were NO changes");
  return GWEN_NetConnectionWorkResult_NoChange;
}



void AH_ConnectionHBCI_SetDownAfterSend(GWEN_NETCONNECTION *conn, int i){
  AH_CONNECTION_HBCI *hconn;

  assert(conn);
  hconn=GWEN_INHERIT_GETDATA(GWEN_NETCONNECTION, AH_CONNECTION_HBCI, conn);
  assert(hconn);

  hconn->downAfterSend=i;
}






