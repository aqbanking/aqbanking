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

#include "message_p.h"
#include "aqhbci_l.h"
#include "hbci_l.h"
#include "mediumctx_l.h"

#include <aqhbci/msgengine.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/list.h>

#include <aqbanking/banking.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>




/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_PrepareCryptoSeg(AH_MSG *hmsg,
                            AH_CUSTOMER *cu,
                            GWEN_DB_NODE *cfg,
                            const GWEN_KEYSPEC *ks,
                            int crypt,
                            int createCtrlRef) {
  char sdate[9];
  char stime[7];
  char ctrlref[15];
  struct tm *lt;
  time_t tt;
  GWEN_MSGENGINE *e;
  AH_BANK *b;
  AH_USER *u;
  const char *userId;
  const char *peerId;

  assert(hmsg);
  assert(cfg);
  assert(ks);

  b=AH_Dialog_GetBank(hmsg->dialog);
  assert(b);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  u=AH_Customer_GetUser(cu);
  assert(u);

  userId=AH_User_GetUserId(u);
  assert(userId);
  assert(*userId);
  peerId=AH_User_GetPeerId(u);
  if (!peerId || !*peerId)
    peerId=GWEN_KeySpec_GetOwner(ks);
  if (!peerId || !*peerId)
    peerId=userId;

  tt=time(0);
  lt=localtime(&tt);

  if (createCtrlRef) {
    /* create control reference */
    if (!strftime(ctrlref, sizeof(ctrlref),
                  "%Y%m%d%H%M%S", lt)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "CtrlRef string too long");
      return -1;
    }

    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "ctrlref", ctrlref);
  }
  /* create date */
  if (!strftime(sdate, sizeof(sdate),
                "%Y%m%d", lt)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Date string too long");
    return -1;
  }
  /* create time */
  if (!strftime(stime, sizeof(stime),
                "%H%M%S", lt)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Date string too long");
    return -1;
  }

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Date and Time: %s / %s",
           sdate, stime);

  if (AH_Dialog_GetFlags(hmsg->dialog) &
      AH_DIALOG_FLAGS_INITIATOR)
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                        "SecDetails/dir", 1);
  else
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                        "SecDetails/dir", 2);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "SecStamp/date", sdate);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "SecStamp/time", stime);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "key/bankcode",
                       AH_Bank_GetBankId(b));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "key/userid",
                       crypt?peerId:userId);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "key/keytype",
                       crypt?"V":"S");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "key/keynum",
                      GWEN_KeySpec_GetNumber(ks));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "key/keyversion",
                      GWEN_KeySpec_GetVersion(ks));

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_SignMsg(AH_MSG *hmsg,
                   GWEN_BUFFER *rawBuf,
                   const GWEN_KEYSPEC *ks) {
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *sigbuf;
  GWEN_BUFFER *hbuf;
  unsigned int l;
  int rv;
  char ctrlref[15];
  const char *p;
  GWEN_MSGENGINE *e;
  AH_MEDIUM *medium;
  AH_MEDIUM_CTX *mctx;
  AH_BANK *b;
  AH_USER *u;
  AH_CUSTOMER *scu; /* singing customer */
  const char *userId;

  assert(hmsg);
  b=AH_Dialog_GetBank(hmsg->dialog);
  u=AH_Customer_GetUser(AH_Dialog_GetDialogOwner(hmsg->dialog));
  assert(u);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  userId=GWEN_KeySpec_GetOwner(ks);
  //if (!userId || *userId==0)
  //  userId=AH_User_GetUserId(u);
  assert(userId);
  assert(*userId);

  scu=AH_HBCI_FindCustomer(AH_Bank_GetHbci(b),
                           AH_Bank_GetCountry(b),
                           AH_Bank_GetBankId(b),
                           "*",
                           userId);
  if (!scu) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Unknown customer \"%s\"",
              userId);
    return -1;
  }

  /* get medium of signer */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Getting medium for signing");
  medium=AH_HBCI_GetMedium(AH_Bank_GetHbci(b), scu);
  if (!medium) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not find medium for user \"%s\"",
             AH_User_GetUserId(u));
    return -1;
  }

  mctx=AH_Medium_GetCurrentContext(medium);
  assert(mctx);

  p=AH_CryptMode_toString(AH_User_GetCryptMode(u));
  GWEN_MsgEngine_SetMode(e, p);

  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         "SEG",
                                         "id",
                                         0,
                                         "SigHead");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"SigHead\" not found");
    return -1;
  }

  cfg=GWEN_DB_Group_new("sighead");

  /* get real keyspec */
  ks=AH_MediumCtx_GetLocalSignKeySpec(mctx);
  if (!ks) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No keyspec for local sign key");
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN,
            "Really signing with \"%s\"",
            userId);

  /* prepare config for segment */
  if (AH_Msg_PrepareCryptoSeg(hmsg, scu, cfg, ks, 0, 1)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* store security id */
  if (strcasecmp(GWEN_MsgEngine_GetMode(e), "RDH")==0) {
    const char *secid;
    const GWEN_KEYSPEC *eks;

    /* set expected signer */
    if (AH_Customer_GetBankSigns(AH_Dialog_GetDialogOwner(hmsg->dialog))) {
      eks=AH_MediumCtx_GetRemoteSignKeySpec(mctx);
      if (eks) {
        const char *remoteId;

        remoteId=GWEN_KeySpec_GetOwner(eks);
        if (!remoteId || *remoteId==0)
          remoteId=AH_User_GetPeerId(u);
        if (!remoteId || *remoteId==0)
          remoteId=AH_User_GetUserId(u);
        assert(remoteId);
        assert(*remoteId);

        DBG_DEBUG(AQHBCI_LOGDOMAIN,
                 "Expecting \"%s\" to sign the response",
                 remoteId);
        AH_Msg_SetExpectedSigner(hmsg, remoteId);
      }
    }

    if (hmsg->noSysId)
      secid="0";
    else {
      secid=AH_Customer_GetSystemId(AH_Dialog_GetDialogOwner(hmsg->dialog));
      if (!secid)
        secid="0";
    }

    /* RDH mode */
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "SecDetails/SecId",
                         secid);
  }
  else if (strcasecmp(GWEN_MsgEngine_GetMode(e), "DDV")==0) {
    const GWEN_KEYSPEC *eks;
    GWEN_BUFFER *idBuf;

    /* set expected signer */
    if (AH_Customer_GetBankSigns(AH_Dialog_GetDialogOwner(hmsg->dialog))) {
      eks=AH_MediumCtx_GetRemoteSignKeySpec(mctx);
      if (eks) {
        const char *remoteId;

        remoteId=GWEN_KeySpec_GetOwner(eks);
        if (!remoteId || *remoteId==0)
          remoteId=AH_User_GetPeerId(u);
        if (!remoteId || *remoteId==0)
          remoteId=AH_User_GetUserId(u);
        assert(remoteId);
        assert(*remoteId);

        DBG_DEBUG(AQHBCI_LOGDOMAIN,
                 "Expecting \"%s\" to sign the response",
                 remoteId);
        AH_Msg_SetExpectedSigner(hmsg, remoteId);
      }
    }

    idBuf=GWEN_Buffer_new(0, 128, 0, 1);
    rv=AH_Medium_GetTokenIdData(medium, idBuf);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "No system id on DDV medium, should not happen (%d)", rv);
      GWEN_Buffer_free(idBuf);
      GWEN_DB_Group_free(cfg);
      return -1;
    }

    /* DDV mode */
    GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                        "SecDetails/SecId",
                        GWEN_Buffer_GetStart(idBuf),
                        GWEN_Buffer_GetUsedBytes(idBuf));
    GWEN_Buffer_free(idBuf);
  }
  else if (strcasecmp(GWEN_MsgEngine_GetMode(e), "PINTAN")==0) {
    const char *secid;

    if (hmsg->noSysId)
      secid="0";
    else {
      secid=AH_Customer_GetSystemId(AH_Dialog_GetDialogOwner(hmsg->dialog));
      if (!secid)
        secid="0";
    }

    /* PINTAN mode, uses a system id as RDH mode does */
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "SecDetails/SecId",
                         secid);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Unknown mode \"%s\"",
              GWEN_MsgEngine_GetMode(e));
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  p=GWEN_DB_GetCharValue(cfg, "ctrlref", 0, "");
  if (strlen(p)>=sizeof(ctrlref)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Control reference too long (14 bytes maximum)");
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  strcpy(ctrlref, p);

  /* create SigHead */
  hbuf=GWEN_Buffer_new(0, 128+GWEN_Buffer_GetUsedBytes(rawBuf), 0, 1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", hmsg->firstSegment-1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "signseq",
                      AH_Medium_GetNextSignSeq(medium));



  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  GWEN_DB_Group_free(cfg);
  cfg=0;
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create SigHead");
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  /* remember size of sighead for now */
  l=GWEN_Buffer_GetUsedBytes(hbuf);

  /* add raw data to to-sign data buffer */
  if (GWEN_Buffer_AppendBuffer(hbuf, rawBuf)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  /* sign message, create sigtail */
  cfg=GWEN_DB_Group_new("sigtail");
  sigbuf=GWEN_Buffer_new(0, 512, 0, 1);
  if (strcasecmp(GWEN_MsgEngine_GetMode(e), "PINTAN")==0) {
    char pin[8];

    GWEN_Buffer_AppendString(sigbuf, "NOSIGNATURENEEDED");
    memset(pin, 0, sizeof(pin));
    if (AH_Medium_InputPin(medium, pin, 4, sizeof(pin), 0)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error getting pin from medium");
      GWEN_Buffer_free(sigbuf);
      GWEN_DB_Group_free(cfg);
      GWEN_Buffer_free(hbuf);
      return -1;
    }
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "pin", pin);
    AH_Msg_SetPin(hmsg, pin);

    if (hmsg->needTan) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN,
                 "This queue needs a TAN");
      if (hmsg->usedTan) {
        DBG_NOTICE(AQHBCI_LOGDOMAIN,
                   "Using existing TAN");
        GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                             "tan", hmsg->usedTan);
      }
      else {
        char tan[16];

        memset(tan, 0, sizeof(tan));
        DBG_NOTICE(AQHBCI_LOGDOMAIN,
                   "Asking for TAN");
        if (AH_Medium_InputTan(medium, tan, 4, sizeof(tan))) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Error getting TAN from medium");
          GWEN_Buffer_free(sigbuf);
          GWEN_DB_Group_free(cfg);
          GWEN_Buffer_free(hbuf);
          return -1;
        }
        GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                             "tan", tan);
        AH_Msg_SetTan(hmsg, tan);
      }
    }
    else {
      DBG_NOTICE(AQHBCI_LOGDOMAIN,
                 "This queue doesn't need a TAN");
    }
  }
  else {
    if (AH_Medium_Sign(medium, hbuf, sigbuf)!=AH_MediumResultOk) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Error signing with medium of user \"%s\"",
               userId);
      GWEN_Buffer_free(sigbuf);
      GWEN_DB_Group_free(cfg);
      GWEN_Buffer_free(hbuf);
      return -1;
    }
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Signing done");

  /* insert new SigHead at beginning of message buffer */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Inserting signature head");
  GWEN_Buffer_Rewind(hmsg->buffer);
  GWEN_Buffer_InsertBytes(hmsg->buffer, GWEN_Buffer_GetStart(hbuf), l);

  /* create sigtail */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Completing signature tail");
  GWEN_Buffer_Reset(hbuf);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", hmsg->lastSegment+1);
  /* store to DB */
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "signature",
                      GWEN_Buffer_GetStart(sigbuf),
                      GWEN_Buffer_GetUsedBytes(sigbuf));

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "ctrlref", ctrlref);
  GWEN_Buffer_free(sigbuf);

  /* get node */
  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         "SEG",
                                         "id",
                                         0,
                                         "SigTail");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"SigTail\"not found");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create SigTail");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* append sigtail */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Appending signature tail");
  if (GWEN_Buffer_AppendBuffer(hmsg->buffer, hbuf)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Appending signature tail: done");

  GWEN_Buffer_free(hbuf);
  GWEN_DB_Group_free(cfg);

  /* adjust segment numbers (for next signature and message tail */
  hmsg->firstSegment--;
  hmsg->lastSegment++;

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_EncryptMsg(AH_MSG *hmsg) {
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *cryptbuf;
  GWEN_BUFFER *hbuf;
  GWEN_BUFFER *msgkeybuf;
  int rv;
  GWEN_MSGENGINE *e;
  AH_MEDIUM *medium;
  AH_MEDIUM_CTX *mctx;
  const GWEN_KEYSPEC *ks;
  const GWEN_KEYSPEC *lks;
  AH_BANK *b;
  AH_USER *u;
  const char *p;
  const char *localId;

  assert(hmsg);
  b=AH_Dialog_GetBank(hmsg->dialog);
  assert(b);
  u=AH_Customer_GetUser(AH_Dialog_GetDialogOwner(hmsg->dialog));
  assert(u);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Encrypting message");
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  ks=hmsg->crypter;
  if (!ks) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypter");
    return -1;
  }

  /* get medium */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Getting medium for encrypting");
  medium=AH_HBCI_GetMedium(AH_Bank_GetHbci(b),
                           AH_Dialog_GetDialogOwner(hmsg->dialog));
  if (!medium) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not find medium for customer \"%s\"",
             AH_Customer_GetCustomerId(AH_Dialog_GetDialogOwner(hmsg->dialog)));
    /* GWEN_DB_Group_free(cfg); */
    return -1;
  }

  mctx=AH_Medium_GetCurrentContext(medium);
  assert(mctx);

  p=AH_CryptMode_toString(AH_User_GetCryptMode(u));
  GWEN_MsgEngine_SetMode(e, p);

  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         "SEG",
                                         "id",
                                         0,
                                         "CryptHead");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"CryptHead\" not found");
    return -1;
  }

  /* get real keyspec */
  ks=AH_MediumCtx_GetRemoteCryptKeySpec(mctx);
  if (!ks) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No keyspec for remote crypt key");
    /* GWEN_DB_Group_free(cfg); */
    return -1;
  }

  lks=AH_MediumCtx_GetLocalCryptKeySpec(mctx);
  if (!lks) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No keyspec for local crypt key");
    /* GWEN_DB_Group_free(cfg); */
    return -1;
  }
  localId=AH_User_GetUserId(u);
  assert(localId);
  assert(*localId);

  /* encrypt message */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Encrypting message");
  msgkeybuf=GWEN_Buffer_new(0, 128, 0, 1);
  cryptbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(hmsg->buffer)+256,0,1);
  GWEN_Buffer_Rewind(hmsg->buffer);

  if (strcasecmp(GWEN_MsgEngine_GetMode(e), "PINTAN")==0) {
    GWEN_Buffer_AppendBuffer(cryptbuf, hmsg->buffer);
    GWEN_Buffer_AppendString(msgkeybuf, "NOKEYINSSLMODE");
  }
  else {
    /* set expected crypter */
    DBG_DEBUG(AQHBCI_LOGDOMAIN,
             "Expecting \"%s\" to encrypt the response",
             localId);
    AH_Msg_SetExpectedCrypter(hmsg, localId);

    if (AH_Medium_Encrypt(medium, hmsg->buffer, cryptbuf, msgkeybuf)!=
        AH_MediumResultOk) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Error encrypting with medium for user \"%s\"",
               localId);
      GWEN_Buffer_free(cryptbuf);
      GWEN_Buffer_free(msgkeybuf);
      /* GWEN_DB_Group_free(cfg); */
      return -1;
    }
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Encrypting message: done");

  /* create CryptHead */
  hbuf=GWEN_Buffer_new(0, 256+GWEN_Buffer_GetUsedBytes(hmsg->buffer), 0, 1);
  cfg=GWEN_DB_Group_new("crypthead");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", 998);

  if (AH_Msg_PrepareCryptoSeg(hmsg,
                              AH_Dialog_GetDialogOwner(hmsg->dialog),
                              cfg, ks, 1, 0)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* store security id */
  if (strcasecmp(GWEN_MsgEngine_GetMode(e), "RDH")==0) {
    const char *secid;

    if (hmsg->noSysId)
      secid="0";
    else {
      secid=AH_Customer_GetSystemId(AH_Dialog_GetDialogOwner(hmsg->dialog));
      if (!secid)
        secid="0";
    }

    /* RDH mode */
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "SecDetails/SecId", secid);
  }
  else if (strcasecmp(GWEN_MsgEngine_GetMode(e), "DDV")==0) {
    GWEN_BUFFER *idBuf;
    int rv;

    idBuf=GWEN_Buffer_new(0, 128, 0, 1);
    rv=AH_Medium_GetTokenIdData(medium, idBuf);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "No system Id on DDV medium, should not happen (%d)", rv);
      GWEN_Buffer_free(idBuf);
      GWEN_DB_Group_free(cfg);
      return -1;
    }

    /* DDV mode */
    GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                        "SecDetails/SecId",
                        GWEN_Buffer_GetStart(idBuf),
                        GWEN_Buffer_GetUsedBytes(idBuf));
    GWEN_Buffer_free(idBuf);
  }
  else if (strcasecmp(GWEN_MsgEngine_GetMode(e), "PINTAN")==0) {
    const char *secid;

    if (hmsg->noSysId)
      secid="0";
    else {
      secid=AH_Customer_GetSystemId(AH_Dialog_GetDialogOwner(hmsg->dialog));
      if (!secid)
        secid="0";
    }

    /* RDH mode */
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "SecDetails/SecId",
                         secid);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Unknown security mode \"%s\"",
              GWEN_MsgEngine_GetMode(e));
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* store encrypted message key */
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "CryptAlgo/MsgKey",
                      GWEN_Buffer_GetStart(msgkeybuf),
                      GWEN_Buffer_GetUsedBytes(msgkeybuf));
  GWEN_Buffer_free(msgkeybuf);

  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create CryptHead");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  GWEN_DB_Group_free(cfg);

  /* create cryptdata */
  cfg=GWEN_DB_Group_new("cryptdata");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", 999);
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "cryptdata",
                      GWEN_Buffer_GetStart(cryptbuf),
                      GWEN_Buffer_GetUsedBytes(cryptbuf));
  GWEN_Buffer_free(cryptbuf);

  /* get node */
  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         "SEG",
                                         "id",
                                         0,
                                         "CryptData");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"CryptData\"not found");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create CryptData");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  /* replace existing buffer by encrypted one */
  GWEN_Buffer_free(hmsg->buffer);
  hmsg->buffer=hbuf;
  GWEN_DB_Group_free(cfg);

  return 0;
}














/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_PrepareCryptoSegDec(AH_MSG *hmsg,
                               GWEN_DB_NODE *n,
                               int crypt,
                               GWEN_KEYSPEC **keySpec,
                               int *signseq,
                               const char **pSecurityId,
                               int *lSecurityId,
                               GWEN_BUFFER *msgkeybuf) {
  GWEN_KEYSPEC *ks;
  const void *p;
  const char *s;
  unsigned int size;
  GWEN_MSGENGINE *e;
  int rdhMode;
  int pinTanMode;
  AH_BANK *b;

  assert(hmsg);
  assert(n);

  pinTanMode=0;
  rdhMode=0;
  b=AH_Dialog_GetBank(hmsg->dialog);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  ks=GWEN_KeySpec_new();

  /* prepare keyspec */
  GWEN_KeySpec_SetOwner(ks, GWEN_DB_GetCharValue(n, "key/userid", 0, ""));
  GWEN_KeySpec_SetNumber(ks, GWEN_DB_GetIntValue(n, "key/keynum", 0, 0));
  GWEN_KeySpec_SetKeyName(ks, GWEN_DB_GetCharValue(n, "key/keytype", 0, ""));
  GWEN_KeySpec_SetVersion(ks,
                          GWEN_DB_GetIntValue(n, "key/keyversion", 0, 0));
  s=GWEN_DB_GetCharValue(n, "key/keytype", 0, 0);
  if (!s) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "no keytype");
    GWEN_KeySpec_free(ks);
    return -1;
  }
  GWEN_KeySpec_SetKeyName(ks, s);

  *keySpec=ks;

  if (GWEN_Text_Compare(GWEN_DB_GetCharValue(n, "key/bankcode", 0, ""),
                        AH_Bank_GetBankId(b),
                        1)) {
    DBG_WARN(AQHBCI_LOGDOMAIN,
             "BankId in received message does not match (%s!=%s)",
             GWEN_DB_GetCharValue(n, "key/bankcode", 0, ""),
             AH_Bank_GetBankId(b));
    AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(hmsg->dialog),
                           0,
                           AB_Banking_LogLevelWarn,
                           I18N("BankId in received message does not match, "
                                "ignoring"));
  }

  /* get security mode */
  if (crypt) {
    int m;

    m=GWEN_DB_GetIntValue(n, "function", 0, -1);
    if (m==-1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad crypt head");
      return -1;
    }
    if (m==998) {
      /* PINTAN */
      pinTanMode=1;
      GWEN_KeySpec_SetKeyType(ks, "RSA");
      GWEN_MsgEngine_SetMode(e, "PINTAN");
    }
    else {
      /* DDV/RDH */
      assert(msgkeybuf);
      p=GWEN_DB_GetBinValue(n,
                            "CryptAlgo/MsgKey",
                            0,
                            0,0,
                            &size);
      if (p && size) {
	int kt;

	kt=GWEN_DB_GetIntValue(n, "CryptAlgo/keyType", 0, 0);
	/* store messsage key */
	if (size<96 && kt==6) {
          /* RDH mode */
	  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Filling message key (only %d bytes)",
		   size);
	  GWEN_Buffer_FillWithBytes(msgkeybuf, 0, 96-size);
	}
	GWEN_Buffer_AppendBytes(msgkeybuf, p, size);
      }
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Message key missing (HBCI server error)");
        return -1;
      }
      m=GWEN_DB_GetIntValue(n, "CryptAlgo/keytype", 0, -1);
      if (m==-1) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad security mode (non-DDV, non-RDH)");
        return -1;
      }
      if (m==6) {
        rdhMode=1;
        GWEN_KeySpec_SetKeyType(ks, "RSA");
        GWEN_MsgEngine_SetMode(e, "RDH");
      }
      else if (m==5) {
        GWEN_KeySpec_SetKeyType(ks, "DES");
        GWEN_MsgEngine_SetMode(e, "DDV");
      }
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad security mode (non-DDV, non-RDH: %d)", m);
        return -1;
      }
    }
  } /* if crypt */
  else {
    int m;

    m=GWEN_DB_GetIntValue(n, "function", 0, -1);
    if (m==-1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad sign head");
      return -1;
    }
    if (m==999) {
      /* PINTAN */
      GWEN_MsgEngine_SetMode(e, "PINTAN");
      GWEN_KeySpec_SetKeyType(ks, "RSA");
      pinTanMode=1;
    }
    else {
      AH_DIALOG *dlg;
      AH_CUSTOMER *cu;

      dlg=AH_Msg_GetDialog(hmsg);
      assert(dlg);
      cu=AH_Dialog_GetDialogOwner(dlg);
      assert(cu);

      assert(signseq);
      *signseq=GWEN_DB_GetIntValue(n, "signseq", 0, 0);
      if (*signseq==0 && AH_Customer_GetBankUsesSignSeq(cu)) {
	int but;

	DBG_WARN(AQHBCI_LOGDOMAIN,
		 "Invalid signature counter value 0");

	/* check whether the user want's to accept the mal-signed message */
	but=AB_Banking_MessageBox
	  (AH_Dialog_GetBankingApi(dlg),
	   AB_BANKING_MSG_FLAGS_TYPE_WARN |
	   AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
	   AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS,
	   I18N("Security Warning"),
	   I18N(
"The HBCI response of the bank has an invalid signature counter,\n"
"contrary to what has been expected. This can be the case because the \n"
"bank just stopped using the signature counter. This error message \n"
"would also occur if there were a replay attack against your computer \n"
"in progress right now, which is probably quite unlikely. \n"
" \n"
"Please contact your bank and ask them whether their HBCI server \n"
"stopped using the signature counter in HBCI responses. \n"
"\n"
"Do you nevertheless want to accept this response this time or always?"
"<html><p>"
"The HBCI response of the bank has an invalid signature counter, "
"contrary to what has been expected. This can be the case because the\n"
"bank just stopped using the signature counter. This error message\n"
"would also occur if there were a replay attack against your computer\n"
"in progress right now, which is probably quite unlikely.\n"
"</p><p>"
"Please contact your bank and ask them whether their HBCI server\n"
"stopped using the signature counter in HBCI responses.\n"
"</p><p>"
"Do you nevertheless want to accept this response this time or always?"
"</p></html>"
),
	   I18N("Accept this time"),
	   I18N("Accept always"),
	   I18N("Abort"));
	if (but==1) {
	  AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
				 0,
				 AB_Banking_LogLevelNotice,
				 I18N("User accepts this response "
				      "with an invalid signature counter"));
	}
	else if (but==2) {
	  AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
				 0,
				 AB_Banking_LogLevelNotice,
				 I18N("User accepts all further responses "
				      "with an invalid signature counter"));
	  AH_Customer_SetBankUsesSignSeq(cu, 0);
	}
	else {
	  AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
				 0,
				 AB_Banking_LogLevelError,
				 I18N("Aborted"));
	  return AB_ERROR_SECURITY;
	}
      }

      m=GWEN_DB_GetIntValue(n, "SignAlgo/algo", 0, -1);
      if (m==-1) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad security mode (non-DDV, non-RDH)");
        return -1;
      }
      if (m==10) {
        rdhMode=1;
        GWEN_KeySpec_SetKeyType(ks, "RSA");
        GWEN_MsgEngine_SetMode(e, "RDH");
      }
      else if (m==1) {
        GWEN_KeySpec_SetKeyType(ks, "DES");
        GWEN_MsgEngine_SetMode(e, "DDV");
      }
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad security mode (non-DDV, non-RDH: %d)", m);
        return -1;
      }
    }
  }

  /* store security id */
  if (rdhMode || pinTanMode) {
    /* RDH or PIN/TAN mode */
    s=GWEN_DB_GetCharValue(n,
                           "SecDetails/SecId",
                           0,
                           0);
    if (s) {
      *pSecurityId=s;
      *lSecurityId=strlen(s);
    }
  }
  else {
    /* DDV mode */
    p=GWEN_DB_GetBinValue(n,
                          "SecDetails/SecId",
                          0,
                          0,0,
                          &size);
    if (p) {
      *pSecurityId=p;
      *lSecurityId=size;
    }
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_Decrypt(AH_MSG *hmsg, GWEN_DB_NODE *gr){
  const void *p;
  unsigned int size;
  GWEN_DB_NODE *nhead=0;
  GWEN_DB_NODE *ndata=0;
  GWEN_BUFFER *cdbuf=0;
  GWEN_BUFFER *ndbuf=0;
  GWEN_BUFFER *msgkeybuf=0;
  AH_MEDIUM_RESULT rv;
  GWEN_KEYSPEC *ks=0;
  const char *pSecurityId=0;
  int lSecurityId;
  AH_MEDIUM *medium=0;
  AH_MEDIUM_CTX *mctx;
  AH_BANK *b;
  AH_CUSTOMER *cu;
  AH_USER *u;
  const char *s;

  assert(hmsg);
  b=AH_Dialog_GetBank(hmsg->dialog);
  cu=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(cu);
  u=AH_Customer_GetUser(cu);
  assert(u);

  /* decrypt */
  nhead=GWEN_DB_GetGroup(gr,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "CryptHead");
  if (!nhead) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt head");
    return -1;
  }

  ndata=GWEN_DB_GetGroup(gr,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                         "CryptData");
  if (!ndata) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt data");
    return -1;
  }

  msgkeybuf=GWEN_Buffer_new(0, 128, 0, 1);
  if (AH_Msg_PrepareCryptoSegDec(hmsg,
                                 nhead,
                                 1,
                                 &ks,
                                 0,
                                 &pSecurityId,
                                 &lSecurityId,
                                 msgkeybuf)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_Buffer_free(msgkeybuf);
    return -1;
  }

  /* get medium */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Getting medium for decrypting");
  medium=AH_HBCI_GetMedium(AH_Bank_GetHbci(b),
                           AH_Dialog_GetDialogOwner(hmsg->dialog));
  if (!medium) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not find medium for user \"%s\"",
             GWEN_KeySpec_GetOwner(ks));
    GWEN_Buffer_free(msgkeybuf);
    GWEN_KeySpec_free(ks);
    return -1;
  }

  mctx=AH_Medium_GetCurrentContext(medium);
  assert(mctx);

  s=AH_CryptMode_toString(AH_User_GetCryptMode(u));
  GWEN_MsgEngine_SetMode(AH_Dialog_GetMsgEngine(hmsg->dialog), s);

  p=GWEN_DB_GetBinValue(ndata,
                        "CryptData",
                        0,
                        0,0,
                        &size);
  if (!p) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt data");
    GWEN_Buffer_free(msgkeybuf);
    GWEN_KeySpec_free(ks);
    return -1;
  }

  cdbuf=GWEN_Buffer_new((void*)p, size, size, 0);
  GWEN_Buffer_SetMode(cdbuf, 0); /* no dynamic mode ! */

  ndbuf=GWEN_Buffer_new(0, AH_MSG_DEFAULTSIZE, 0, 1);
  GWEN_Buffer_SetStep(ndbuf, AH_MSG_DEFAULTSTEP);

  if (AH_User_GetCryptMode(u)==AH_CryptMode_Pintan) {
    GWEN_Buffer_AppendBuffer(ndbuf, cdbuf);
  }
  else {
    rv=AH_Medium_Decrypt(medium,
                         cdbuf,
                         ndbuf,
                         msgkeybuf);
    if (rv!=AH_MediumResultOk) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Error decrypting with medium for user \"%s\"",
               GWEN_KeySpec_GetOwner(ks));
      GWEN_Buffer_free(cdbuf);
      GWEN_Buffer_free(ndbuf);
      GWEN_Buffer_free(msgkeybuf);
      GWEN_KeySpec_free(ks);
      return -1;
    }
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Decrypting done");

  /* store new buffer inside message */
  GWEN_Buffer_free(hmsg->origbuffer);
  hmsg->origbuffer=hmsg->buffer;
  GWEN_Buffer_Rewind(ndbuf);
  hmsg->buffer=ndbuf;

  /* store crypter */
  AH_Msg_SetCrypter(hmsg, ks);

  GWEN_Buffer_free(cdbuf);
  GWEN_Buffer_free(msgkeybuf);
  GWEN_KeySpec_free(ks);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_Verify(AH_MSG *hmsg,
                  GWEN_DB_NODE *gr,
                  unsigned int flags) {
  GWEN_LIST *sigheads;
  GWEN_LIST *sigtails;
  GWEN_DB_NODE *n;
  int nonSigHeads;
  int nSigheads;
  unsigned int dataBegin;
  char *dataStart;
  unsigned int dataLength;
  unsigned int i;
  AH_BANK *b;
  AH_CUSTOMER *cu;
  AH_USER *u;

  assert(hmsg);
  b=AH_Dialog_GetBank(hmsg->dialog);
  cu=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(cu);
  u=AH_Customer_GetUser(cu);
  assert(u);

  sigheads=GWEN_List_new();

  /* enumerate signature heads */
  nonSigHeads=0;
  nSigheads=0;
  n=GWEN_DB_GetFirstGroup(gr);
  while(n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigHead")==0) {
      /* found a signature head */
      if (nonSigHeads) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Found some unsigned parts at the beginning");
        GWEN_List_free(sigheads);
        return AB_ERROR_BAD_DATA;
      }
      GWEN_List_PushBack(sigheads, n);
      nSigheads++;
    }
    else if (strcasecmp(GWEN_DB_GroupName(n), "MsgHead")!=0) {
      if (nSigheads)
        break;
      nonSigHeads++;
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    if (nSigheads) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Found Signature heads but no other segments");
      GWEN_List_free(sigheads);
      return AB_ERROR_BAD_DATA;
    }
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No signatures");
    GWEN_List_free(sigheads);
    return 0;
  }

  /* store begin of signed data */
  dataBegin=GWEN_DB_GetIntValue(n, "segment/pos", 0, 0);
  if (!dataBegin) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No position specifications in segment");
    GWEN_List_free(sigheads);
    return AB_ERROR_BAD_DATA;
  }

  /* now get first signature tail */
  while(n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigTail")==0) {
      unsigned int currpos;

      /* found a signature tail */
      currpos=GWEN_DB_GetIntValue(n, "segment/pos", 0, 0);
      if (!currpos || dataBegin>currpos) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad position specification in Signature tail");
        GWEN_List_free(sigheads);
        return AB_ERROR_BAD_DATA;
      }
      dataLength=currpos-dataBegin;
      break;
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No signature tail found");
    GWEN_List_free(sigheads);
    return AB_ERROR_BAD_DATA;
  }

  sigtails=GWEN_List_new();
  while(n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigTail")!=0)
      break;
    GWEN_List_PushBack(sigtails, n);
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Message tail expected");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return AB_ERROR_BAD_DATA;
  }

  if (strcasecmp(GWEN_DB_GroupName(n), "MsgTail")!=0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unexpected segment (msg tail expected)");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return AB_ERROR_BAD_DATA;
  }

  n=GWEN_DB_GetNextGroup(n);
  if (n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unexpected segment (end expected)");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return AB_ERROR_BAD_DATA;
  }

  if (GWEN_List_GetSize(sigheads)!=
      GWEN_List_GetSize(sigtails)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Number of signature heads (%d) does not match "
              "number of signature tails (%d)",
              GWEN_List_GetSize(sigheads),
              GWEN_List_GetSize(sigtails));
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return AB_ERROR_BAD_DATA;
  }

  /* ok, now verify all signatures */
  dataStart=GWEN_Buffer_GetStart(hmsg->buffer)+dataBegin;
  for (i=0; i< GWEN_List_GetSize(sigtails); i++) {
    GWEN_DB_NODE *sighead;
    GWEN_DB_NODE *sigtail;
    GWEN_BUFFER *dbuf;
    GWEN_BUFFER *sigbuf=0;
    const void *p;
    unsigned int size;
    GWEN_KEYSPEC *ks=0;
    int signSeq;
    const char *pSecurityId=0;
    int lSecurityId;
    AH_MEDIUM *medium=0;
    AH_MEDIUM_CTX *mctx;
    AH_MEDIUM_RESULT mres;

    /* get signature tail */
    sigtail=(GWEN_DB_NODE*)GWEN_List_GetBack(sigtails);

    /* get corresponding signature head */
    sighead=(GWEN_DB_NODE*)GWEN_List_GetFront(sigheads);

    if (!sighead || !sigtail) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No signature head/tail left (internal error)");
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return AB_ERROR_BAD_DATA;
    }

    GWEN_List_PopBack(sigtails);
    GWEN_List_PopFront(sigheads);

    /* some checks */
    if (strcasecmp(GWEN_DB_GetCharValue(sighead, "ctrlref", 0, ""),
                   GWEN_DB_GetCharValue(sigtail, "ctrlref", 0, ""))!=0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Non-matching signature tail");
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return AB_ERROR_BAD_DATA;
    }

    /* prepare data buffer */
    dbuf=GWEN_Buffer_new(0,
                         dataLength+GWEN_DB_GetIntValue(sighead,
                                                        "segment/length",
                                                        0,
                                                        0),
                         0,1);
    GWEN_Buffer_AppendBytes(dbuf,
                            GWEN_Buffer_GetStart(hmsg->buffer)+
                            GWEN_DB_GetIntValue(sighead,
                                                "segment/pos",
                                                0,
                                                0),
                            GWEN_DB_GetIntValue(sighead,
                                                "segment/length",
                                                0,
                                                0));
    GWEN_Buffer_AppendBytes(dbuf, dataStart, dataLength);

    /* prepare signature buffer */
    p=GWEN_DB_GetBinValue(sigtail, "signature", 0, 0, 0, &size);
    if (!p) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "No signature");
    }

    GWEN_Buffer_Rewind(dbuf);

    if (p && size) {
      sigbuf=GWEN_Buffer_new((char*)p, size, size, 0);
      GWEN_Buffer_SetMode(sigbuf, 0);
    }

    if (AH_Msg_PrepareCryptoSegDec(hmsg,
                                   sighead,
                                   0,
                                   &ks,
                                   &signSeq,
                                   &pSecurityId,
                                   &lSecurityId,
                                   0)) {
      GWEN_Buffer_free(sigbuf);
      GWEN_Buffer_free(dbuf);
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      GWEN_KeySpec_free(ks);
      DBG_INFO(AQHBCI_LOGDOMAIN, "here");
      return AB_ERROR_BAD_DATA;
    }

    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Got sign seq %d", signSeq);

    /* get medium */
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Getting medium for verification");
    medium=AH_HBCI_GetMedium(AH_Bank_GetHbci(b), cu);
    assert(medium);
    mctx=AH_Medium_GetCurrentContext(medium);
    assert(mctx);
    /* verify signature */
    if (AH_User_GetCryptMode(u)==AH_CryptMode_Pintan) {
      /* PIN/TAN mode */
      GWEN_Buffer_free(sigbuf);
      GWEN_Buffer_free(dbuf);
    } /* if PIN/TAN */
    else {
      /* DDV or RDH mode */
      GWEN_Buffer_Rewind(dbuf);

      if (GWEN_Buffer_GetUsedBytes(sigbuf)==0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No signature");
        GWEN_List_free(sigheads);
        GWEN_List_free(sigtails);
        GWEN_KeySpec_free(ks);
        return AB_ERROR_BAD_DATA;
      }

      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Checking with sign seq %d", signSeq);
      /* TODO: Extend signature counter width and really check it */
      mres=AH_Medium_Verify(medium, dbuf, sigbuf, 0);
      GWEN_Buffer_free(sigbuf);
      GWEN_Buffer_free(dbuf);

      if (mres==AH_MediumResultOk) {
        /* signature ok, add signer */
        AH_Msg_AddSigner(hmsg, ks);
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Signature is valid");
        GWEN_DB_SetIntValue(sighead,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "segment/error/code",
                            10);
        GWEN_DB_SetCharValue(sighead,
                             GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "segment/error/text",
                             "Signierer bekannt");

        GWEN_DB_SetIntValue(sigtail,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "segment/error/code",
                            10);
        GWEN_DB_SetCharValue(sigtail,
                             GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "segment/error/text",
                             "Signatur ist gueltig");
      }
      else if (mres==AH_MediumResultNoKey) {
        char *nb;
        GWEN_KEYSPEC *nks;

        DBG_WARN(AQHBCI_LOGDOMAIN, "No key to verify signature");

        nks=GWEN_KeySpec_dup(ks);
        p=GWEN_KeySpec_GetOwner(nks);
        assert(p);
        nb=(char*)malloc(strlen(p)+2);
        assert(nb);
        nb[0]='~';
        nb[1]=0;
        strcat(nb, p);
        GWEN_KeySpec_SetOwner(nks, nb);
        AH_Msg_AddSigner(hmsg, nks);
        GWEN_KeySpec_free(nks);
        free(nb);

        GWEN_DB_SetIntValue(sigtail,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "segment/error/code",
                            3340);
        GWEN_DB_SetCharValue(sigtail,
                             GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "segment/error/text",
                             "Unbekannter Signierer");
      }
      else {
        char *nb;
        GWEN_KEYSPEC *nks;

        if (mres==AH_MediumResultSignSeq) {
          DBG_WARN(AQHBCI_LOGDOMAIN, "Double use of signature");
          GWEN_DB_SetIntValue(sigtail,
                              GWEN_DB_FLAGS_OVERWRITE_VARS,
                              "segment/error/code",
                              9390);
          GWEN_DB_SetCharValue(sigtail,
                               GWEN_DB_FLAGS_OVERWRITE_VARS,
                               "segment/error/text",
                               "Doppeleinreichung");
        }
        else if (mres==AH_MediumResultInvalidSignature) {
          DBG_WARN(AQHBCI_LOGDOMAIN, "Invalid signature");
          GWEN_DB_SetIntValue(sigtail,
                              GWEN_DB_FLAGS_OVERWRITE_VARS,
                              "segment/error/code",
                              9340);
          GWEN_DB_SetCharValue(sigtail,
                               GWEN_DB_FLAGS_OVERWRITE_VARS,
                               "segment/error/text",
                               "Elektronische Signatur falsch");
        }
        else {
          DBG_WARN(AQHBCI_LOGDOMAIN, "Error in security medium");
          GWEN_DB_SetIntValue(sigtail,
                              GWEN_DB_FLAGS_OVERWRITE_VARS,
                              "segment/error/code",
                              9400);
          GWEN_DB_SetCharValue(sigtail,
                               GWEN_DB_FLAGS_OVERWRITE_VARS,
                               "segment/error/text",
                               "Allgemeiner Fehler des Sicherheitsmediums");
        }

        nks=GWEN_KeySpec_dup(ks);
        p=GWEN_KeySpec_GetOwner(nks);
        assert(p);
        nb=(char*)malloc(strlen(p)+2);
        assert(nb);
        nb[0]='!';
        nb[1]=0;
        strcat(nb, p);
        GWEN_KeySpec_SetOwner(nks, nb);
        AH_Msg_AddSigner(hmsg, nks);
        GWEN_KeySpec_free(nks);
        free(nb);
      }
    } /* if not PIN/TAN */
    GWEN_KeySpec_free(ks);
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Verification done");
  } /* for */

  GWEN_List_free(sigheads);
  GWEN_List_free(sigtails);
  return 0;
}







