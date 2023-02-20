/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "aqhbci/msglayer/msgcrypt_pintan.h"
#include "aqhbci/banking/user_l.h"

#include "aqbanking/i18n_l.h"
#include "aqbanking/banking_be.h"



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static GWEN_BUFFER *_pinTanCreateSigHead(AH_MSG *hmsg, AB_USER *su, GWEN_MSGENGINE *e, const char *ctrlref);
static GWEN_BUFFER *_pinTanCreateSigTail(AH_MSG *hmsg, AB_USER *su, GWEN_MSGENGINE *e, const char *ctrlref);
static int _pinTanGenerateAndAddSegment(GWEN_MSGENGINE *e, const char *segName, GWEN_DB_NODE *cfg, GWEN_BUFFER *hbuf);
static int _createCtrlRef(char *ctrlref, int len);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_MsgPinTan_PrepareCryptoSeg(AH_MSG *hmsg,
                                  AB_USER *u,
                                  GWEN_DB_NODE *cfg,
                                  int crypt,
                                  int createCtrlRef)
{
  char sdate[9];
  char stime[7];
  char ctrlref[15];
  struct tm *lt;
  time_t tt;
  const char *userId;
  const char *peerId;

  assert(hmsg);
  assert(u);
  assert(cfg);

  userId=AB_User_GetUserId(u);
  assert(userId);
  assert(*userId);
  peerId=AH_User_GetPeerId(u);
  if (!peerId || *peerId==0)
    peerId=userId;

  tt=time(0);
  lt=localtime(&tt);

  if (createCtrlRef) {
    int rv;

    rv=_createCtrlRef(ctrlref, sizeof(ctrlref));
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "ctrlref", ctrlref);
  }

  /* create date */
  if (!strftime(sdate, sizeof(sdate), "%Y%m%d", lt)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Date string too long");
    return GWEN_ERROR_INTERNAL;
  }
  /* create time */
  if (!strftime(stime, sizeof(stime), "%H%M%S", lt)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Date string too long");
    return GWEN_ERROR_INTERNAL;
  }

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/dir", 1);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/date", sdate);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/time", stime);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/bankcode", AB_User_GetBankCode(u));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/userid", crypt?peerId:userId);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keytype", crypt?"V":"S");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keynum", 0);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keyversion", 0);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/code", "PIN");

  if (AH_Msg_GetItanMethod(hmsg)==999) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Using itanMethod 999");
  }

  /*
  if (crypt)
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                        "secProfile/version", 1);
  else
                        */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/version", (AH_Msg_GetItanMethod(hmsg)==999)?1:2);

  return 0;
}




int AH_Msg_SignPinTan(AH_MSG *hmsg, GWEN_UNUSED GWEN_BUFFER *rawBuf, const char *signer)
{
  AH_DIALOG *dlg;
  AH_HBCI *h;
  int rv;
  char ctrlref[15];
  GWEN_MSGENGINE *e;
  AB_USER *su;

  assert(hmsg);
  dlg=AH_Msg_GetDialog(hmsg);
  h=AH_Dialog_GetHbci(dlg);
  assert(h);
  e=AH_Dialog_GetMsgEngine(dlg);
  assert(e);
  GWEN_MsgEngine_SetMode(e, "pintan");

  rv=_createCtrlRef(ctrlref, sizeof(ctrlref));
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  su=AH_Msg_GetUser(hmsg, signer);
  if (!su) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown user \"%s\"", signer);
    return GWEN_ERROR_NOT_FOUND;
  }

  { /* create and insert signature head */
    GWEN_BUFFER *hbuf;
    GWEN_BUFFER *msgBuffer;

    msgBuffer=AH_Msg_GetBuffer(hmsg);
    hbuf=_pinTanCreateSigHead(hmsg, su, e, ctrlref);
    if (hbuf==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }
    /* insert new SigHead at beginning of message buffer */
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Inserting signature head");
    GWEN_Buffer_Rewind(msgBuffer);
    GWEN_Buffer_InsertBytes(msgBuffer,
                            GWEN_Buffer_GetStart(hbuf),
                            GWEN_Buffer_GetUsedBytes(hbuf));
    GWEN_Buffer_free(hbuf);
  }

  { /* create and append signature tail */
    GWEN_BUFFER *hbuf;
    GWEN_BUFFER *msgBuffer;

    msgBuffer=AH_Msg_GetBuffer(hmsg);
    hbuf=_pinTanCreateSigTail(hmsg, su, e, ctrlref);
    if (hbuf==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here");
      return GWEN_ERROR_GENERIC;
    }

    /* append sigtail */
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Appending signature tail");
    if (GWEN_Buffer_AppendBuffer(msgBuffer, hbuf)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here");
      GWEN_Buffer_free(hbuf);
      return GWEN_ERROR_MEMORY_FULL;
    }
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Appending signature tail: done");

    GWEN_Buffer_free(hbuf);
  }
  /* adjust segment numbers (for next signature and message tail */
  AH_Msg_DecFirstSegment(hmsg);
  AH_Msg_IncLastSegment(hmsg);

  return 0;
}



int AH_Msg_EncryptPinTan(AH_MSG *hmsg)
{
  AH_DIALOG *dlg;
  AH_HBCI *h;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *hbuf;
  int rv;
  const char *p;
  GWEN_MSGENGINE *e;
  AB_USER *u;
  GWEN_BUFFER *msgBuffer;

  assert(hmsg);
  dlg=AH_Msg_GetDialog(hmsg);
  h=AH_Dialog_GetHbci(dlg);
  assert(h);
  e=AH_Dialog_GetMsgEngine(dlg);
  assert(e);
  GWEN_MsgEngine_SetMode(e, "pintan");
  u=AH_Dialog_GetDialogOwner(dlg);

  /* buffer for final message */
  hbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* create crypt head */
  cfg=GWEN_DB_Group_new("crypthead");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", 998);

  rv=AH_MsgPinTan_PrepareCryptoSeg(hmsg, u, cfg, 1, 0);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(hbuf);
    return rv;
  }

  /* store system id */
  if (!AH_Msg_NoSysId(hmsg))
    p=AH_User_GetSystemId(u);
  else
    p=NULL;
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/SecId", p?p:"0");
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT, "CryptAlgo/MsgKey", "XXXXXXXX", 8);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "CryptAlgo/keytype", 5);

  rv=_pinTanGenerateAndAddSegment(e, "CryptHead", cfg, hbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(hbuf);
    return GWEN_ERROR_INTERNAL;
  }
  GWEN_DB_Group_free(cfg);


  /* create cryptdata */
  msgBuffer=AH_Msg_GetBuffer(hmsg);
  cfg=GWEN_DB_Group_new("cryptdata");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", 999);
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "cryptdata",
                      GWEN_Buffer_GetStart(msgBuffer),
                      GWEN_Buffer_GetUsedBytes(msgBuffer));

  rv=_pinTanGenerateAndAddSegment(e, "CryptData", cfg, hbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return GWEN_ERROR_INTERNAL;
  }
  GWEN_DB_Group_free(cfg);

  /* replace existing buffer by encrypted one */
  AH_Msg_SetBuffer(hmsg, hbuf);
  return 0;
}




int AH_Msg_DecryptPinTan(AH_MSG *hmsg, GWEN_DB_NODE *gr)
{
  AH_DIALOG *dlg;
  AH_HBCI *h;
  GWEN_BUFFER *mbuf;
  uint32_t l;
  const uint8_t *p;
  GWEN_MSGENGINE *e;
  AB_USER *u;
  const char *peerId;
//  uint32_t uFlags;
  GWEN_DB_NODE *nhead=NULL;
  GWEN_DB_NODE *ndata=NULL;
  const char *crypterId;

  dlg=AH_Msg_GetDialog(hmsg);
  h=AH_Dialog_GetHbci(dlg);
  assert(h);
  e=AH_Dialog_GetMsgEngine(dlg);
  assert(e);
  GWEN_MsgEngine_SetMode(e, "pintan");
  u=AH_Dialog_GetDialogOwner(dlg);
  GWEN_MsgEngine_SetMode(e, "pintan");

//  uFlags=AH_User_GetFlags(u);

  peerId=AH_User_GetPeerId(u);
  if (!peerId || *peerId==0)
    peerId=AB_User_GetUserId(u);

  /* get encrypted session key */
  nhead=GWEN_DB_GetGroup(gr,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "CryptHead");
  if (!nhead) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt head");
    return GWEN_ERROR_BAD_DATA;
  }

  ndata=GWEN_DB_GetGroup(gr,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "CryptData");
  if (!ndata) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt data");
    return GWEN_ERROR_BAD_DATA;
  }

  crypterId=GWEN_DB_GetCharValue(nhead, "key/userId", 0, I18N("unknown"));

  /* get encrypted data */
  p=GWEN_DB_GetBinValue(ndata,
                        "CryptData",
                        0,
                        0, 0,
                        &l);
  if (!p || !l) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt data");
    return GWEN_ERROR_BAD_DATA;
  }

  /* decipher message with session key */
  mbuf=GWEN_Buffer_new(0, l, 0, 1);
  GWEN_Buffer_AppendBytes(mbuf, (const char *)p, l);

  /* store crypter id */
  AH_Msg_SetCrypterId(hmsg, crypterId);

  /* store new buffer inside message */
  AH_Msg_ExchangeBufferWithOrigBuffer(hmsg);
  GWEN_Buffer_Rewind(mbuf);
  AH_Msg_SetBuffer(hmsg, mbuf);

  return 0;
}



GWEN_BUFFER *_pinTanCreateSigHead(AH_MSG *hmsg, AB_USER *su, GWEN_MSGENGINE *e, const char *ctrlref)
{
  AH_DIALOG *dlg;
  uint32_t uFlags;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *hbuf;
  uint32_t tm;
  const char *p;
  int rv;

  dlg=AH_Msg_GetDialog(hmsg);
  hbuf=GWEN_Buffer_new(0, 256, 0, 1);
  cfg=GWEN_DB_Group_new("sighead");

  uFlags=AH_User_GetFlags(su);

  /* for iTAN mode: set selected mode (Sicherheitsfunktion, kodiert) */
  tm=AH_Msg_GetItanMethod(hmsg);
  if (tm==0) {
    tm=AH_Dialog_GetItanMethod(dlg);
    if (tm)
      /* this is needed by AH_MsgPinTan_PrepareCryptoSeg */
      AH_Msg_SetItanMethod(hmsg, tm);
  }

  /* prepare config for segment */
  rv=AH_MsgPinTan_PrepareCryptoSeg(hmsg, su, cfg, 0, 0); /* dont create trlref */
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(hbuf);
    return NULL;
  }
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "ctrlref", ctrlref);

  /* set expected signer */
  if (!(uFlags & AH_USER_FLAGS_BANK_DOESNT_SIGN)) {
    const char *remoteId;

    remoteId=AH_User_GetPeerId(su);
    if (!remoteId || *remoteId==0)
      remoteId=AB_User_GetUserId(su);
    assert(remoteId);
    assert(*remoteId);

    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Expecting \"%s\" to sign the response", remoteId);
    AH_Msg_SetExpectedSigner(hmsg, remoteId);
  }

  /* store system id */
  if (!AH_Msg_NoSysId(hmsg))
    p=AH_User_GetSystemId(su);
  else
    p=NULL;
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/SecId", p?p:"0");

  if (tm)
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "function", tm);

  /* create SigHead */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", AH_Msg_GetFirstSegment(hmsg)-1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signseq", 1);

  /* create signature head segment */
  rv=_pinTanGenerateAndAddSegment(e, "SigHead", cfg, hbuf);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create SigHead");
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(hbuf);
    return NULL;
  }

  GWEN_DB_Group_free(cfg);
  return hbuf;
}



GWEN_BUFFER *_pinTanCreateSigTail(AH_MSG *hmsg, AB_USER *su, GWEN_MSGENGINE *e, const char *ctrlref)
{
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *hbuf;
  char pin[64];
  int rv;

  /* create sigtail */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Completing signature tail");
  hbuf=GWEN_Buffer_new(0, 256, 0, 1);
  cfg=GWEN_DB_Group_new("sigtail");

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", AH_Msg_GetLastSegment(hmsg)+1);
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signature", "NOSIGNATURE", 11);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "ctrlref", ctrlref);

  /* handle pin */
  memset(pin, 0, sizeof(pin));
  rv=AH_User_InputPin(su, pin, 4, sizeof(pin), 0);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error getting pin from medium (%d)", rv);
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(hbuf);
    memset(pin, 0, sizeof(pin));
    return NULL;
  }
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "pin", pin);
  AH_Msg_SetPin(hmsg, pin);
  memset(pin, 0, sizeof(pin));

  /* handle tan */
  if (AH_Msg_GetNeedTan(hmsg)) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "This queue needs a TAN");
    if (AH_Msg_GetTan(hmsg)) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Using existing TAN");
      GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "tan", AH_Msg_GetTan(hmsg));
    }
    else {
      char tan[16];

      memset(tan, 0, sizeof(tan));
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Asking for TAN");
      rv=AH_User_InputTan(su, tan, 4, sizeof(tan));
      if (rv<0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Error getting TAN from medium");
        GWEN_DB_Group_free(cfg);
        GWEN_Buffer_free(hbuf);
        return NULL;
      }
      GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "tan", tan);
      AH_Msg_SetTan(hmsg, tan);
    }
  }
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "This queue doesn't need a TAN");
  }

  rv=_pinTanGenerateAndAddSegment(e, "SigTail", cfg, hbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return NULL;
  }

  GWEN_DB_Group_free(cfg);

  return hbuf;
}



int _pinTanGenerateAndAddSegment(GWEN_MSGENGINE *e, const char *segName, GWEN_DB_NODE *cfg, GWEN_BUFFER *hbuf)
{
  GWEN_XMLNODE *node;
  int rv;

  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e, "SEG", "id", 0, segName);
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"%s\" not found", segName);
    return GWEN_ERROR_NOT_FOUND;
  }

  rv=GWEN_MsgEngine_CreateMessageFromNode(e, node, hbuf, cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create CryptHead (%d)", rv);
    return rv;
  }

  return 0;
}



int _createCtrlRef(char *ctrlref, int len)
{
  struct tm *lt;
  time_t tt;

  tt=time(0);
  lt=localtime(&tt); // TODO: free later?

  /* create control reference */
  if (!strftime(ctrlref, len, "%Y%m%d%H%M%S", lt)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "CtrlRef string too long");
    return GWEN_ERROR_INTERNAL;
  }
  //ctrlref[len-1]=0;
  return 0;
}






