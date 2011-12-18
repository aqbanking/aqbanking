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



int AH_MsgPinTan_PrepareCryptoSeg(AH_MSG *hmsg,
				  AB_USER *u,
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
  if (!peerId || *peerId==0)
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
		      "key/keynum", 1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		      "key/keyversion", 1);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		       "secProfile/code",
		       "PIN");
  if (crypt)
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
			"secProfile/version", 1);
  else
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
			"secProfile/version",
			(hmsg->itanMethod==999)?1:2);

  return 0;
}




int AH_Msg_SignPinTan(AH_MSG *hmsg,
		      GWEN_BUFFER *rawBuf,
		      const char *signer) {
  AH_HBCI *h;
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *hbuf;
  int rv;
  char ctrlref[15];
  const char *p;
  GWEN_MSGENGINE *e;
  AB_USER *su;
  uint32_t uFlags;
  char pin[64];
  uint32_t tm;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  GWEN_MsgEngine_SetMode(e, "pintan");

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

  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e,
						    "SEG",
						    "id",
						    0,
						    "SigHead");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"SigHead\" not found");
    return GWEN_ERROR_INTERNAL;
  }

  /* for iTAN mode: set selected mode (Sicherheitsfunktion, kodiert) */
  tm=AH_Msg_GetItanMethod(hmsg);
  if (tm==0) {
    tm=AH_Dialog_GetItanMethod(hmsg->dialog);
    if (tm)
      /* this is needed by AH_MsgPinTan_PrepareCryptoSeg */
      AH_Msg_SetItanMethod(hmsg, tm);
  }

  /* prepare config for segment */
  cfg=GWEN_DB_Group_new("sighead");
  rv=AH_MsgPinTan_PrepareCryptoSeg(hmsg, su, cfg, 0, 1);
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
  p=NULL;
  if (!hmsg->noSysId)
    p=AH_User_GetSystemId(su);
  if (!p)
    p="0";
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/SecId", p);

  if (tm) {
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
			"function", tm);
  }

  /* retrieve control reference for sigtail (to be used later) */
  p=GWEN_DB_GetCharValue(cfg, "ctrlref", 0, "");
  if (strlen(p)>=sizeof(ctrlref)) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Control reference too long (14 bytes maximum)");
    GWEN_DB_Group_free(cfg);
    return -1;
  }
  strcpy(ctrlref, p);

  /* create SigHead */
  hbuf=GWEN_Buffer_new(0, 128+GWEN_Buffer_GetUsedBytes(rawBuf), 0, 1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", hmsg->firstSegment-1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		      "signseq", 1);

  /* create signature head segment */
  rv=GWEN_MsgEngine_CreateMessageFromNode(e, node, hbuf, cfg);
  GWEN_DB_Group_free(cfg);
  cfg=0;
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create SigHead");
    GWEN_Buffer_free(hbuf);
    return rv;
  }

  /* insert new SigHead at beginning of message buffer */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Inserting signature head");
  GWEN_Buffer_Rewind(hmsg->buffer);
  GWEN_Buffer_InsertBytes(hmsg->buffer,
			  GWEN_Buffer_GetStart(hbuf),
			  GWEN_Buffer_GetUsedBytes(hbuf));

  /* create sigtail */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Completing signature tail");
  cfg=GWEN_DB_Group_new("sigtail");
  GWEN_Buffer_Reset(hbuf);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", hmsg->lastSegment+1);
  /* store to DB */
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		      "signature",
		      "NOSIGNATURE",
                      11);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		       "ctrlref", ctrlref);

  /* handle pin */
  memset(pin, 0, sizeof(pin));
  rv=AH_User_InputPin(su, pin, 4, sizeof(pin), 0);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "Error getting pin from medium (%d)", rv);
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(hbuf);
    memset(pin, 0, sizeof(pin));
    return rv;
  }
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "pin", pin);
  AH_Msg_SetPin(hmsg, pin);
  memset(pin, 0, sizeof(pin));

  /* handle tan */
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
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Asking for TAN");
      rv=AH_User_InputTan(su, tan, 4, sizeof(tan));
      if (rv<0) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Error getting TAN from medium");
	GWEN_DB_Group_free(cfg);
	GWEN_Buffer_free(hbuf);
	return rv;
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
    return GWEN_ERROR_INTERNAL;
  }

  rv=GWEN_MsgEngine_CreateMessageFromNode(e, node, hbuf, cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create SigTail (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return rv;
  }

  /* append sigtail */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Appending signature tail");
  if (GWEN_Buffer_AppendBuffer(hmsg->buffer, hbuf)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return GWEN_ERROR_MEMORY_FULL;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Appending signature tail: done");

  GWEN_Buffer_free(hbuf);
  GWEN_DB_Group_free(cfg);

  /* adjust segment numbers (for next signature and message tail */
  hmsg->firstSegment--;
  hmsg->lastSegment++;

  return 0;
}



int AH_Msg_EncryptPinTan(AH_MSG *hmsg) {
  AH_HBCI *h;
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *hbuf;
  int rv;
  const char *p;
  GWEN_MSGENGINE *e;
  AB_USER *u;
  const char *peerId;
//  uint32_t uFlags;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  GWEN_MsgEngine_SetMode(e, "pintan");

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
//  uFlags=AH_User_GetFlags(u);

  peerId=AH_User_GetPeerId(u);
  if (!peerId || *peerId==0)
    peerId=AB_User_GetUserId(u);

  /* create crypt head */
  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e,
						    "SEG",
						    "id",
						    0,
						    "CryptHead");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"CryptHead\" not found");
    return GWEN_ERROR_INTERNAL;
  }

  /* create CryptHead */
  cfg=GWEN_DB_Group_new("crypthead");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		      "head/seq", 998);

  rv=AH_MsgPinTan_PrepareCryptoSeg(hmsg, u, cfg, 1, 0);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(cfg);
    return rv;
  }

  /* store system id */
  p=NULL;
  if (!hmsg->noSysId)
    p=AH_User_GetSystemId(u);
  if (!p)
    p="0";
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/SecId", p);

  /* store encrypted message key */
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		      "CryptAlgo/MsgKey",
                      "NOKEY", 5);

  hbuf=GWEN_Buffer_new(0, 256+GWEN_Buffer_GetUsedBytes(hmsg->buffer), 0, 1);
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
					  node,
					  hbuf,
					  cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create CryptHead (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return rv;
  }
  GWEN_DB_Group_free(cfg);

  /* create cryptdata */
  cfg=GWEN_DB_Group_new("cryptdata");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", 999);
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
		      "cryptdata",
		      GWEN_Buffer_GetStart(hmsg->buffer),
                      GWEN_Buffer_GetUsedBytes(hmsg->buffer));

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




int AH_Msg_DecryptPinTan(AH_MSG *hmsg, GWEN_DB_NODE *gr){
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

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  GWEN_MsgEngine_SetMode(e, "pintan");

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
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
                        0,0,
			&l);
  if (!p || !l) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt data");
    return GWEN_ERROR_BAD_DATA;
  }

  /* decipher message with session key */
  mbuf=GWEN_Buffer_new(0, l, 0, 1);
  GWEN_Buffer_AppendBytes(mbuf, (const char*)p, l);

  /* store crypter id */
  AH_Msg_SetCrypterId(hmsg, crypterId);

  /* store new buffer inside message */
  GWEN_Buffer_free(hmsg->origbuffer);
  hmsg->origbuffer=hmsg->buffer;
  GWEN_Buffer_Rewind(mbuf);
  hmsg->buffer=mbuf;

  return 0;
}



int AH_Msg_VerifyPinTan(AH_MSG *hmsg, GWEN_DB_NODE *gr) {
  AH_HBCI *h;
  GWEN_LIST *sigheads;
  GWEN_LIST *sigtails;
  GWEN_DB_NODE *n;
  int nonSigHeads;
  int nSigheads;
  unsigned int dataBegin;
//  char *dataStart;
//  unsigned int dataLength;
  unsigned int i;
  AB_USER *u;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(u);

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
//      dataLength=currpos-dataBegin;
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
//  dataStart=GWEN_Buffer_GetStart(hmsg->buffer)+dataBegin;
  for (i=0; i< GWEN_List_GetSize(sigtails); i++) {
    GWEN_DB_NODE *sighead;
    GWEN_DB_NODE *sigtail;
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

    /* verify signature */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Message signed by \"%s\"", signerId);
    AH_Msg_AddSignerId(hmsg, signerId);

    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Verification done");
  } /* for */

  GWEN_List_free(sigheads);
  GWEN_List_free(sigtails);
  return 0;
}
















