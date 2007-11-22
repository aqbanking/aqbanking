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
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>

#include <gwenhywfar/iolayer.h>
#include <gwenhywfar/iomanager.h>

#include <string.h>
#include <ctype.h>
#include <errno.h>


#ifdef OS_WIN32
# define AH_PATH_SEP "\\"
#else
# define AH_PATH_SEP "/"
#endif


AH_DIALOG *AH_Dialog_new(AB_USER *u, uint32_t guiid) {
  AH_DIALOG *dlg;
  AH_HBCI *h;
  GWEN_BUFFER *pbuf;

  assert(u);
  h=AH_User_GetHbci(u);

  GWEN_NEW_OBJECT(AH_DIALOG, dlg);
  dlg->usage=1;

  dlg->guiid=guiid;

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
      GWEN_Io_Layer_free(dlg->ioLayer);
      free(dlg->dialogId);
      free(dlg->logName);
      GWEN_MsgEngine_free(dlg->msgEngine);
      GWEN_DB_Group_free(dlg->globalValues);

      GWEN_FREE_OBJECT(dlg);
    }
  }
}



uint32_t AH_Dialog_GetGuiId(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->guiid;
}



const char *AH_Dialog_GetLogFile(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->logName;
}



uint32_t AH_Dialog_GetNextMsgNum(AH_DIALOG *dlg){
  assert(dlg);
  return ++dlg->lastMsgNum;
}



uint32_t
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



GWEN_MSGENGINE *AH_Dialog_GetMsgEngine(const AH_DIALOG *dlg){
  assert(dlg);
  assert(dlg->msgEngine);
  return dlg->msgEngine;
}



int AH_Dialog_CheckReceivedMsgNum(AH_DIALOG *dlg,
                                  uint32_t msgnum){
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



uint32_t AH_Dialog_GetFlags(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->flags;
}



void AH_Dialog_SetFlags(AH_DIALOG *dlg, uint32_t f){
  assert(dlg);
  dlg->flags=f;
}



GWEN_DB_NODE *AH_Dialog_GetGlobalValues(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->globalValues;
}



void AH_Dialog_AddFlags(AH_DIALOG *dlg, uint32_t f){
  assert(dlg);
  dlg->flags|=f;
}



void AH_Dialog_SubFlags(AH_DIALOG *dlg, uint32_t f){
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



uint32_t AH_Dialog_GetLastMsgNum(const AH_DIALOG *dlg){
  assert(dlg);
  return dlg->lastMsgNum;
}






/* network stuff */
int AH_Dialog_RecvMessage(AH_DIALOG *dlg, AH_MSG **pMsg) {
  assert(dlg);
  if (AH_User_GetCryptMode(dlg->dialogOwner)==AH_CryptMode_Pintan)
    return AH_Dialog_RecvMessage_Https(dlg, pMsg, GWEN_TIMEOUT_FOREVER);
  else
    return AH_Dialog_RecvMessage_Hbci(dlg, pMsg, GWEN_TIMEOUT_FOREVER);
}



int AH_Dialog__SendPacket(AH_DIALOG *dlg,
			  const char *buf, int blen,
			  int timeout) {
  assert(dlg);
  if (AH_User_GetCryptMode(dlg->dialogOwner)==AH_CryptMode_Pintan)
    return AH_Dialog_SendPacket_Https(dlg, buf, blen, timeout);
  else
    return AH_Dialog_SendPacket_Hbci(dlg, buf, blen, timeout);
}



int AH_Dialog_SendMessage(AH_DIALOG *dlg, AH_MSG *msg) {
  int rv;
  GWEN_BUFFER *mbuf;

  assert(dlg);
  assert(msg);

  if (AH_Msg_GetDialog(msg)!=dlg) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Message wasn't created for this dialog !");
    return GWEN_ERROR_INVALID;
  }

  mbuf=AH_Msg_GetBuffer(msg);
  assert(mbuf);

  rv=AH_Dialog__SendPacket(dlg,
			   GWEN_Buffer_GetStart(mbuf),
			   GWEN_Buffer_GetUsedBytes(mbuf),
			   GWEN_TIMEOUT_FOREVER);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error sending message for dialog");
    return rv;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message sent");
  return 0;
}



int AH_Dialog__SetAddress(AH_DIALOG *dlg,
			  GWEN_INETADDRESS *addr,
			  const char *bankAddr) {
  int err;

  err=GWEN_InetAddr_SetAddress(addr, bankAddr);
  if (err) {
    char dbgbuf[256];

    snprintf(dbgbuf, sizeof(dbgbuf)-1,
	     I18N("Resolving hostname \"%s\" ..."),
	     bankAddr);
    dbgbuf[sizeof(dbgbuf)-1]=0;
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Notice,
			 dbgbuf);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Resolving hostname \"%s\"",
	     bankAddr);
    err=GWEN_InetAddr_SetName(addr, bankAddr);
    if (err) {
      snprintf(dbgbuf, sizeof(dbgbuf)-1,
	       I18N("Unknown hostname \"%s\""),
	       bankAddr);
      dbgbuf[sizeof(dbgbuf)-1]=0;
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   dbgbuf);
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"Error resolving hostname \"%s\":",
		bankAddr);
      DBG_ERROR_ERR(AQHBCI_LOGDOMAIN, err);
      return err;
    }
    else {
      char addrBuf[256];
      int err;

      err=GWEN_InetAddr_GetAddress(addr, addrBuf, sizeof(addrBuf)-1);
      addrBuf[sizeof(addrBuf)-1]=0;
      if (err) {
	DBG_ERROR_ERR(AQHBCI_LOGDOMAIN, err);
      }
      else {
	snprintf(dbgbuf, sizeof(dbgbuf)-1,
		 I18N("IP address is %s"),
		 addrBuf);
	dbgbuf[sizeof(dbgbuf)-1]=0;
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Notice,
			     dbgbuf);
      }
    }
  }

  return 0;
}



int AH_Dialog__CreateIoLayer(AH_DIALOG *dlg) {
  if (dlg->ioLayer==NULL) {
    if (AH_User_GetCryptMode(dlg->dialogOwner)==AH_CryptMode_Pintan)
      return AH_Dialog_CreateIoLayer_Https(dlg);
    else
      return AH_Dialog_CreateIoLayer_Hbci(dlg);
  }
  else
    return 0;
}



int AH_Dialog_Connect(AH_DIALOG *dlg, int timeout) {
  int rv;

  AH_Dialog_AddFlags(dlg, AH_DIALOG_FLAGS_INITIATOR);
  GWEN_Gui_ProgressLog(dlg->guiid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Connecting to bank..."));

  rv=AH_Dialog__CreateIoLayer(dlg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (AH_User_GetCryptMode(dlg->dialogOwner)!=AH_CryptMode_Pintan) {
    /* only connect for non-HTTP mode (HTTP connects when necessary) */
    rv=GWEN_Io_Layer_ConnectRecursively(dlg->ioLayer,
					NULL,
					0,
					dlg->guiid,
					timeout);
    if (rv) {
      if (rv==GWEN_ERROR_TIMEOUT) {
	GWEN_Gui_ProgressLog(dlg->guiid,
			     GWEN_LoggerLevel_Notice,
			     I18N("Timeout."));
      }
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"Could not connect to bank (%d)", rv);
      GWEN_Io_Layer_free(dlg->ioLayer);
      dlg->ioLayer=NULL;
      return AB_ERROR_NETWORK;
    }
    GWEN_Gui_ProgressLog(dlg->guiid,
			 GWEN_LoggerLevel_Notice,
			 I18N("Connected."));
  }

  return 0;
}



int AH_Dialog_Disconnect(AH_DIALOG *dlg, int timeout) {
  if (AH_User_GetCryptMode(dlg->dialogOwner)!=AH_CryptMode_Pintan) {
    int rv;

    GWEN_Gui_ProgressLog(dlg->guiid,
			 GWEN_LoggerLevel_Notice,
			 I18N("Disconnecting from bank..."));

    rv=GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer,
					   NULL,
					   0,
					   dlg->guiid, timeout);
    dlg->flags&=~AH_DIALOG_FLAGS_OPEN;
    if (rv) {
      DBG_WARN(AQHBCI_LOGDOMAIN,
	       "Could not disconnect from bank (%d)", rv);
    }

    GWEN_Io_Layer_free(dlg->ioLayer);
    dlg->ioLayer=0;

    GWEN_Gui_ProgressLog(dlg->guiid,
			 GWEN_LoggerLevel_Notice,
			 I18N("Disconnected."));
  }

  return 0;
}



void AH_Dialog_SetItanMethod(AH_DIALOG *dlg, uint32_t i) {
  assert(dlg);
  dlg->itanMethod=i;
}



uint32_t AH_Dialog_GetItanMethod(const AH_DIALOG *dlg) {
  assert(dlg);
  return dlg->itanMethod;
}



int AH_Dialog_GetItanProcessType(const AH_DIALOG *dlg) {
  assert(dlg);
  return dlg->itanProcessType;
}



void AH_Dialog_SetItanProcessType(AH_DIALOG *dlg, int i) {
  assert(dlg);
  dlg->itanProcessType=i;
}




#include "dialog_hbci.c"
#include "dialog_https.c"












