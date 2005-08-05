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

/* #define AH_DIALOG_HEAVY_DEBUG */


#include "dialog_p.h"
#include "aqhbci_l.h"
#include "hbci_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/netconnectionhttp.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/text.h>
#include <aqhbci/msgengine.h>

#include <string.h>
#include <ctype.h>
#include <errno.h>


GWEN_LIST_FUNCTIONS(AH_DIALOG, AH_Dialog);
GWEN_INHERIT_FUNCTIONS(AH_DIALOG);

#ifdef OS_WIN32
# define AH_PATH_SEP "\\"
#else
# define AH_PATH_SEP "/"
#endif


AH_DIALOG *AH_Dialog_new(AH_CUSTOMER *cu,
                         GWEN_NETCONNECTION *conn) {
  AH_DIALOG *dlg;
  AH_HBCI *h;
  AH_BANK *b;
  GWEN_BUFFER *pbuf;

  assert(cu);
  assert(conn);
  b=AH_User_GetBank(AH_Customer_GetUser(cu));
  h=AH_Bank_GetHbci(b);

  GWEN_NEW_OBJECT(AH_DIALOG, dlg);
  dlg->usage=1;
  GWEN_LIST_INIT(AH_DIALOG, dlg);
  GWEN_INHERIT_INIT(AH_DIALOG, dlg);
  dlg->bank=b;
  dlg->globalValues=GWEN_DB_Group_new("globalValues");

  AH_Bank_Attach(b);
  dlg->connection=conn;
  dlg->dialogId=strdup("0");
  GWEN_NetConnection_Attach(conn);

  dlg->msgEngine=AH_Customer_GetMsgEngine(cu);
  GWEN_MsgEngine_Attach(dlg->msgEngine);
  dlg->dialogOwner=cu;
  AH_Customer_Attach(cu);
  AH_MsgEngine_SetCustomer(dlg->msgEngine, cu);

  /* create path */
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (AH_HBCI_AddBankPath(h, b, pbuf)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Could not add bank path, cannot log");
    GWEN_Buffer_free(pbuf);
  }
  else {
    GWEN_Buffer_AppendString(pbuf, AH_PATH_SEP "logs" AH_PATH_SEP);
    AH_HBCI_AppendUniqueName(h, pbuf);
    GWEN_Buffer_AppendString(pbuf, ".log");
    dlg->logName=strdup(GWEN_Buffer_GetStart(pbuf));
  }
  GWEN_Buffer_free(pbuf);

  return dlg;
}



void AH_Dialog_Attach(AH_DIALOG *dlg){
  assert(dlg);
  dlg->usage++;
}


void AH_Dialog_free(AH_DIALOG *dlg){
  if (dlg) {
    assert(dlg->usage);
    if (--(dlg->usage)==0) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Destroying AH_DIALOG");
      GWEN_LIST_FINI(AH_DIALOG, dlg);
      GWEN_INHERIT_FINI(AH_DIALOG, dlg);
      GWEN_NetConnection_free(dlg->connection);
      AH_Bank_free(dlg->bank);
      free(dlg->dialogId);
      free(dlg->logName);
      GWEN_MsgEngine_free(dlg->msgEngine);
      GWEN_DB_Group_free(dlg->globalValues);
      AH_Customer_free(dlg->dialogOwner);

      free(dlg);
    }
  }
}



const char *AH_Dialog_GetLogFile(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->logName;
}



AH_BANK *AH_Dialog_GetBank(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->bank;
}



GWEN_TYPE_UINT32 AH_Dialog_GetNextMsgNum(AH_DIALOG *dlg){
  assert(dlg);
  return ++dlg->lastMsgNum;
}



GWEN_TYPE_UINT32
AH_Dialog_GetLastReceivedMsgNum(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->lastReceivedMsgNum;
}



const char *AH_Dialog_GetDialogId(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->dialogId;
}



void AH_Dialog_SetDialogId(AH_DIALOG *dlg, const char *s){
  assert(dlg);
  free(dlg->dialogId);
  if (s) dlg->dialogId=strdup(s);
  else dlg->dialogId=0;
}



AH_CUSTOMER *AH_Dialog_GetDialogOwner(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->dialogOwner;
}



GWEN_NETCONNECTION *AH_Dialog_GetConnection(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->connection;
}



GWEN_MSGENGINE *AH_Dialog_GetMsgEngine(const AH_DIALOG *dlg){
  assert(dlg);
  assert(dlg->msgEngine);
  return dlg->msgEngine;
}



int AH_Dialog_CheckReceivedMsgNum(AH_DIALOG *dlg,
                                  GWEN_TYPE_UINT32 msgnum){
  assert(dlg);
  if (msgnum!=dlg->lastReceivedMsgNum+1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Continuity error in received message "
              "(expected %d, got %d)",
              dlg->lastReceivedMsgNum+1,
              msgnum);
    return -1;
  }
  dlg->lastReceivedMsgNum++;
  return 0;
}



GWEN_TYPE_UINT32 AH_Dialog_GetFlags(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->flags;
}



void AH_Dialog_SetFlags(AH_DIALOG *dlg, GWEN_TYPE_UINT32 f){
  assert(dlg);
  dlg->flags=f;
}



AH_MSG *AH_Dialog__MsgFromNetMsg_HBCI(AH_DIALOG *dlg, GWEN_NETMSG *netmsg) {
  AH_MSG *msg;

  msg=AH_Msg_new(dlg);
  AH_Msg_SetBuffer(msg, GWEN_NetMsg_TakeBuffer(netmsg));
  GWEN_NetMsg_free(netmsg);
  return msg;
}



AH_MSG *AH_Dialog__MsgFromNetMsg_SSL(AH_DIALOG *dlg, GWEN_NETMSG *netmsg) {
  AH_MSG *msg;
  GWEN_DB_NODE *dbResponse;
  GWEN_BUFFER *mbuf;
  GWEN_BUFFER *newbuf;
  int pos;
  const char *s;
  int i;
  int status;
  char errBuf[256];
  char numbuf[16];
  const char *p;
  int isChunked;
  int isErr;

  /* check HTTP response */
  dbResponse=GWEN_NetMsg_GetDB(netmsg);
  assert(dbResponse);

  /* client code */
  status=GWEN_DB_GetIntValue(dbResponse, "status/code", 0, -1);
  if (status==-1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No status, bad HTTP response, assuming HTTP 0.9 response");
    AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
                           0,
                           AB_Banking_LogLevelError,
                           I18N("No status, bad HTTP response, "
                                "assuming HTTP 0.9 response"));
    status=200;
  }
  snprintf(numbuf, sizeof(numbuf), "%d", status);
  DBG_INFO(AQHBCI_LOGDOMAIN, "HTTP-Status: %d", status);

  isErr=(status<200 || status>299);

  p=GWEN_DB_GetCharValue(dbResponse, "status/text", 0, "");
  errBuf[0]=0;
  errBuf[sizeof(errBuf)-1]=0;
  if (isErr)
    snprintf(errBuf, sizeof(errBuf)-1,
             I18N("HTTP-Error: %d %s"),
             status, p);
  else
    snprintf(errBuf, sizeof(errBuf)-1,
             I18N("HTTP-Status: %d %s"),
             status, p);
  DBG_INFO(AQHBCI_LOGDOMAIN, "%s", errBuf);

  if (isErr)
    AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
                           0,
                           AB_Banking_LogLevelError,
                           errBuf);
  else
    AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
                           0,
                           AB_Banking_LogLevelInfo,
                           errBuf);
  if (isErr) {
    FILE *f;

    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Saving response in \"/tmp/error_response.html\" ...");
    mbuf=GWEN_NetMsg_GetBuffer(netmsg);
    assert(mbuf);
    f=fopen("/tmp/error_response.html", "w+");
    if (!f) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "fopen: %s", strerror(errno));
    }
    else {
      if (fwrite(GWEN_Buffer_GetStart(mbuf),
                 GWEN_Buffer_GetUsedBytes(mbuf),
                 1,
                 f)!=1) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "fwrite: %s", strerror(errno));
      }
      if (fclose(f)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "fclose: %s", strerror(errno));
      }
    }
    GWEN_NetMsg_free(netmsg);
    return 0;
  }

  /* status is ok or not needed, decode message */
  mbuf=GWEN_NetMsg_GetBuffer(netmsg);
  assert(mbuf);
#ifdef AH_DIALOG_HEAVY_DEBUG
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Got this response: ");
  GWEN_Buffer_Dump(mbuf, stderr, 2);
#endif
  newbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(mbuf)*3/4, 0, 1);
  pos=GWEN_Buffer_GetBookmark(mbuf, 1);
  if (pos>=GWEN_Buffer_GetUsedBytes(mbuf)) {
    GWEN_Buffer_free(newbuf);
    GWEN_NetMsg_free(netmsg);
    return 0;
  }

  /* check whether the base64 stuff directly follows */
  isChunked=0;
  p=GWEN_DB_GetCharValue(dbResponse, "header/Transfer-Encoding", 0, 0);
  if (p) {
    /* check for "chunked" */
    if (strcasecmp(p, "chunked")==0)
      isChunked=1;
  }

  if (isChunked) {
    GWEN_BUFFER *dbuf;
    const unsigned char *d;

    DBG_ERROR(AQHBCI_LOGDOMAIN, "Got chunked data");
    dbuf=GWEN_Buffer_new(0, 1024, 0, 1);
    d=(unsigned char*)GWEN_Buffer_GetStart(mbuf)+pos;
    while(*d) {
      GWEN_TYPE_UINT32 len=0;
      GWEN_TYPE_UINT32 cpos;

      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Starting here: %d (%x)",
                d-(unsigned char*)(GWEN_Buffer_GetStart(mbuf)),
                d-(unsigned char*)(GWEN_Buffer_GetStart(mbuf)));

      /* get start of chunk size */
      while(*d && !isxdigit(*d))
        d++;
      if (*d==0)
        break;

      /* read chunk size */
      while(*d && isxdigit(*d)) {
        unsigned char c;

        c=toupper(*d)-'0';
        if (c>9)
          c-=7;
        len=(len<<4)+c;
        d++;
      }
      if (len==0)
        /* chunk size 0, end */
        break;
      /* read until rest of line */
      while (*d && *d !=10 && *d !=13)
        d++;
      if (*d==10 || *d==13)
        d++;
      if (*d==10 || *d==13)
        d++;
      cpos=d-(unsigned char*)(GWEN_Buffer_GetStart(mbuf));
      if (cpos+len>GWEN_Buffer_GetUsedBytes(mbuf)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Bad chunk size \"%d\" (only %d bytes left)",
                  len, GWEN_Buffer_GetUsedBytes(mbuf)-cpos);
        GWEN_Buffer_free(dbuf);
        GWEN_NetMsg_free(netmsg);
        return 0;
      }
      DBG_VERBOUS(AQHBCI_LOGDOMAIN, "Chunksize is %d (%x):", len, len);
      if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevelVerbous)
        GWEN_Text_DumpString((const char*)d, len, stderr, 4);
      GWEN_Buffer_AppendBytes(dbuf, (const char*)d, len);
      d+=len;
      /* skip trailing CR/LF */
      if (*d==10 || *d==13)
        d++;
      if (*d==10 || *d==13)
        d++;
    } /* while */

    DBG_VERBOUS(AQHBCI_LOGDOMAIN, "Decoding base64:");
    if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevelVerbous)
      GWEN_Buffer_Dump(dbuf, stderr, 2);
    if (GWEN_Base64_Decode((const unsigned char*)GWEN_Buffer_GetStart(dbuf),
                           0,
                           newbuf)) {
      DBG_WARN(AQHBCI_LOGDOMAIN,
               "Hmm, message does not seem to be BASE64-encoded...");
      GWEN_Buffer_Reset(newbuf);
      GWEN_Buffer_AppendBuffer(newbuf, dbuf);
    }
    GWEN_Buffer_free(dbuf);
  } /* if chunked */
  else {
    if (GWEN_Base64_Decode((const unsigned char*)GWEN_Buffer_GetStart(mbuf)+pos,
                           0,
                           newbuf)) {
      DBG_WARN(AQHBCI_LOGDOMAIN,
               "Hmm, message does not seem to be BASE64-encoded...");
      GWEN_Buffer_Reset(newbuf);
      GWEN_Buffer_AppendBytes(newbuf,
                              GWEN_Buffer_GetStart(mbuf)+pos,
                              GWEN_Buffer_GetUsedBytes(mbuf));
    }
  }

  if (GWEN_Buffer_GetUsedBytes(newbuf)<6) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Answer too small to be a HBCI message");
    AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
                           0,
                           AB_Banking_LogLevelError,
                           I18N("Answer too small to be a HBCI message"));
    GWEN_Buffer_free(newbuf);
    GWEN_NetMsg_free(netmsg);
    return 0;
  }

  if (strncasecmp(GWEN_Buffer_GetStart(newbuf), "HNHBK:", 6)!=0) {
    FILE *f;

    AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
                           0,
                           AB_Banking_LogLevelError,
                           I18N("Answer does not contain a HBCI message"));
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Answer does not contain a HBCI message");
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Saving response in \"/tmp/error_response.html\" ...");
    mbuf=GWEN_NetMsg_GetBuffer(netmsg);
    assert(mbuf);
    f=fopen("/tmp/error_response.html", "w+");
    if (!f) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "fopen: %s", strerror(errno));
    }
    else {
      if (fwrite(GWEN_Buffer_GetStart(mbuf),
                 GWEN_Buffer_GetUsedBytes(mbuf),
                 1,
                 f)!=1) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "fwrite: %s", strerror(errno));
      }
      if (fclose(f)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "fclose: %s", strerror(errno));
      }
    }

    GWEN_Buffer_free(newbuf);
    GWEN_NetMsg_free(netmsg);
    return 0;
  }

  /* cut of trailing zeros */
  s=GWEN_Buffer_GetStart(newbuf);
  i=GWEN_Buffer_GetUsedBytes(newbuf);
  while((--i)>0) {
    if (s[i]!=0)
      break;
  }
  if (i)
    GWEN_Buffer_Crop(newbuf, 0, i+1);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Empty message received");
    GWEN_Buffer_free(newbuf);
    GWEN_NetMsg_free(netmsg);
    return 0;
  }

  /* finally create HBCI message from data */
  msg=AH_Msg_new(dlg);
  AH_Msg_SetBuffer(msg, newbuf);
  GWEN_NetMsg_free(netmsg);
  return msg;
}



AH_MSG *AH_Dialog__MsgFromNetMsg(AH_DIALOG *dlg, GWEN_NETMSG *netmsg){
  int mark;

  assert(dlg);
  assert(netmsg);

  mark=GWEN_NetConnection_GetUserMark(dlg->connection);
  switch(mark) {
  case AH_HBCI_CONN_MARK_TCP:
    return AH_Dialog__MsgFromNetMsg_HBCI(dlg, netmsg);
  case AH_HBCI_CONN_MARK_SSL:
    return AH_Dialog__MsgFromNetMsg_SSL(dlg, netmsg);
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown connection type (user mark=%d)", mark);
    GWEN_NetMsg_free(netmsg);
    return 0;
  } /* switch */
}



AH_MSG *AH_Dialog_RecvMessage(AH_DIALOG *dlg) {
  AH_MSG *msg;
  GWEN_NETMSG *netmsg;

  assert(dlg->connection);
  netmsg=GWEN_NetConnection_GetInMsg(dlg->connection);
  if (netmsg) {
    /* found a message, transform it */
    msg=AH_Dialog__MsgFromNetMsg(dlg, netmsg);
    if (!msg) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad message received");
      return 0;
    }
    return msg;
  }

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "No incoming message");
  return 0;
}



AH_MSG *AH_Dialog_RecvMessage_Wait(AH_DIALOG *dlg, int timeout) {
  AH_MSG *msg;
  GWEN_NETMSG *netmsg;

  assert(dlg->connection);
  GWEN_WaitCallback_EnterWithText(GWEN_WAITCALLBACK_ID_FAST,
                                  I18N("Waiting for incoming message..."),
                                  I18N("second(s)"),
                                  0);
  GWEN_WaitCallback_SetProgressTotal(GWEN_WAITCALLBACK_PROGRESS_NONE);

  netmsg=GWEN_NetConnection_GetInMsg_Wait(dlg->connection, timeout);
  GWEN_WaitCallback_Leave();

  if (netmsg) {
    /* found a message, transform it */
    msg=AH_Dialog__MsgFromNetMsg(dlg, netmsg);
    if (!msg) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad message received");
      return 0;
    }
    return msg;
  }

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "No incoming message, probably timeout");
  return 0;
}



int AH_Dialog__SendMessageSSL(AH_DIALOG *dlg, AH_MSG *msg) {
  GWEN_BUFFER *nbuf;
  GWEN_BUFFER *obuf;
  GWEN_DB_NODE *dbRequest;
  char numbuf[16];
  GWEN_NETTRANSPORT_STATUS nst;
  const char *s;

  /* we have some things to do:
   * 1) base64-encode the HBCI message
   * 2) create a HTTP request
   */
  obuf=AH_Msg_TakeBuffer(msg);
  nbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(obuf)*2, 0, 1);
  if (GWEN_Base64_Encode((const unsigned char*)GWEN_Buffer_GetStart(obuf),
                         GWEN_Buffer_GetUsedBytes(obuf),
                         nbuf,
                         0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error encoding HBCI message");
    GWEN_Buffer_free(obuf);
    GWEN_Buffer_free(nbuf);
    AH_Msg_free(msg);
    return AB_ERROR_GENERIC;
  }
  GWEN_Buffer_free(obuf);

  dbRequest=GWEN_DB_Group_new("request");
  s=AH_Customer_GetHttpHost(dlg->dialogOwner);
  if (s)
    GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "header/Host", s);
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "header/Pragma", "no-cache");
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "header/Cache-control", "no cache");

  snprintf(numbuf, sizeof(numbuf), "%d", GWEN_Buffer_GetUsedBytes(nbuf));
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "header/Content-type",
                       "application/x-www-form-urlencoded");
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "header/Content-length",
                       numbuf);
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "header/Connection",
                       "keep-alive" /*"close"*/);
  s=AH_Customer_GetHttpUserAgent(dlg->dialogOwner);
  if (s)
    GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "header/User-Agent", s);

  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "command/cmd", "POST");

  /* catch status changes */
  GWEN_NetConnection_WorkIO(dlg->connection);

  nst=GWEN_NetConnection_GetStatus(dlg->connection);
  if (nst==GWEN_NetTransportStatusPDisconnected) {
    /* connection down */
    nst=GWEN_NetTransportStatusUnconnected;
    GWEN_NetConnection_SetStatus(dlg->connection, nst);
  }

  if (nst==GWEN_NetTransportStatusUnconnected) {
    int rv;

    /* start connecting */
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Restarting connection");
    AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
			   0,
			   AB_Banking_LogLevelInfo,
			   I18N("Restarting connection"));
    rv=GWEN_NetConnection_Connect_Wait(dlg->connection, 10);
    if (rv)
      return rv;
  }

  if (GWEN_NetConnectionHTTP_AddRequest(dlg->connection,
                                        dbRequest,
                                        nbuf,
                                        0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not enqueue HTTP request");
    GWEN_DB_Group_free(dbRequest);
    AH_Msg_free(msg);
    return -1;
  }

  GWEN_DB_Group_free(dbRequest);
  AH_Msg_free(msg);
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message enqueued");
  return 0;
}



int AH_Dialog__SendMessageHBCI(AH_DIALOG *dlg, AH_MSG *msg) {
  GWEN_NETMSG *netmsg;

  assert(dlg);
  assert(msg);

  netmsg=GWEN_NetMsg_new(0);
  GWEN_NetMsg_SetBuffer(netmsg, AH_Msg_TakeBuffer(msg));
  assert(dlg->connection);
  GWEN_NetConnection_AddOutMsg(dlg->connection, netmsg);

  if (GWEN_NetConnection_Work(dlg->connection)==
      GWEN_NetConnectionWorkResult_Error) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Error working on HBCI connection");
  }

  AH_Msg_free(msg);
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message enqueued");
  return 0;
}



int AH_Dialog_SendMessage(AH_DIALOG *dlg, AH_MSG *msg) {
  int mark;

  assert(dlg);
  assert(msg);

  if (AH_Msg_GetDialog(msg)!=dlg) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Message wasn't created for this dialog !");
  }

  mark=GWEN_NetConnection_GetUserMark(dlg->connection);

  switch(mark) {
  case AH_HBCI_CONN_MARK_TCP:
    return AH_Dialog__SendMessageHBCI(dlg, msg);
  case AH_HBCI_CONN_MARK_SSL:
    return AH_Dialog__SendMessageSSL(dlg, msg);
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown connection type (user mark=%d)", mark);
    AH_Msg_free(msg);
    return -1;
  } /* switch */


}



int AH_Dialog_SendMessage_Wait(AH_DIALOG *dlg,
                               AH_MSG *msg,
                               int timeout) {
  int rv;

  rv=AH_Dialog_SendMessage(dlg, msg);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not send message");
    return rv;
  }

  if (GWEN_NetConnection_Work(dlg->connection)==
      GWEN_NetConnectionWorkResult_Error) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Error working on HBCI connection");
  }

  GWEN_WaitCallback_EnterWithText(GWEN_WAITCALLBACK_ID_FAST,
                                  I18N("Sending message..."),
                                  I18N("second(s)"),
                                  0);
  GWEN_WaitCallback_SetProgressTotal(GWEN_WAITCALLBACK_PROGRESS_NONE);
  rv=GWEN_NetConnection_Flush(dlg->connection, timeout);
  GWEN_WaitCallback_Leave();
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error sending message for dialog");
    return rv;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message flushed");
  return 0;
}



int AH_Dialog_Work(AH_DIALOG *dlg) {
  int rv;

  assert(dlg);
  assert(dlg->connection);
  rv=GWEN_NetConnection_Work(dlg->connection);
  if (rv==GWEN_NetConnectionWorkResult_Error) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error working on context");
    return rv;
  }
  return 0;
}



GWEN_DB_NODE *AH_Dialog_GetGlobalValues(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->globalValues;
}



void AH_Dialog_AddFlags(AH_DIALOG *dlg, GWEN_TYPE_UINT32 f){
  assert(dlg);
  dlg->flags|=f;
}



void AH_Dialog_SubFlags(AH_DIALOG *dlg, GWEN_TYPE_UINT32 f){
  assert(dlg);
  dlg->flags&=~f;
}



AH_HBCI *AH_Dialog_GetHbci(const AH_DIALOG *dlg) {
  assert(dlg);
  return AH_Bank_GetHbci(dlg->bank);
}



AB_BANKING *AH_Dialog_GetBankingApi(const AH_DIALOG *dlg) {
  return AH_HBCI_GetBankingApi(AH_Bank_GetHbci(dlg->bank));
}



GWEN_TYPE_UINT32 AH_Dialog_GetLastMsgNum(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->lastMsgNum;
}
















