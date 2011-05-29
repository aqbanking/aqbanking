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
      GWEN_SyncIo_free(dlg->ioLayer);
      GWEN_HttpSession_free(dlg->httpSession);
      free(dlg->dialogId);
      free(dlg->logName);
      GWEN_MsgEngine_free(dlg->msgEngine);
      GWEN_DB_Group_free(dlg->globalValues);
      AH_TanMethod_free(dlg->tanMethodDescription);

      GWEN_FREE_OBJECT(dlg);
    }
  }
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
    return AH_Dialog_RecvMessage_Https(dlg, pMsg);
  else
    return AH_Dialog_RecvMessage_Hbci(dlg, pMsg);
}



int AH_Dialog_SendPacket(AH_DIALOG *dlg, const char *buf, int blen) {
  assert(dlg);
  if (AH_User_GetCryptMode(dlg->dialogOwner)==AH_CryptMode_Pintan)
    return AH_Dialog_SendPacket_Https(dlg, buf, blen);
  else
    return AH_Dialog_SendPacket_Hbci(dlg, buf, blen);
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

  rv=AH_Dialog_SendPacket(dlg,
			  GWEN_Buffer_GetStart(mbuf),
			  GWEN_Buffer_GetUsedBytes(mbuf));
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error sending message for dialog (%d)", rv);
    return rv;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message sent");
  return 0;
}



int AH_Dialog_Connect(AH_DIALOG *dlg) {
  AH_Dialog_AddFlags(dlg, AH_DIALOG_FLAGS_INITIATOR);
  if (AH_User_GetCryptMode(dlg->dialogOwner)==AH_CryptMode_Pintan)
    return AH_Dialog_Connect_Https(dlg);
  else
    return AH_Dialog_Connect_Hbci(dlg);
}



int AH_Dialog_Disconnect(AH_DIALOG *dlg) {
  if (AH_User_GetCryptMode(dlg->dialogOwner)==AH_CryptMode_Pintan)
    return AH_Dialog_Disconnect_Https(dlg);
  else
    return AH_Dialog_Disconnect_Hbci(dlg);
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



int AH_Dialog_GetTanJobVersion(const AH_DIALOG *dlg) {
  assert(dlg);
  return dlg->tanJobVersion;
}



void AH_Dialog_SetTanJobVersion(AH_DIALOG *dlg, int i) {
  assert(dlg);
  dlg->tanJobVersion=i;
}



const AH_TAN_METHOD *AH_Dialog_GetTanMethodDescription(const AH_DIALOG *dlg) {
  assert(dlg);
  return dlg->tanMethodDescription;
}



void AH_Dialog_SetTanMethodDescription(AH_DIALOG *dlg, const AH_TAN_METHOD *tm) {
  assert(dlg);
  AH_TanMethod_free(dlg->tanMethodDescription);
  if (tm)
    dlg->tanMethodDescription=AH_TanMethod_dup(tm);
  else
    dlg->tanMethodDescription=NULL;
}




#include "dialog_hbci.c"
#include "dialog_https.c"












