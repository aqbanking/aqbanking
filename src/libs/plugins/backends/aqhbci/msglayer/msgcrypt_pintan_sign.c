/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "aqhbci/msglayer/msgcrypt_pintan_sign.h"
#include "aqhbci/msglayer/msgcrypt.h"
#include "aqhbci/banking/user_l.h"
#include "message_p.h"

#include "aqbanking/i18n_l.h"
#include "aqbanking/banking_be.h"



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _createAndInsertSigHead(AH_MSG *hmsg, AB_USER *su, GWEN_MSGENGINE *e, const char *ctrlref);
static int _createAndAppendSigTail(AH_MSG *hmsg, AB_USER *su, GWEN_MSGENGINE *e, const char *ctrlref);
static GWEN_BUFFER *_pinTanCreateSigHead(AH_MSG *hmsg, AB_USER *su, GWEN_MSGENGINE *e, const char *ctrlref);
static GWEN_BUFFER *_pinTanCreateSigTail(AH_MSG *hmsg, AB_USER *su, GWEN_MSGENGINE *e, const char *ctrlref);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

int AH_Msg_SignPinTan(AH_MSG *hmsg, GWEN_UNUSED GWEN_BUFFER *rawBuf, const char *signer)
{
  int rv;
  char ctrlref[15];
  GWEN_MSGENGINE *e;
  AB_USER *su;

  assert(hmsg);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  GWEN_MsgEngine_SetMode(e, "pintan");

  rv=AH_Msg_CreateCtrlRef(ctrlref, sizeof(ctrlref));
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  su=AH_Msg_GetUser(hmsg, signer);
  if (!su) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown user \"%s\"", signer);
    return GWEN_ERROR_NOT_FOUND;
  }

  rv=_createAndInsertSigHead(hmsg, su, e, ctrlref);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_createAndAppendSigTail(hmsg, su, e, ctrlref);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* adjust segment numbers (for next signature and message tail */
  hmsg->firstSegment--;
  hmsg->lastSegment++;

  return 0;
}



int _createAndInsertSigHead(AH_MSG *hmsg, AB_USER *su, GWEN_MSGENGINE *e, const char *ctrlref)
{ /* create and insert signature head */
  GWEN_BUFFER *hbuf;

  hbuf=_pinTanCreateSigHead(hmsg, su, e, ctrlref);
  if (hbuf==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }
  /* insert new SigHead at beginning of message buffer */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Inserting signature head");
  GWEN_Buffer_Rewind(hmsg->buffer);
  GWEN_Buffer_InsertBytes(hmsg->buffer,
                          GWEN_Buffer_GetStart(hbuf),
                          GWEN_Buffer_GetUsedBytes(hbuf));
  GWEN_Buffer_free(hbuf);
  return 0;
}



int _createAndAppendSigTail(AH_MSG *hmsg, AB_USER *su, GWEN_MSGENGINE *e, const char *ctrlref)
{
  GWEN_BUFFER *hbuf;

  hbuf=_pinTanCreateSigTail(hmsg, su, e, ctrlref);
  if (hbuf==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }

  /* append sigtail */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Appending signature tail");
  if (GWEN_Buffer_AppendBuffer(hmsg->buffer, hbuf)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_Buffer_free(hbuf);
    return GWEN_ERROR_MEMORY_FULL;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Appending signature tail: done");

  GWEN_Buffer_free(hbuf);
  return 0;
}



GWEN_BUFFER *_pinTanCreateSigHead(AH_MSG *hmsg, AB_USER *su, GWEN_MSGENGINE *e, const char *ctrlref)
{
  uint32_t uFlags;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *hbuf;
  uint32_t tm;
  const char *p;
  int rv;

  hbuf=GWEN_Buffer_new(0, 256, 0, 1);
  cfg=GWEN_DB_Group_new("sighead");

  uFlags=AH_User_GetFlags(su);

  /* for iTAN mode: set selected mode (Sicherheitsfunktion, kodiert) */
  tm=AH_Msg_GetItanMethod(hmsg);
  if (tm==0) {
    tm=AH_Dialog_GetItanMethod(hmsg->dialog);
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
  if (!hmsg->noSysId)
    p=AH_User_GetSystemId(su);
  else
    p=NULL;
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/SecId", p?p:"0");

  if (tm)
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "function", tm);

  /* create SigHead */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", hmsg->firstSegment-1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signseq", 1);

  /* create signature head segment */
  rv=AH_Msg_GenerateAndAddSegment(e, "SigHead", cfg, hbuf);
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

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", hmsg->lastSegment+1);
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
  if (hmsg->needTan) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "This queue needs a TAN");
    if (hmsg->usedTan) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Using existing TAN");
      GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "tan", hmsg->usedTan);
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

  rv=AH_Msg_GenerateAndAddSegment(e, "SigTail", cfg, hbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return NULL;
  }

  GWEN_DB_Group_free(cfg);

  return hbuf;
}




