/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/*
 * This file is included by job.c
 */



int AH_Job__GetJobGroup(GWEN_DB_NODE *dbJob, const char *groupName, GWEN_DB_NODE **pResult)
{
  GWEN_DB_NODE *dbRd;

  dbRd=GWEN_DB_GetGroup(dbJob, GWEN_PATH_FLAGS_NAMEMUSTEXIST, groupName);
  if (dbRd==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Group \"%s\" not found in response", groupName);
    return GWEN_ERROR_NOT_FOUND;
  }

  dbRd=GWEN_DB_GetGroup(dbRd, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
  if (dbRd==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing \"data\" group inside group \"%s\"", groupName);
    return GWEN_ERROR_INVALID;
  }

  dbRd=GWEN_DB_GetGroup(dbRd, GWEN_PATH_FLAGS_NAMEMUSTEXIST, groupName);
  if (dbRd==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing effective group \"%s\" inside response", groupName);
    return GWEN_ERROR_INVALID;
  }

  *pResult=dbRd;
  return 0;
}



int AH_Job__Commit_Bpd(AH_JOB *j)
{
  GWEN_DB_NODE *dbJob    ;
  GWEN_DB_NODE *dbRd=NULL;
  AH_BPD *bpd;
  GWEN_DB_NODE *n;
  const char *p;
  int i;
  int rv;
  GWEN_MSGENGINE *msgEngine;

  //dbJob=GWEN_DB_GetFirstGroup(j->jobResponses);
  dbJob=j->jobResponses;

  rv=AH_Job__GetJobGroup(dbJob, "bpd", &dbRd);
  if (rv<0) {
    if (rv!=GWEN_ERROR_NOT_FOUND) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    DBG_INFO(AQHBCI_LOGDOMAIN, "No BPD in response for job %s", AH_Job_GetName(j));
    /*GWEN_DB_Dump(j->jobResponses, 2);*/
    return 0;
  }

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found BPD, replacing existing");

  msgEngine=AH_User_GetMsgEngine(j->user);
  assert(msgEngine);

  /* create new BPD */
  bpd=AH_Bpd_new();

  /* read version */
  AH_Bpd_SetBpdVersion(bpd, GWEN_DB_GetIntValue(dbRd, "version", 0, 0));

  /* read bank name */
  p=GWEN_DB_GetCharValue(dbRd, "name", 0, 0);
  if (p)
    AH_Bpd_SetBankName(bpd, p);

  /* read message and job limits */
  AH_Bpd_SetJobTypesPerMsg(bpd, GWEN_DB_GetIntValue(dbRd, "jobtypespermsg", 0, 0));
  AH_Bpd_SetMaxMsgSize(bpd, GWEN_DB_GetIntValue(dbRd, "maxmsgsize", 0, 0));

  /* read languages */
  n=GWEN_DB_GetGroup(dbRd, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "languages");
  if (n) {
    for (i=0;; i++) {
      int k;

      k=GWEN_DB_GetIntValue(n, "language", i, 0);
      if (k) {
        if (AH_Bpd_AddLanguage(bpd, k)) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Too many languages (%d)", i);
          break;
        }
      }
      else
        break;
    } /* for */
  } /* if languages */

  /* read supported version */
  n=GWEN_DB_GetGroup(dbRd, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "versions");
  if (n) {
    for (i=0;; i++) {
      int k;

      k=GWEN_DB_GetIntValue(n, "version", i, 0);
      if (k) {
        if (AH_Bpd_AddHbciVersion(bpd, k)) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Too many versions (%d)", i);
          break;
        }
      }
      else
        break;
    } /* for */
  } /* if versions */


  /* communication parameters */
  rv=AH_Job__GetJobGroup(dbJob, "ComData", &dbRd);
  if (rv==0) {
    GWEN_DB_NODE *currService;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Found communication infos");

    currService=GWEN_DB_FindFirstGroup(dbRd, "service");
    while (currService) {
      AH_BPD_ADDR *ba;

      ba=AH_BpdAddr_FromDb(currService);
      if (ba) {
        if (1) { /* dump info */
          GWEN_BUFFER *tbuf;
          const char *s;

          tbuf=GWEN_Buffer_new(0, 256, 0, 1);

          switch (AH_BpdAddr_GetType(ba)) {
          case AH_BPD_AddrTypeTCP:
            GWEN_Buffer_AppendString(tbuf, "TCP: ");
            break;
          case AH_BPD_AddrTypeBTX:
            GWEN_Buffer_AppendString(tbuf, "BTX: ");
            break;
          case AH_BPD_AddrTypeSSL:
            GWEN_Buffer_AppendString(tbuf, "SSL: ");
            break;
          default:
            GWEN_Buffer_AppendString(tbuf, "<UNK>: ");
            break;
          }

          s=AH_BpdAddr_GetAddr(ba);
          if (s && *s)
            GWEN_Buffer_AppendString(tbuf, s);
          else
            GWEN_Buffer_AppendString(tbuf, "<empty>");

          s=AH_BpdAddr_GetSuffix(ba);
          if (s && *s) {
            GWEN_Buffer_AppendString(tbuf, ", ");
            GWEN_Buffer_AppendString(tbuf, s);
          }

          GWEN_Buffer_AppendString(tbuf, ", ");
          switch (AH_BpdAddr_GetFType(ba)) {
          case AH_BPD_FilterTypeNone:
            GWEN_Buffer_AppendString(tbuf, "none");
            break;
          case AH_BPD_FilterTypeBase64:
            GWEN_Buffer_AppendString(tbuf, "base64");
            break;
          case AH_BPD_FilterTypeUUE:
            GWEN_Buffer_AppendString(tbuf, "uue");
            break;
          default:
            GWEN_Buffer_AppendString(tbuf, "<unk>");
            break;
          }

          DBG_INFO(AQHBCI_LOGDOMAIN, "Server address found: %s", GWEN_Buffer_GetStart(tbuf));
          GWEN_Gui_ProgressLog2(0,
                                GWEN_LoggerLevel_Info,
                                I18N("Server address found: %s"),
                                GWEN_Buffer_GetStart(tbuf));
          GWEN_Buffer_free(tbuf);
        }

        /* add service */
        AH_Bpd_AddAddr(bpd, ba);
      }
      currService=GWEN_DB_FindNextGroup(currService, "service");
    }
  } /* if ComData found */


  /* special extension of BPD for PIN/TAN mode */
  rv=AH_Job__GetJobGroup(dbJob, "PinTanBPD", &dbRd);
  if (rv==0) {
    GWEN_DB_NODE *bn;
    GWEN_DB_NODE *currJob;

    bn=AH_Bpd_GetBpdJobs(bpd, GWEN_MsgEngine_GetProtocolVersion(msgEngine));
    assert(bn);

    currJob=GWEN_DB_FindFirstGroup(dbRd, "job");
    while (currJob) {
      const char *jobName;
      int needTAN;
      GWEN_DB_NODE *dbJob;

      jobName=GWEN_DB_GetCharValue(currJob, "job", 0, 0);
      assert(jobName);
      dbJob=GWEN_DB_GetGroup(bn, GWEN_DB_FLAGS_DEFAULT, jobName);
      assert(dbJob);
      needTAN=strcasecmp(GWEN_DB_GetCharValue(currJob, "needTan", 0, "N"), "J")==0;
      GWEN_DB_SetIntValue(dbJob, GWEN_DB_FLAGS_OVERWRITE_VARS, "needTan", needTAN);
      currJob=GWEN_DB_FindNextGroup(currJob, "job");
    } /* while */
  } /* if PIN/TAN extension found */


  /* check for BPD jobs */
  n=GWEN_DB_GetFirstGroup(dbJob);
  while (n) {
    dbRd=GWEN_DB_GetGroup(n, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
    if (dbRd)
      dbRd=GWEN_DB_GetFirstGroup(dbRd);
    if (dbRd) {
      GWEN_XMLNODE *bpdn;
      int segver;
      /* check for BPD job */

      DBG_INFO(AQHBCI_LOGDOMAIN, "Checking whether \"%s\" is a BPD job", GWEN_DB_GroupName(dbRd));
      segver=GWEN_DB_GetIntValue(dbRd, "head/version", 0, 0);
      /* get segment description (first try id, then code) */
      bpdn=GWEN_MsgEngine_FindNodeByProperty(msgEngine, "SEG", "id", segver, GWEN_DB_GroupName(dbRd));
      if (!bpdn)
        bpdn=GWEN_MsgEngine_FindNodeByProperty(msgEngine, "SEG", "code", segver, GWEN_DB_GroupName(dbRd));
      if (bpdn) {
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found a candidate");
        if (atoi(GWEN_XMLNode_GetProperty(bpdn, "isbpdjob", "0"))) {
          /* segment contains a BPD job, move contents */
          GWEN_DB_NODE *bn;
          char numbuffer[32];

          DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found BPD job \"%s\"", GWEN_DB_GroupName(dbRd));
          bn=AH_Bpd_GetBpdJobs(bpd, GWEN_MsgEngine_GetProtocolVersion(msgEngine));
          assert(bn);
          bn=GWEN_DB_GetGroup(bn, GWEN_DB_FLAGS_DEFAULT,
                              GWEN_DB_GroupName(dbRd));
          assert(bn);

          if (GWEN_Text_NumToString(segver, numbuffer, sizeof(numbuffer)-1, 0)<1) {
            DBG_NOTICE(AQHBCI_LOGDOMAIN, "Buffer too small");
            abort();
          }
          bn=GWEN_DB_GetGroup(bn, GWEN_DB_FLAGS_OVERWRITE_GROUPS, numbuffer);
          assert(bn);

          GWEN_DB_AddGroupChildren(bn, dbRd);
          /* remove "head" and "segment" group */
          GWEN_DB_DeleteGroup(bn, "head");
          GWEN_DB_DeleteGroup(bn, "segment");
          DBG_INFO(AQHBCI_LOGDOMAIN, "Added BPD Job %s:%d", GWEN_DB_GroupName(dbRd), segver);
        } /* if isbpdjob */
        else {
          DBG_INFO(AQHBCI_LOGDOMAIN,
                   "Segment \"%s\" is known but not as a BPD job",
                   GWEN_DB_GroupName(dbRd));
        }
      } /* if segment found */
      else {
        DBG_WARN(AQHBCI_LOGDOMAIN, "Did not find segment \"%s\" (%d) ignoring",
                 GWEN_DB_GroupName(dbRd), segver);
      }
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */


  /* set BPD */
  AH_User_SetBpd(j->user, bpd);
  return 0;
}

int AH_Job__VerifiyInitialKey(AH_HBCI *h, GWEN_CRYPT_KEY *key, const char *keyName, const char *tokenType,
                               const char *tokenName, uint32_t tokenCtxId, AH_CRYPT_MODE cryptMode, int rdhType)
{
  GWEN_CRYPT_TOKEN *ct = NULL;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx = NULL;
  int rv = 0;

  rv = AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h), tokenType, tokenName, &ct);
  if(rv != 0)
  {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not get crypt token (%d).", rv);
    return rv;
  }
  if(!GWEN_Crypt_Token_IsOpen(ct))
  {
    GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN);
    rv = GWEN_Crypt_Token_Open(ct, 0, 0);
    if(rv != 0)
    {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not open crypt token (%d).", rv);
      return rv;
    }
  }
  if((ctx = GWEN_Crypt_Token_GetContext(ct, tokenCtxId, 0)) == NULL)
  {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Context %d not found on crypt token [%s:%s]", tokenCtxId, tokenType, tokenName);
    return GWEN_ERROR_NOT_FOUND;
  }
  return AH_User_VerifyInitialKey(h, key, keyName, cryptMode, rdhType, ctx);
}

int AH_Job__CommitSystemData(AH_JOB *j, int doLock)
{
  GWEN_DB_NODE *dbCurr, *dbKeyRspV = NULL, *dbKeyRspS = NULL, *dbKeyRspO = NULL;
  AB_USER *u;
  AB_BANKING *ab;
  AH_HBCI *h;
  GWEN_MSGENGINE *e;
  uint8_t rspSigned = 0;
  int rv = 0;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Committing data");
  assert(j);
  assert(j->usage);

  u=j->user;
  assert(u);
  h=AH_Job_GetHbci(j);
  assert(h);
  ab=AH_Job_GetBankingApi(j);
  assert(ab);
  e=AH_User_GetMsgEngine(j->user);
  assert(e);

  /* GWEN_DB_Dump(j->jobResponses, 2); */

  dbCurr=GWEN_DB_GetFirstGroup(j->jobResponses);
  while (dbCurr) {
    GWEN_DB_NODE *dbRd;

    dbRd=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
    if (dbRd) {
      dbRd=GWEN_DB_GetFirstGroup(dbRd);
    }
    if (dbRd) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Checking group \"%s\"", GWEN_DB_GroupName(dbRd));
      if (strcasecmp(GWEN_DB_GroupName(dbRd), "SegResult")==0) {
        GWEN_DB_NODE *dbRes;

        dbRes=GWEN_DB_GetFirstGroup(dbRd);
        while (dbRes) {
          if (strcasecmp(GWEN_DB_GroupName(dbRes), "result")==0) {
            int code;
            const char *text;

            code=GWEN_DB_GetIntValue(dbRes, "resultcode", 0, 0);
            text=GWEN_DB_GetCharValue(dbRes, "text", 0, 0);
            DBG_NOTICE(AQHBCI_LOGDOMAIN, "Segment result: %d (%s)", code, text?text:"<none>");
            if (code==3920) {
              int i;

              AH_User_ClearTanMethodList(u);
              for (i=0; ; i++) {
                int j;

                j=GWEN_DB_GetIntValue(dbRes, "param", i, 0);
                if (j==0)
                  break;
                DBG_NOTICE(AQHBCI_LOGDOMAIN, "Adding allowed TAN method %d", j);
                AH_User_AddTanMethod(u, j);
              } /* for */
              if (i==0) {
                /* add single step if empty list */
                DBG_INFO(AQHBCI_LOGDOMAIN, "No allowed TAN method reported, assuming 999");
                AH_User_AddTanMethod(u, 999);
              }
            }
          } /* if result */
          dbRes=GWEN_DB_GetNextGroup(dbRes);
        } /* while */
      }
      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "GetKeyResponse")==0)
      {
        const char *keytype = GWEN_DB_GetCharValue(dbRd, "keyname/keytype",  0, NULL);
        if(keytype && *keytype)
        {
#if 1
          if(strcasecmp(keytype, "V") == 0)
#else
          // test 'unknown' key
          if(strcasecmp(keytype, "X") == 0)
#endif
             dbKeyRspV = dbRd;
          else if(strcasecmp(keytype, "S") == 0)
             dbKeyRspS = dbRd;
          else
            dbKeyRspO = dbRd;
        }
      }
      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "SigHead")==0)
      {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): Found sigHead", __FUNCTION__);
        // assume if sigHead found, msg is signed *and* verified
        rspSigned = 1;
      }

      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "SecurityMethods")==0) {
        GWEN_DB_NODE *dbT;

        dbT=GWEN_DB_FindFirstGroup(dbRd, "SecProfile");
        while (dbT) {
          const char *code;
          int version;

          code=GWEN_DB_GetCharValue(dbT, "code", 0, NULL);
          version=GWEN_DB_GetIntValue(dbT, "version", 0, -1);
          if (code && (version>0)) {
            DBG_ERROR(AQHBCI_LOGDOMAIN,
                      "Bank supports mode %s %d",
                      code, version);
            /* TODO: store possible modes */
          }
          dbT=GWEN_DB_FindNextGroup(dbT, "SecProfile");
        } /* while */
      }

      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "UserData")==0) {
        /* UserData found */
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found UserData");
        AH_User_SetUpdVersion(j->user, GWEN_DB_GetIntValue(dbRd, "version", 0, 0));
      }

      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "BankMsg")==0) {
        const char *subject;
        const char *text;

        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found a bank message");
        GWEN_Gui_ProgressLog(0,
                             GWEN_LoggerLevel_Notice,
                             I18N("Bank message received"));
        subject=GWEN_DB_GetCharValue(dbRd, "subject", 0, "(Kein Betreff)");
        text=GWEN_DB_GetCharValue(dbRd, "text", 0, 0);
        if (subject && text) {
          AB_MESSAGE *amsg;
          GWEN_TIME *ti;

          ti=GWEN_CurrentTime();
          amsg=AB_Message_new();
          AB_Message_SetSource(amsg, AB_Message_SourceBank);
          AB_Message_SetSubject(amsg, subject);
          AB_Message_SetText(amsg, text);
          AB_Message_SetDateReceived(amsg, ti);
          GWEN_Time_free(ti);
          AB_Message_SetUserId(amsg, AB_User_GetUniqueId(u));
          AB_Message_List_Add(amsg, j->messages);

          if (1) {
            GWEN_DB_NODE *dbTmp;

            /* save message, later this will no longer be necessary */
            dbTmp=GWEN_DB_Group_new("bank message");
            GWEN_DB_SetCharValue(dbTmp, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                 "subject", subject);
            GWEN_DB_SetCharValue(dbTmp, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                 "text", text);
            if (AH_HBCI_SaveMessage(h, j->user, dbTmp)) {
              DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not save this message:");
              GWEN_DB_Dump(dbTmp, 2);
            }
            GWEN_DB_Group_free(dbTmp);
          }

        } /* if subject and text given */
      } /* if bank msg */


    } /* if response data found */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */


  if(dbKeyRspV || dbKeyRspS || dbKeyRspO)
  {
    uint8_t nDbKey = 0;
    GWEN_CRYPT_KEY *bpk;
    uint8_t *expp, *modp;
    unsigned int expl, modl;
    int keynum, keyver;
    uint16_t nbits;
    int keySize = 0;
    int8_t verified = 0;
    uint8_t signKeyVerifed = 0;
    const char *peerId = NULL;
    const char *tokenType = AH_User_GetTokenType(u);
    const char *tokenName = AH_User_GetTokenName(u);
    uint32_t tokenCtxId = AH_User_GetTokenContextId(u);
    AH_CRYPT_MODE cryptMode = AH_User_GetCryptMode(u);
    int rdhType = AH_User_GetRdhType(u);
    GWEN_DB_NODE *jargs = AH_Job_GetArguments(j);
    GWEN_DB_NODE *keyChange = GWEN_DB_GetGroup(jargs, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "keyChange");

    if(keyChange)
    {
      DBG_INFO(AQHBCI_LOGDOMAIN, "%s(): key-change in progress.", __FUNCTION__);
      tokenType = GWEN_DB_GetCharValue(keyChange, "tokenType", 0, "");
      tokenName = GWEN_DB_GetCharValue(keyChange, "tokenName", 0, "");
      tokenCtxId = GWEN_DB_GetIntValue(keyChange, "tokenCtxId", 0, 0);
      cryptMode = GWEN_DB_GetIntValue(keyChange, "cryptMode", 0, 0);
      rdhType = GWEN_DB_GetIntValue(keyChange, "rdhType", 0, 0);
    }

    for(nDbKey = 0; (rv == 0) && (nDbKey < 3); nDbKey++)
    {
      const char *keyName = (nDbKey == 0) ? "sign" : (nDbKey == 1) ? "crypt" : "unknown";
      GWEN_DB_NODE *dbk = (nDbKey == 0) ? dbKeyRspS : (nDbKey == 1) ? dbKeyRspV : dbKeyRspO;
      if(!dbk)
        continue;
      keynum = GWEN_DB_GetIntValue(dbk, "keyname/keynum",  0, -1);
      keyver = GWEN_DB_GetIntValue(dbk, "keyname/keyversion",  0, -1);
      modp = (uint8_t*)GWEN_DB_GetBinValue(dbk, "key/modulus",  0, NULL, 0, &modl);
      DBG_INFO(AQHBCI_LOGDOMAIN, "Got Key with modulus length %d.", modl);
      /* skip zero bytes if any */
      while(modl && (*modp==0))
      {
        modp++;
        modl--;
      }
      /* calc real length in bits for information purposes */
      nbits = modl * 8;
      if(modl)
      {
        uint8_t b = *modp;
        int j;
        uint8_t mask = 0x80;
        for(j = 0; j < 8; j++)
        {
          if(b & mask)
            break;
          nbits--;
          mask >>= 1;
        }
      }

      /* calculate key size in bytes */
      if(modl <= 96) /* could only be for RDH1, in this case we have to pad to 768 bits */
        keySize=96;
      else
        keySize = modl;

      DBG_INFO(AQHBCI_LOGDOMAIN, "Key has real modulus length %d bytes (%d bits) after skipping leading zero bits.", modl, nbits);
      expp = (uint8_t*)GWEN_DB_GetBinValue(dbk, "key/exponent", 0, NULL, 0, &expl);
      bpk = GWEN_Crypt_KeyRsa_fromModExp(keySize, modp, modl, expp, expl);
      GWEN_Crypt_Key_SetKeyNumber(bpk, keynum);
      GWEN_Crypt_Key_SetKeyVersion(bpk, keyver);

      if(peerId == NULL)
        peerId = GWEN_DB_GetCharValue(dbk, "keyname/userId", 0, NULL);

      /* check if it was already verified and saved at the signature verification stage
       * (this is implemented for RDH7 and RDH9 only at the moment) */
      // if keyChange is in progress, users actual bankkey can not used to test if verified
      // (and anyway test will fail)
      if(rspSigned)
      {
        char msg[512];
        snprintf(msg, sizeof(msg), "Received server %s key in signed message: num=%d, version=%d.", keyName, keynum, keyver);
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): %s", __FUNCTION__, msg);
        GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Warning, I18N("%s"), msg);
        verified = 1;
      }
      else if(!keyChange)
      {
        GWEN_CRYPT_KEY *bpkToVerify = (nDbKey == 0) ? AH_User_GetBankPubSignKey(u) : (nDbKey == 1) ? AH_User_GetBankPubCryptKey(u) : NULL;
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): #%d bkv %p.", __FUNCTION__, nDbKey, bpkToVerify);
        if(bpkToVerify)
        {
          uint32_t l = GWEN_Crypt_Key_GetKeySize(bpk);
          uint32_t lv = GWEN_Crypt_Key_GetKeySize(bpkToVerify);
          uint32_t flags = GWEN_Crypt_KeyRsa_GetFlags(bpkToVerify);
          DBG_NOTICE(AQHBCI_LOGDOMAIN, "bk l %ld %ld, f 0x%08lX.", (long)l, (long)lv, (long)flags);
          if(l == lv)
          {
            uint8_t *m = malloc(l);
            uint8_t *mv = malloc(l);
            if(!GWEN_Crypt_KeyRsa_GetModulus(bpk, m, &l) && !GWEN_Crypt_KeyRsa_GetModulus(bpkToVerify, mv, &l))
            {
              int cmp = memcmp(m, mv, l);
              if((cmp == 0) && (flags & GWEN_CRYPT_KEYRSA_FLAGS_ISVERIFIED))
              {
                DBG_NOTICE(AQHBCI_LOGDOMAIN, "---> keys matched and flag says key is verified. <---");
                verified = 1;
              }
            }
            else
              DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): GWEN_Crypt_KeyRsa_GetModulus() failed.", __FUNCTION__);
            free(m);
            free(mv);
          }
        }
      }

      if(nDbKey < 2)
      {
        if(verified == 0)
        {
          if((nDbKey != 0) && dbKeyRspS)
          {
            if(signKeyVerifed)
              verified = 1;
            else verified = -1;
          }
          if(verified == 0)
            verified = AH_Job__VerifiyInitialKey(h, bpk, keyName, tokenType, tokenName, tokenCtxId, cryptMode, rdhType);
        }
        if(verified == 1)
        {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Imported %s key.", keyName);
          GWEN_Crypt_KeyRsa_AddFlags(bpk, GWEN_CRYPT_KEYRSA_FLAGS_ISVERIFIED);
          if(nDbKey == 0)
            AH_User_SetBankPubSignKey(u, bpk);
          else
            AH_User_SetBankPubCryptKey(u, bpk);
          if(nDbKey == 0)
            signKeyVerifed = verified;
        }
        else
        {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "%s key not imported.", keyName);
          rv = GWEN_ERROR_USER_ABORTED;
        }
      }
      else
      {
        // unknown key
        struct AH_KEYHASH *kh = AH_Provider_KeyHash_new();
        const char *kt = GWEN_DB_GetCharValue(dbk, "keyname/keytype",  0, "?");
        rv = AH_Provider_GetKeyHash(AH_HBCI_GetProvider(h), NULL, bpk, (kt && *kt) ? kt[0] : '?',
                                    cryptMode, rdhType, tokenType, tokenName, tokenCtxId, NULL, 1, kh);
        if(rv != 0)
        {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): AH_Provider_GetKeyHash() failed.", __FUNCTION__);
        }
        else
        {
          uint8_t j = 0;
          char msg[512];
          const char *hn = NULL;
          uint8_t *hstr = NULL;
          uint32_t hl = 0;
          const uint8_t *h = AH_Provider_KeyHash_Hash(kh, &hl);
          AH_Provider_KeyHash_Info(kh, NULL, NULL, &hn);
          hstr = malloc((hl * 3) + 1);
          hstr[hl * 3] = 0;
          for(j = 0; j < hl; j++)
            sprintf((char*)hstr + (j * 3), "%02X ", h[j]);
          snprintf(msg, sizeof(msg), "Received unknown server key: type=%s, num=%d, version=%d, hash (%s)=%s", kt, keynum,
                    keyver, hn, hstr);
          DBG_ERROR(AQHBCI_LOGDOMAIN, "%s", msg);
          GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Warning, I18N("%s"), msg);
          free(hstr);
        }
        AH_Provider_KeyHash_free(kh);
      }
    }
    if(rv != 0)
      return rv;
    if(peerId)
    {
      const char *upid = AH_User_GetPeerId(u);
      if(!upid || strcmp(peerId, upid))
      {
        char msg[256];
        if(upid == NULL)
          snprintf(msg, sizeof(msg), I18N("Setting peer ID to '%s'"), peerId);
        else
          snprintf(msg, sizeof(msg), I18N("Changing peer ID from '%s' to '%s'"), upid, peerId);
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s", msg);
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice, msg);
        AH_User_SetPeerId(u, peerId);
      }
    }
  }

  /* try to extract bank parameter data */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Committing BPD");
  rv=AH_Job__Commit_Bpd(j);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* try to extract accounts */
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_IGNOREACCOUNTS) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Ignoring possibly received accounts");
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Committing accounts");
    rv=AH_Job__Commit_Accounts(j);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }



  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Finished.");
  return 0;
}



int AH_Job_CommitSystemData(AH_JOB *j, int doLock)
{
  AB_USER *u;
  AB_BANKING *ab;
  AB_PROVIDER *pro;
  int rv, rv2;

  u=j->user;
  assert(u);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);
  pro=AH_Job_GetProvider(j);
  assert(pro);

  /* lock user */
  if (doLock) {
    rv=AB_Provider_BeginExclUseUser(pro, u);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* commit data */
  rv2=AH_Job__CommitSystemData(j, doLock);

  if (doLock) {
    /* unlock user */
    rv=AB_Provider_EndExclUseUser(pro, u, 0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AB_Provider_EndExclUseUser(pro, u, 1); /* abandon */
      return rv;
    }
  }

  return rv2;
}



void AH_Job_ReadAccountDataSeg(AB_ACCOUNT *acc, GWEN_DB_NODE *dbAccountData)
{
  int t;

  assert(acc);

  AB_Account_SetBankCode(acc, GWEN_DB_GetCharValue(dbAccountData, "bankCode", 0, 0));
  AB_Account_SetAccountNumber(acc, GWEN_DB_GetCharValue(dbAccountData, "accountId", 0, 0));
  AB_Account_SetIban(acc, GWEN_DB_GetCharValue(dbAccountData, "iban", 0, 0));
  AB_Account_SetAccountName(acc, GWEN_DB_GetCharValue(dbAccountData, "account/name", 0, 0));
  AB_Account_SetSubAccountId(acc, GWEN_DB_GetCharValue(dbAccountData, "accountsubid", 0, 0));
  AB_Account_SetOwnerName(acc, GWEN_DB_GetCharValue(dbAccountData, "name1", 0, 0));

  if (GWEN_DB_GetIntValue(dbAccountData, "head/version", 0, 1)>=4)
    /* KTV in version 2 available */
    AH_Account_AddFlags(acc, AH_BANK_FLAGS_KTV2);
  else
    AH_Account_SubFlags(acc, AH_BANK_FLAGS_KTV2);

  /* account type (from FinTS_3.0_Formals) */
  t=GWEN_DB_GetIntValue(dbAccountData, "type", 0, 1);
  if (t>=1 && t<=9)          /* Kontokorrent-/Girokonto */
    AB_Account_SetAccountType(acc, AB_AccountType_Bank);
  else if (t>=10 && t<=19)   /* Sparkonto */
    AB_Account_SetAccountType(acc, AB_AccountType_Savings);
  else if (t>=20 && t<=29)   /* Festgeldkonto/Termineinlagen */
    AB_Account_SetAccountType(acc, AB_AccountType_MoneyMarket);
  else if (t>=30 && t<=39)   /* Wertpapierdepot */
    AB_Account_SetAccountType(acc, AB_AccountType_Investment);
  else if (t>=40 && t<=49)   /* Kredit-/Darlehenskonto */
    AB_Account_SetAccountType(acc, AB_AccountType_Credit);
  else if (t>=50 && t<=59)   /* Kreditkartenkonto */
    AB_Account_SetAccountType(acc, AB_AccountType_CreditCard);
  else if (t>=60 && t<=69)   /* Fonds-Depot bei einer Kapitalanlagengesellschaft */
    AB_Account_SetAccountType(acc, AB_AccountType_Investment);
  else if (t>=70 && t<=79)   /* Bausparvertrag */
    AB_Account_SetAccountType(acc, AB_AccountType_Savings);
  else if (t>=80 && t<=89)   /* Versicherungsvertrag */
    AB_Account_SetAccountType(acc, AB_AccountType_Savings);
  else if (t>=90 && t<=99)   /* sonstige */
    AB_Account_SetAccountType(acc, AB_AccountType_Unknown);
  else
    AB_Account_SetAccountType(acc, AB_AccountType_Unknown);
}


