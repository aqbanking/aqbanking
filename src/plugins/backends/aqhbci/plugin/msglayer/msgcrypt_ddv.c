/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: msgcrypt.inc 1109 2007-01-10 14:30:14Z martin $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



int AH_MsgDdv_PrepareCryptoSeg(AH_MSG *hmsg,
			       AB_USER *u,
			       const GWEN_CRYPT_TOKEN_KEYINFO *ki,
			       GWEN_DB_NODE *cfg,
			       int crypt,
			       int createCtrlRef) {
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
  if (!peerId || !*peerId)
    peerId=userId;

  tt=time(0);
  lt=localtime(&tt);

  if (createCtrlRef) {
    /* create control reference */
    if (!strftime(ctrlref, sizeof(ctrlref),
                  "%Y%m%d%H%M%S", lt)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "CtrlRef string too long");
      return GWEN_ERROR_INTERNAL;
    }

    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "ctrlref", ctrlref);
  }

  /* create date */
  if (!strftime(sdate, sizeof(sdate),
                "%Y%m%d", lt)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Date string too long");
    return GWEN_ERROR_INTERNAL;
  }
  /* create time */
  if (!strftime(stime, sizeof(stime),
                "%H%M%S", lt)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Date string too long");
    return GWEN_ERROR_INTERNAL;
  }

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		      "SecDetails/dir", 1);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "SecStamp/date", sdate);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "SecStamp/time", stime);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "key/bankcode",
                       AB_User_GetBankCode(u));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "key/userid",
                       crypt?peerId:userId);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "key/keytype",
                       crypt?"V":"S");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		      "key/keynum",
		      GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		      "key/keyversion",
		      GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		       "secProfile/code",
		       "DDV");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		      "secProfile/version",
		      1);

  return 0;
}




int AH_Msg_SignDdv(AH_MSG *hmsg,
		   GWEN_BUFFER *rawBuf,
		   const char *signer) {
  AH_HBCI *h;
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *sigbuf;
  GWEN_BUFFER *hbuf;
  unsigned int l;
  int rv;
  char ctrlref[15];
  const char *p;
  GWEN_MSGENGINE *e;
  AB_USER *su;
  uint32_t uFlags;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  uint32_t gid;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  GWEN_MsgEngine_SetMode(e, "ddv");

  gid=0;

  su=AB_Banking_FindUser(AH_HBCI_GetBankingApi(h),
			 AH_PROVIDER_NAME,
			 "de", "*",
			 signer, "*");
  if (!su) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "Unknown user \"%s\"",
	      signer);
    return GWEN_ERROR_NOT_FOUND;
  }

  uFlags=AH_User_GetFlags(su);

  /* get crypt token of signer */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
			      AH_User_GetTokenType(su),
			      AH_User_GetTokenName(su),
			      &ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Could not get crypt token for user \"%s\" (%d)",
	     AB_User_GetUserId(su), rv);
    return rv;
  }

  /* open CryptToken if necessary */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    rv=GWEN_Crypt_Token_Open(ct, 0, gid);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
	       "Could not open crypt token for user \"%s\" (%d)",
	       AB_User_GetUserId(su), rv);
      return rv;
    }
  }

  /* get context and key info */
  ctx=GWEN_Crypt_Token_GetContext(ct,
				  AH_User_GetTokenContextId(su),
				  gid);
  if (ctx==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Context %d not found on crypt token [%s:%s]",
	     AH_User_GetTokenContextId(su),
	     GWEN_Crypt_Token_GetTypeName(ct),
	     GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

  keyId=GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
  ki=GWEN_Crypt_Token_GetKeyInfo(ct,
                                 keyId,
				 0xffffffff,
				 gid);
  if (ki==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Keyinfo %04x not found on crypt token [%s:%s]",
	     keyId,
	     GWEN_Crypt_Token_GetTypeName(ct),
	     GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e,
						    "SEG",
						    "id",
						    0,
						    "SigHead");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"SigHead\" not found");
    return GWEN_ERROR_INTERNAL;
  }

  /* prepare config for segment */
  cfg=GWEN_DB_Group_new("sighead");
  rv=AH_MsgDdv_PrepareCryptoSeg(hmsg, su, ki, cfg, 0, 1);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(cfg);
    return rv;
  }

  /* set expected signer */
  if (!(uFlags & AH_USER_FLAGS_BANK_DOESNT_SIGN)) {
    const char *remoteId;

    remoteId=AH_User_GetPeerId(su);
    if (!remoteId || *remoteId==0)
      remoteId=AB_User_GetUserId(su);
    assert(remoteId);
    assert(*remoteId);

    DBG_DEBUG(AQHBCI_LOGDOMAIN,
	      "Expecting \"%s\" to sign the response",
	      remoteId);
    AH_Msg_SetExpectedSigner(hmsg, remoteId);
  }

  /* store system id */
  p=GWEN_Crypt_Token_Context_GetSystemId(ctx);
  if (p) {
    GWEN_BUFFER *idBuf;

    idBuf=GWEN_Buffer_new(0, 128, 0, 1);
    rv=GWEN_Text_FromHexBuffer(p, idBuf);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"Bad system id on DDV medium, should not happen (%d)", rv);
      GWEN_Buffer_free(idBuf);
      GWEN_DB_Group_free(cfg);
      return rv;
    }

    GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
			"SecDetails/SecId",
			GWEN_Buffer_GetStart(idBuf),
			GWEN_Buffer_GetUsedBytes(idBuf));
    GWEN_Buffer_free(idBuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "No system id on DDV medium, should not happen (%d)", rv);
    GWEN_DB_Group_free(cfg);
    return GWEN_ERROR_INTERNAL;
  }

  /* retrieve control reference for sigtail (to be used later) */
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
                      GWEN_Crypt_Token_KeyInfo_GetSignCounter(ki));

  /* create signature head segment */
  rv=GWEN_MsgEngine_CreateMessageFromNode(e, node, hbuf, cfg);
  GWEN_DB_Group_free(cfg);
  cfg=0;
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create SigHead");
    GWEN_Buffer_free(hbuf);
    return rv;
  }

  /* remember size of sighead for now */
  l=GWEN_Buffer_GetUsedBytes(hbuf);

  /* add raw data to to-sign data buffer */
  GWEN_Buffer_AppendBuffer(hbuf, rawBuf);

  /* sign message */
  sigbuf=GWEN_Buffer_new(0, 512, 0, 1);
  if (1) {
    uint32_t signLen;
    GWEN_CRYPT_PADDALGO *algo;
    GWEN_MDIGEST *md;
    uint32_t seq;

    /* hash sighead + data */
    md=GWEN_MDigest_Rmd160_new();
    rv=GWEN_MDigest_Begin(md);
    if (rv==0)
      rv=GWEN_MDigest_Update(md,
			     (uint8_t*)GWEN_Buffer_GetStart(hbuf),
			     GWEN_Buffer_GetUsedBytes(hbuf));
    if (rv==0)
      rv=GWEN_MDigest_End(md);
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Hash error (%d)", rv);
      GWEN_MDigest_free(md);
      GWEN_Buffer_free(sigbuf);
      GWEN_Buffer_free(hbuf);
      return rv;
    }

    /* sign hash */
    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_None);
    signLen=20;
    rv=GWEN_Crypt_Token_Sign(ct, keyId,
			     algo,
			     GWEN_MDigest_GetDigestPtr(md),
			     GWEN_MDigest_GetDigestSize(md),
			     (uint8_t*)GWEN_Buffer_GetPosPointer(sigbuf),
			     &signLen,
			     &seq,
			     gid);
    GWEN_Crypt_PaddAlgo_free(algo);
    GWEN_MDigest_free(md);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"Could not sign data with medium of user \"%s\" (%d)",
		AB_User_GetUserId(su), rv);
      GWEN_Buffer_free(sigbuf);
      GWEN_Buffer_free(hbuf);
      return rv;
    }
    GWEN_Buffer_IncrementPos(sigbuf, signLen);
    GWEN_Buffer_AdjustUsedBytes(sigbuf);
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Signing done");

  /* insert new SigHead at beginning of message buffer */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Inserting signature head");
  GWEN_Buffer_Rewind(hmsg->buffer);
  GWEN_Buffer_InsertBytes(hmsg->buffer, GWEN_Buffer_GetStart(hbuf), l);

  /* create sigtail */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Completing signature tail");
  cfg=GWEN_DB_Group_new("sigtail");
  GWEN_Buffer_Reset(hbuf);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", hmsg->lastSegment+1);
  /* store to DB */
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "signature",
                      GWEN_Buffer_GetStart(sigbuf),
                      GWEN_Buffer_GetUsedBytes(sigbuf));
  GWEN_Buffer_free(sigbuf);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		       "ctrlref", ctrlref);

  /* get node */
  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e,
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
  rv=GWEN_MsgEngine_CreateMessageFromNode(e, node, hbuf, cfg);
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



int AH_Msg_EncryptDdv(AH_MSG *hmsg) {
  AH_HBCI *h;
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *mbuf;
  GWEN_BUFFER *hbuf;
  uint32_t l;
  int rv;
  const char *p;
  GWEN_MSGENGINE *e;
  AB_USER *u;
  const char *peerId;
//  uint32_t uFlags;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  GWEN_CRYPT_KEY *sk;
  uint8_t encKey[16];
  uint32_t gid;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  GWEN_MsgEngine_SetMode(e, "ddv");

  gid=0;

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
//  uFlags=AH_User_GetFlags(u);

  peerId=AH_User_GetPeerId(u);
  if (!peerId || *peerId==0)
    peerId=AB_User_GetUserId(u);

  /* get crypt token of signer */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
			   AH_User_GetTokenType(u),
			   AH_User_GetTokenName(u),
			   &ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Could not get crypt token for user \"%s\" (%d)",
	     AB_User_GetUserId(u), rv);
    return rv;
  }

  /* open CryptToken if necessary */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    rv=GWEN_Crypt_Token_Open(ct, 0, gid);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
	       "Could not open crypt token for user \"%s\" (%d)",
	       AB_User_GetUserId(u), rv);
      return rv;
    }
  }

  /* get context and key info */
  ctx=GWEN_Crypt_Token_GetContext(ct,
				  AH_User_GetTokenContextId(u),
				  gid);
  if (ctx==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Context %d not found on crypt token [%s:%s]",
	     AH_User_GetTokenContextId(u),
	     GWEN_Crypt_Token_GetTypeName(ct),
	     GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

  keyId=GWEN_Crypt_Token_Context_GetEncipherKeyId(ctx);
  ki=GWEN_Crypt_Token_GetKeyInfo(ct,
                                 keyId,
				 0xffffffff,
				 gid);
  if (ki==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Keyinfo %04x not found on crypt token [%s:%s]",
	     keyId,
	     GWEN_Crypt_Token_GetTypeName(ct),
	     GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

  rv=GWEN_Padd_PaddWithAnsiX9_23(hmsg->buffer);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Error padding message with ANSI X9.23 (%d)", rv);
    return rv;
  }

  /* create session key */
  sk=GWEN_Crypt_KeyDes3K_Generate(GWEN_Crypt_CryptMode_Cbc, 24, 2);
  if (sk==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not generate DES key");
    return GWEN_ERROR_INTERNAL;
  }

  /* encrypt message with that session key */
  mbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(hmsg->buffer), 0, 1);
  l=GWEN_Buffer_GetUsedBytes(hmsg->buffer);
  rv=GWEN_Crypt_Key_Encipher(sk,
			     (uint8_t*)GWEN_Buffer_GetStart(hmsg->buffer),
			     GWEN_Buffer_GetUsedBytes(hmsg->buffer),
			     (uint8_t*)GWEN_Buffer_GetPosPointer(mbuf),
			     &l);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Could not encipher with DES session key (%d)",
	     rv);
    GWEN_Buffer_free(mbuf);
    GWEN_Crypt_Key_free(sk);
    return rv;
  }
  GWEN_Buffer_IncrementPos(mbuf, l);
  GWEN_Buffer_AdjustUsedBytes(mbuf);

  /* encrypt session key */
  if (1) {
    uint32_t elen;
    GWEN_CRYPT_PADDALGO *algo;

    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_None);
    elen=sizeof(encKey);
    rv=GWEN_Crypt_Token_Encipher(ct,
				 keyId,
				 algo,
				 GWEN_Crypt_KeyDes3K_GetKeyDataPtr(sk),
				 16,
				 encKey,
				 &elen,
				 gid);
    GWEN_Crypt_PaddAlgo_free(algo);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(mbuf);
      GWEN_Crypt_Key_free(sk);
    }
  }
  GWEN_Crypt_Key_free(sk);

  /* create crypt head */
  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e,
						    "SEG",
						    "id",
						    0,
						    "CryptHead");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"CryptHead\" not found");
    GWEN_Buffer_free(mbuf);
    GWEN_Crypt_Key_free(sk);
    return GWEN_ERROR_INTERNAL;
  }

  /* create CryptHead */
  cfg=GWEN_DB_Group_new("crypthead");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		      "head/seq", 998);

  rv=AH_MsgDdv_PrepareCryptoSeg(hmsg, u, ki, cfg, 1, 0);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(mbuf);
    return rv;
  }

  /* store system id */
  p=GWEN_Crypt_Token_Context_GetSystemId(ctx);
  if (p) {
    GWEN_BUFFER *idBuf;

    idBuf=GWEN_Buffer_new(0, 128, 0, 1);
    rv=GWEN_Text_FromHexBuffer(p, idBuf);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"Bad system id on DDV medium, should not happen (%d)", rv);
      GWEN_Buffer_free(idBuf);
      GWEN_DB_Group_free(cfg);
      GWEN_Buffer_free(mbuf);
      return rv;
    }

    GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
			"SecDetails/SecId",
			GWEN_Buffer_GetStart(idBuf),
			GWEN_Buffer_GetUsedBytes(idBuf));
    GWEN_Buffer_free(idBuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "No system id on DDV medium, should not happen (%d)", rv);
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(mbuf);
    return GWEN_ERROR_INTERNAL;
  }

  /* store encrypted message key */
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		      "CryptAlgo/MsgKey",
                      encKey,
		      16);

  hbuf=GWEN_Buffer_new(0, 256+GWEN_Buffer_GetUsedBytes(mbuf), 0, 1);
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create CryptHead (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(mbuf);
    return rv;
  }
  GWEN_DB_Group_free(cfg);

  /* create cryptdata */
  cfg=GWEN_DB_Group_new("cryptdata");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", 999);
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "cryptdata",
		      GWEN_Buffer_GetStart(mbuf),
                      GWEN_Buffer_GetUsedBytes(mbuf));
  GWEN_Buffer_free(mbuf);

  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e,
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
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create CryptData (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return rv;
  }

  /* replace existing buffer by encrypted one */
  GWEN_Buffer_free(hmsg->buffer);
  hmsg->buffer=hbuf;
  GWEN_DB_Group_free(cfg);

  return 0;
}




int AH_Msg_DecryptDdv(AH_MSG *hmsg, GWEN_DB_NODE *gr){
  AH_HBCI *h;
  GWEN_BUFFER *mbuf;
  uint32_t l;
  int rv;
  const char *p;
  GWEN_MSGENGINE *e;
  AB_USER *u;
  const char *peerId;
//  uint32_t uFlags;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  GWEN_CRYPT_KEY *sk=NULL;
  uint8_t decKey[16];
  GWEN_DB_NODE *nhead=NULL;
  GWEN_DB_NODE *ndata=NULL;
  const char *crypterId;
  uint32_t gid;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  GWEN_MsgEngine_SetMode(e, "ddv");

  gid=0;

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
//  uFlags=AH_User_GetFlags(u);

  peerId=AH_User_GetPeerId(u);
  if (!peerId || *peerId==0)
    peerId=AB_User_GetUserId(u);

  /* get crypt token of signer */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
			   AH_User_GetTokenType(u),
			   AH_User_GetTokenName(u),
			   &ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Could not get crypt token for user \"%s\" (%d)",
	     AB_User_GetUserId(u), rv);
    return rv;
  }

  /* open CryptToken if necessary */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    rv=GWEN_Crypt_Token_Open(ct, 0, gid);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
	       "Could not open crypt token for user \"%s\" (%d)",
	       AB_User_GetUserId(u), rv);
      return rv;
    }
  }

  /* get context and key info */
  ctx=GWEN_Crypt_Token_GetContext(ct,
				  AH_User_GetTokenContextId(u),
				  gid);
  if (ctx==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Context %d not found on crypt token [%s:%s]",
	     AH_User_GetTokenContextId(u),
	     GWEN_Crypt_Token_GetTypeName(ct),
	     GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

  keyId=GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx);
  ki=GWEN_Crypt_Token_GetKeyInfo(ct,
                                 keyId,
				 0xffffffff,
				 gid);
  if (ki==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Keyinfo %04x not found on crypt token [%s:%s]",
	     keyId,
	     GWEN_Crypt_Token_GetTypeName(ct),
	     GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

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

  p=GWEN_DB_GetBinValue(nhead,
			"CryptAlgo/MsgKey",
			0,
			0,0,
			&l);
  if (p && l) {
    uint32_t elen;
    GWEN_CRYPT_PADDALGO *algo;

    algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_None);
    elen=sizeof(decKey);
    rv=GWEN_Crypt_Token_Decipher(ct,
				 keyId,
				 algo,
				 (const uint8_t*)p, l,
				 decKey,
				 &elen,
				 gid);
    GWEN_Crypt_PaddAlgo_free(algo);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    sk=GWEN_Crypt_KeyDes3K_fromData(GWEN_Crypt_CryptMode_Cbc, 24,
				    decKey, elen);
    if (sk==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create DES key from data");
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing message key");
    return GWEN_ERROR_BAD_DATA;
  }

  /* get encrypted data */
  p=GWEN_DB_GetBinValue(ndata,
			"CryptData",
                        0,
                        0,0,
			&l);
  if (!p || !l) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt data");
    GWEN_Crypt_Key_free(sk);
    return GWEN_ERROR_BAD_DATA;
  }

  /* decipher message with session key */
  mbuf=GWEN_Buffer_new(0, l, 0, 1);
  rv=GWEN_Crypt_Key_Decipher(sk,
			     (const uint8_t*)p, l,
			     (uint8_t*)GWEN_Buffer_GetPosPointer(mbuf),
			     &l);
  GWEN_Crypt_Key_free(sk);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Could not decipher with DES session key (%d)",
	     rv);
    GWEN_Buffer_free(mbuf);
    return rv;
  }
  GWEN_Buffer_IncrementPos(mbuf, l);
  GWEN_Buffer_AdjustUsedBytes(mbuf);

  /* unpadd message */
  rv=GWEN_Padd_UnpaddWithAnsiX9_23(mbuf);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Error unpadding message with ANSI X9.23 (%d)", rv);
    GWEN_Buffer_free(mbuf);
    return rv;
  }

  /* store crypter id */
  AH_Msg_SetCrypterId(hmsg, crypterId);

  /* store new buffer inside message */
  GWEN_Buffer_free(hmsg->origbuffer);
  hmsg->origbuffer=hmsg->buffer;
  GWEN_Buffer_Rewind(mbuf);
  hmsg->buffer=mbuf;

  return 0;
}



int AH_Msg_VerifyDdv(AH_MSG *hmsg, GWEN_DB_NODE *gr) {
  AH_HBCI *h;
  GWEN_LIST *sigheads;
  GWEN_LIST *sigtails;
  GWEN_DB_NODE *n;
  int nonSigHeads;
  int nSigheads;
  unsigned int dataBegin;
  char *dataStart;
  unsigned int dataLength;
  unsigned int i;
  AB_USER *u;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  int rv;
  uint32_t gid;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(u);

  gid=0;

  /* get crypt token of signer */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
			   AH_User_GetTokenType(u),
			   AH_User_GetTokenName(u),
			   &ct);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Could not get crypt token for user \"%s\" (%d)",
	     AB_User_GetUserId(u), rv);
    return rv;
  }

  /* open CryptToken if necessary */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    rv=GWEN_Crypt_Token_Open(ct, 0, gid);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
	       "Could not open crypt token for user \"%s\" (%d)",
	       AB_User_GetUserId(u), rv);
      return rv;
    }
  }

  /* get context and key info */
  ctx=GWEN_Crypt_Token_GetContext(ct,
				  AH_User_GetTokenContextId(u),
				  gid);
  if (ctx==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Context %d not found on crypt token [%s:%s]",
	     AH_User_GetTokenContextId(u),
	     GWEN_Crypt_Token_GetTypeName(ct),
	     GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

  keyId=GWEN_Crypt_Token_Context_GetVerifyKeyId(ctx);
  ki=GWEN_Crypt_Token_GetKeyInfo(ct,
                                 keyId,
				 0xffffffff,
				 gid);
  if (ki==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Keyinfo %04x not found on crypt token [%s:%s]",
	     keyId,
	     GWEN_Crypt_Token_GetTypeName(ct),
	     GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

  /* let's go */
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
        return GWEN_ERROR_BAD_DATA;
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
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"Found Signature heads but no other segments");
      GWEN_List_free(sigheads);
      return GWEN_ERROR_BAD_DATA;
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
    return GWEN_ERROR_BAD_DATA;
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
        return GWEN_ERROR_BAD_DATA;
      }
      dataLength=currpos-dataBegin;
      break;
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No signature tail found");
    GWEN_List_free(sigheads);
    return GWEN_ERROR_BAD_DATA;
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
    return GWEN_ERROR_BAD_DATA;
  }

  if (strcasecmp(GWEN_DB_GroupName(n), "MsgTail")!=0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unexpected segment (msg tail expected)");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return GWEN_ERROR_BAD_DATA;
  }

  n=GWEN_DB_GetNextGroup(n);
  if (n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unexpected segment (end expected)");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return GWEN_ERROR_BAD_DATA;
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
    return GWEN_ERROR_BAD_DATA;
  }

  /* ok, now verify all signatures */
  dataStart=GWEN_Buffer_GetStart(hmsg->buffer)+dataBegin;
  for (i=0; i< GWEN_List_GetSize(sigtails); i++) {
    GWEN_DB_NODE *sighead;
    GWEN_DB_NODE *sigtail;
    const uint8_t *p;
    uint32_t l;
    int rv;
    uint8_t hash[20];
    const char *signerId;

    /* get signature tail */
    sigtail=(GWEN_DB_NODE*)GWEN_List_GetBack(sigtails);

    /* get corresponding signature head */
    sighead=(GWEN_DB_NODE*)GWEN_List_GetFront(sigheads);

    if (!sighead || !sigtail) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"No signature head/tail left (internal error)");
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return GWEN_ERROR_INTERNAL;
    }

    GWEN_List_PopBack(sigtails);
    GWEN_List_PopFront(sigheads);

    signerId=GWEN_DB_GetCharValue(sighead, "key/userid", 0,
				  I18N("unknown"));

    /* some checks */
    if (strcasecmp(GWEN_DB_GetCharValue(sighead, "ctrlref", 0, ""),
                   GWEN_DB_GetCharValue(sigtail, "ctrlref", 0, ""))!=0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Non-matching signature tail");
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return GWEN_ERROR_BAD_DATA;
    }

    /* hash signature head and data */
    if (1) {
      GWEN_MDIGEST *md;

      /* hash sighead + data */
      p=(const uint8_t*)GWEN_Buffer_GetStart(hmsg->buffer);
      p+=GWEN_DB_GetIntValue(sighead,
			     "segment/pos",
			     0,
			     0);
      l=GWEN_DB_GetIntValue(sighead,
			    "segment/length",
			    0,
			    0);

      md=GWEN_MDigest_Rmd160_new();
      rv=GWEN_MDigest_Begin(md);
      if (rv==0)
        /* digest signature head */
	rv=GWEN_MDigest_Update(md, p, l);
      if (rv==0)
	/* digest data */
	rv=GWEN_MDigest_Update(md, (const uint8_t*)dataStart, dataLength);
      if (rv==0)
	rv=GWEN_MDigest_End(md);
      if (rv<0) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Hash error (%d)", rv);
	GWEN_MDigest_free(md);
	GWEN_List_free(sigheads);
	GWEN_List_free(sigtails);
	return rv;
      }
      memmove(hash,
	      GWEN_MDigest_GetDigestPtr(md),
	      GWEN_MDigest_GetDigestSize(md));
      GWEN_MDigest_free(md);
    }

    /* verify signature */
    p=GWEN_DB_GetBinValue(sigtail, "signature", 0, 0, 0, &l);
    if (p && l) {
      GWEN_CRYPT_PADDALGO *algo;

      algo=GWEN_Crypt_PaddAlgo_new(GWEN_Crypt_PaddAlgoId_None);
      rv=GWEN_Crypt_Token_Verify(ct, keyId,
				 algo,
				 hash, 20, p, l, 0, gid);
      GWEN_Crypt_PaddAlgo_free(algo);
      if (rv) {
	if (rv==GWEN_ERROR_VERIFY) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Invalid signature of user \"%s\"", signerId);
	}
	else {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Could not verify data with medium of user \"%s\" (%d)",
		    AB_User_GetUserId(u), rv);
	}
	GWEN_List_free(sigheads);
	GWEN_List_free(sigtails);
	return rv;
      }
      DBG_INFO(AQHBCI_LOGDOMAIN, "Message signed by \"%s\"", signerId);
      AH_Msg_AddSignerId(hmsg, signerId);
    }
    else {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "No signature");
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return GWEN_ERROR_BAD_DATA;
    }

    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Verification done");
  } /* for */

  GWEN_List_free(sigheads);
  GWEN_List_free(sigtails);
  return 0;
}
















