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
#include "user_l.h"
#include "msgengine_l.h"
#include <aqhbci/user.h>

#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/netlayer.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/netlayer.h>
#include <gwenhywfar/nl_socket.h>
#include <gwenhywfar/nl_ssl.h>
#include <gwenhywfar/nl_http.h>
#include <gwenhywfar/nl_hbci.h>
#include <gwenhywfar/net2.h>


#include <string.h>
#include <ctype.h>
#include <errno.h>


#ifdef OS_WIN32
# define AH_PATH_SEP "\\"
#else
# define AH_PATH_SEP "/"
#endif


AH_DIALOG *AH_Dialog_new(AB_USER *u) {
  AH_DIALOG *dlg;
  AH_HBCI *h;
  GWEN_BUFFER *pbuf;

  assert(u);
  h=AH_User_GetHbci(u);

  GWEN_NEW_OBJECT(AH_DIALOG, dlg);
  dlg->usage=1;

  dlg->globalValues=GWEN_DB_Group_new("globalValues");
  dlg->dialogId=strdup("0");

  dlg->msgEngine=AH_User_GetMsgEngine(u);
  GWEN_MsgEngine_Attach(dlg->msgEngine);

  dlg->dialogOwner=u;

  /* create path */
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (AH_HBCI_AddBankPath(h, u, pbuf)) {
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
      GWEN_NetLayer_free(dlg->netLayer);
      free(dlg->dialogId);
      free(dlg->logName);
      GWEN_MsgEngine_free(dlg->msgEngine);
      GWEN_DB_Group_free(dlg->globalValues);

      GWEN_FREE_OBJECT(dlg);
    }
  }
}



const char *AH_Dialog_GetLogFile(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->logName;
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



AB_USER *AH_Dialog_GetDialogOwner(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->dialogOwner;
}



GWEN_NETLAYER *AH_Dialog_GetNetLayer(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->netLayer;
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
  return AH_User_GetHbci(dlg->dialogOwner);
}



AB_BANKING *AH_Dialog_GetBankingApi(const AH_DIALOG *dlg) {
  return AH_HBCI_GetBankingApi(AH_Dialog_GetHbci(dlg));
}



GWEN_TYPE_UINT32 AH_Dialog_GetLastMsgNum(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->lastMsgNum;
}






/* network stuff */


AH_MSG *AH_Dialog_RecvMessage_Wait(AH_DIALOG *dlg, int timeout) {
  AH_MSG *msg;
  GWEN_BUFFER *tbuf;
  int rv;
  const char *p;
  unsigned int i;

  assert(dlg->netLayer);
  tbuf=GWEN_Buffer_new(0, 512, 0, 1);
  GWEN_WaitCallback_EnterWithText(GWEN_WAITCALLBACK_ID_FAST,
                                  I18N("Waiting for incoming message..."),
                                  I18N("second(s)"),
                                  0);
  GWEN_WaitCallback_SetProgressTotal(GWEN_WAITCALLBACK_PROGRESS_NONE);

  rv=GWEN_NetLayer_RecvPacket(dlg->netLayer, tbuf, timeout);
  GWEN_WaitCallback_Leave();
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error receiving packet (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return 0;
  }
  else if (rv==1) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN,
	      "No incoming message due to timeout");
    GWEN_Buffer_free(tbuf);
    return 0;
  }

  /* close connection if wanted */
  if (GWEN_NetLayer_FindBaseLayer(dlg->netLayer, GWEN_NL_HTTP_NAME)) {
    if (!(AH_User_GetFlags(dlg->dialogOwner) & AH_USER_FLAGS_KEEPALIVE)) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN,
                 "Closing connection after reception");
      GWEN_NetLayer_Disconnect(dlg->netLayer);
    }
  }

  /* cut off trailing zeroes */
  i=GWEN_Buffer_GetUsedBytes(tbuf);
  p=GWEN_Buffer_GetStart(tbuf);
  while(i>0) {
    if (p[i-1]!=0)
      break;
    i--;
  }
  GWEN_Buffer_Crop(tbuf, 0, i);

  msg=AH_Msg_new(dlg);
  AH_Msg_SetBuffer(msg, tbuf);

  return msg;
}



AH_MSG *AH_Dialog_RecvMessage(AH_DIALOG *dlg) {
  return AH_Dialog_RecvMessage_Wait(dlg, GWEN_NET2_TIMEOUT_NONE);
}



int AH_Dialog__SendPacket(AH_DIALOG *dlg, const char *buf, int blen,
			  int timeout) {
  AH_HBCI *hbci;
  int rv;

  assert(dlg);
  hbci=AH_Dialog_GetHbci(dlg);
  assert(hbci);
  rv=GWEN_NetLayer_SendPacket(dlg->netLayer,
			      buf, blen,
			      timeout);
  if (rv) {
    if (rv==GWEN_ERROR_NOT_CONNECTED) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN,
                 "Reconnecting dialog");
      if (GWEN_NetLayer_FindBaseLayer(dlg->netLayer, GWEN_NL_HTTP_NAME)) {
	/* this is a http connection, so try to reconnect */
	rv=GWEN_NetLayer_Connect_Wait(dlg->netLayer,
				      AH_HBCI_GetConnectTimeout(hbci));
	if (rv) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Could not connect to bank (%d)", rv);
	  return AB_ERROR_NETWORK;
	}
        /* retry to send the packet */
	rv=GWEN_NetLayer_SendPacket(dlg->netLayer,
				    buf, blen,
				    timeout);
	if (rv) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Could not send packet, giving up");
	  return AB_ERROR_NETWORK;
	}
      }
      else {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Connection down, dialog aborted");
	return AB_ERROR_NETWORK;
      }
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error sending message for dialog");
      return AB_ERROR_NETWORK;
    }
  }

  return 0;
}



int AH_Dialog_SendMessage_Wait(AH_DIALOG *dlg, AH_MSG *msg, int timeout) {
  int rv;
  GWEN_BUFFER *mbuf;

  assert(dlg);
  assert(msg);

  if (AH_Msg_GetDialog(msg)!=dlg) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Message wasn't created for this dialog !");
  }

  mbuf=AH_Msg_GetBuffer(msg);
  assert(mbuf);

  GWEN_WaitCallback_EnterWithText(GWEN_WAITCALLBACK_ID_FAST,
                                  I18N("Sending message..."),
                                  I18N("second(s)"),
                                  0);
  GWEN_WaitCallback_SetProgressTotal(GWEN_WAITCALLBACK_PROGRESS_NONE);
  rv=AH_Dialog__SendPacket(dlg,
			   GWEN_Buffer_GetStart(mbuf),
			   GWEN_Buffer_GetUsedBytes(mbuf),
			   timeout);
  GWEN_WaitCallback_Leave();
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error sending message for dialog");
    return rv;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message sent");
  return 0;
}



int AH_Dialog_SendMessage(AH_DIALOG *dlg, AH_MSG *msg) {
  return AH_Dialog_SendMessage_Wait(dlg, msg, GWEN_NET2_TIMEOUT_NONE);
}



int AH_Dialog__SetAddress(AH_DIALOG *dlg,
			  GWEN_INETADDRESS *addr,
			  const char *bankAddr) {
  GWEN_ERRORCODE err;

  err=GWEN_InetAddr_SetAddress(addr, bankAddr);
  if (!GWEN_Error_IsOk(err)) {
    char dbgbuf[256];

    snprintf(dbgbuf, sizeof(dbgbuf)-1,
	     I18N("Resolving hostname \"%s\" ..."),
	     bankAddr);
    dbgbuf[sizeof(dbgbuf)-1]=0;
    AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg), 0,
			   AB_Banking_LogLevelNotice,
			   dbgbuf);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Resolving hostname \"%s\"",
	     bankAddr);
    err=GWEN_InetAddr_SetName(addr, bankAddr);
    if (!GWEN_Error_IsOk(err)) {
      snprintf(dbgbuf, sizeof(dbgbuf)-1,
	       I18N("Unknown hostname \"%s\""),
	       bankAddr);
      dbgbuf[sizeof(dbgbuf)-1]=0;
      AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg), 0,
			     AB_Banking_LogLevelError,
			     dbgbuf);
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"Error resolving hostname \"%s\":",
		bankAddr);
      DBG_ERROR_ERR(AQHBCI_LOGDOMAIN, err);
      return GWEN_Error_GetSimpleCode(err);
    }
    else {
      char addrBuf[256];
      GWEN_ERRORCODE err;

      err=GWEN_InetAddr_GetAddress(addr, addrBuf, sizeof(addrBuf)-1);
      addrBuf[sizeof(addrBuf)-1]=0;
      if (!GWEN_Error_IsOk(err)) {
	DBG_ERROR_ERR(AQHBCI_LOGDOMAIN, err);
      }
      else {
	snprintf(dbgbuf, sizeof(dbgbuf)-1,
		 I18N("IP address is %s"),
		 addrBuf);
	dbgbuf[sizeof(dbgbuf)-1]=0;
	AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg), 0,
			       AB_Banking_LogLevelNotice,
			       dbgbuf);
      }
    }
  }

  return 0;
}



int AH_Dialog__CreateNetLayer(AH_DIALOG *dlg) {
  GWEN_NETLAYER *nlBase;
  GWEN_NETLAYER *nl;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  int rv;
  const GWEN_URL *url;
  int useHttp;
  GWEN_TYPE_UINT32 uFlags;

  assert(dlg);

  uFlags=AH_User_GetFlags(dlg->dialogOwner);

  /* take bank addr from user */
  url=AH_User_GetServerUrl(dlg->dialogOwner);
  if (!url) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User has no valid address settings");
    return AB_ERROR_INVALID;
  }

  /* create netLayers, the lowest layer is always a socket */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  nlBase=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  rv=AH_Dialog__SetAddress(dlg, addr, GWEN_Url_GetServer(url));
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  GWEN_InetAddr_SetPort(addr, GWEN_Url_GetPort(url));
  GWEN_NetLayer_SetPeerAddr(nlBase, addr);
  GWEN_InetAddr_free(addr);

  useHttp=0;
  if (AH_User_GetCryptMode(dlg->dialogOwner)==AH_CryptMode_Pintan) {
    GWEN_BUFFER *nbuf;
    GWEN_DB_NODE *dbHeader;
    const char *s;

    nbuf=GWEN_Buffer_new(0, 64, 0, 1);
    AH_HBCI_AddBankCertFolder(AH_Dialog_GetHbci(dlg),
                              dlg->dialogOwner, nbuf);

    nl=GWEN_NetLayerSsl_new(nlBase,
			    GWEN_Buffer_GetStart(nbuf),
			    GWEN_Buffer_GetStart(nbuf),
                            0, 0, 0);
    GWEN_NetLayer_free(nlBase);
    nlBase=nl;
    GWEN_Buffer_Reset(nbuf);

    GWEN_NetLayerSsl_SetAskAddCertFn(nlBase,
                                     AB_Banking_AskAddCert,
                                     AH_Dialog_GetBankingApi(dlg));

    nl=GWEN_NetLayerHttp_new(nlBase);
    assert(nl);
    GWEN_NetLayer_free(nlBase);

    dbHeader=GWEN_NetLayerHttp_GetOutHeader(nl);
    assert(dbHeader);

    s=GWEN_Url_GetServer(url);
    if (s)
      GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "Host", s);
    GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "Pragma", "no-cache");
    GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "Cache-control", "no cache");
    GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "Content-type",
                         "application/x-www-form-urlencoded");
    GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "Connection",
                         (uFlags & AH_USER_FLAGS_KEEPALIVE)?
                         "keep-alive":"close");
    s=AH_User_GetHttpUserAgent(dlg->dialogOwner);
    if (s)
      GWEN_DB_SetCharValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "User-Agent", s);

    GWEN_NetLayerHttp_SetOutCommand(nl, "POST", url);
    GWEN_Buffer_free(nbuf);
    useHttp=1;
  }
  else {
    nl=nlBase;
  }

  /* create HBCI layer around whatever layer we just created */
  nlBase=nl;
  nl=GWEN_NetLayerHbci_new(nlBase);
  assert(nl);
  GWEN_NetLayer_free(nlBase);
  if (useHttp)
    GWEN_NetLayer_AddFlags(nl, GWEN_NL_HBCI_FLAGS_BASE64);
  dlg->netLayer=nl;

  GWEN_Net_AddConnectionToPool(nl);

  return 0;
}



int AH_Dialog_Connect(AH_DIALOG *dlg, int timeout) {
  int rv;

  AH_Dialog_AddFlags(dlg, AH_DIALOG_FLAGS_INITIATOR);
  AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
			 0,
			 AB_Banking_LogLevelNotice,
			 I18N("Connecting to bank..."));

  rv=AH_Dialog__CreateNetLayer(dlg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=GWEN_NetLayer_Connect_Wait(dlg->netLayer, timeout);
  if (rv) {
    if (rv==1) {
      AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
                             0,
                             AB_Banking_LogLevelNotice,
                             I18N("Timeout."));

    }
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Could not connect to bank (%d)", rv);
    GWEN_NetLayer_free(dlg->netLayer);
    dlg->netLayer=0;
    return AB_ERROR_NETWORK;
  }
  AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
			 0,
			 AB_Banking_LogLevelNotice,
			 I18N("Connected."));

  return 0;
}



int AH_Dialog_Disconnect(AH_DIALOG *dlg, int timeout) {
  int rv;

  AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
			 0,
			 AB_Banking_LogLevelNotice,
			 I18N("Disconnecting from bank..."));

  rv=GWEN_NetLayer_Disconnect_Wait(dlg->netLayer, timeout);
  dlg->flags&=~AH_DIALOG_FLAGS_OPEN;
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "Could not disconnect from bank (%d)", rv);
    GWEN_NetLayer_free(dlg->netLayer);
    dlg->netLayer=0;
    return AB_ERROR_NETWORK;
  }
  AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
			 0,
			 AB_Banking_LogLevelNotice,
			 I18N("Disconnected."));

  return 0;
}









