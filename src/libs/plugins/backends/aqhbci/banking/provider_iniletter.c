/***************************************************************************
    begin       : Tue Jun 03 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de
    modified    : Wed Sep 18 2019 by Thomas B.
    email       : th.be@t-online.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/*
 * This file is included by provider.c
 */


const char *fmtStr(char *buff, size_t buffLen, const char *fmt, ...)  __attribute__((format(printf, 3, 4)));
#define FB fmtBuff, sizeof(fmtBuff)

void dbgHexOut(unsigned const char *d, uint32_t l)
{
#if 0
  uint32_t j = 0;
  printf("\n");
  for(uint32_t i = 0; i < l; i++, j++)
  {
    if(j >= 32)
    {
      printf("\n");
      j = 0;
    }
    printf(" %02X", d[i]);
  }
  printf("\n\n");
#endif
}

int AH_Provider_GetIniLetter(AB_PROVIDER *pro, AB_USER *u, uint8_t useBankKey,
                             uint8_t outHtml, GWEN_BUFFER *lbuf, int nounmount);

int AH_Provider_GetIniLetterTxt(AB_PROVIDER *pro, AB_USER *u, int useBankKey, int variant,
                                GWEN_BUFFER *lbuf, int nounmount)
{
  if(variant != 0)
    DBG_WARN(AQHBCI_LOGDOMAIN, "'variant' ignored.");
  return AH_Provider_GetIniLetter(pro, u, useBankKey, 0, lbuf, nounmount);
}


int AH_Provider_GetIniLetterHtml(AB_PROVIDER *pro, AB_USER *u, int useBankKey, int variant,
                                 GWEN_BUFFER *lbuf, int nounmount)
{
  if(variant != 0)
    DBG_WARN(AQHBCI_LOGDOMAIN, "'variant' ignored.");
  return AH_Provider_GetIniLetter(pro, u, useBankKey, 1, lbuf, nounmount);
}

struct s_hashOut
{
  uint8_t isBankKey;
  AB_PROVIDER *pro;
  AB_USER *u;
  GWEN_BUFFER *lbuf;
  int kn;
  int kv;
  uint8_t *e;
  uint8_t *m;
  uint32_t kl;
  uint8_t *h;
  uint32_t hl;
  const char *hn;
};

int IniLetterOutTxt(struct s_hashOut *h);
int IniLetterOutHtml(struct s_hashOut *h);

// XXX hash-len from algo-id?
#define HASHOUT_MAXLEN  32

int AH_Provider_GetIniLetter(AB_PROVIDER *pro, AB_USER *u, uint8_t useBankKey,
                             uint8_t outHtml, GWEN_BUFFER *lbuf, int nounmount)
{
  char fmtBuff[256];
  AH_HBCI *hbci = NULL;
  AH_CRYPT_MODE cryptMode = AH_CryptMode_None;
  int rdhType = 0;
  int kn = 0, kv = 0;
  GWEN_BUFFER *hBuff = NULL;
  uint32_t eLen = 0, mLen = 0, hLen = 0;
  uint8_t *mData = NULL, *eData = NULL;
  GWEN_CRYPT_HASHALGOID hAlgo = GWEN_Crypt_HashAlgoId_Unknown;
  uint8_t hOutBuff[HASHOUT_MAXLEN];
  uint32_t hOutLen = 0;
  const char *hName = NULL;
  const char *emsg = NULL;
  int rv = 0;

  assert(pro);
  assert(u);

  hbci = AH_Provider_GetHbci(pro);

  memset(hOutBuff, 0, HASHOUT_MAXLEN);

  cryptMode = AH_User_GetCryptMode(u);
  rdhType = AH_User_GetRdhType(u);
  switch(cryptMode)
  {
  case AH_CryptMode_Rdh:
    switch(rdhType)
    {
    case 1:
    case 2:
    case 3:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
      break;
    default:
      rv = GWEN_ERROR_INVALID;
    }
    break;
  case AH_CryptMode_Rah:
    switch(rdhType)
    {
    case 7:
    case 9:
    case 10:
      break;
    default:
      rv = GWEN_ERROR_INVALID;
    }
    break;
  default:
    rv = GWEN_ERROR_INVALID;
  }

  if(rv != 0)
    emsg = fmtStr(FB, "Cryptmode %s%d not valid.", AH_CryptMode_toString(cryptMode), rdhType);


  if(rv == 0)
  {
    if(useBankKey)
    {
      // read bank-keys from config, keys from token may differ.
      // if bank uses ini-letter, that *must* checked against sent key, which is actually in config
      GWEN_CRYPT_KEY *key = AH_User_GetBankPubSignKey(u);
      eLen = 3;
      if(!key)
      {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "No signkey for bank, using cryptkey.");
        key = AH_User_GetBankPubCryptKey(u);
      }
      if(!key)
      {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Server keys missing, please get them first.");
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Server keys missing, please get them first."));
        return GWEN_ERROR_NOT_FOUND;
      }
      mLen = GWEN_Crypt_Key_GetKeySize(key);

      if(key && eLen && mLen)
      {
        kn = GWEN_Crypt_Key_GetKeyNumber(key);
        kv = GWEN_Crypt_Key_GetKeyVersion(key);
        mData = malloc(mLen);
        eData = malloc(eLen);
        rv = GWEN_Crypt_KeyRsa_GetModulus(key, mData, &mLen);
        if(rv == 0)
          GWEN_Crypt_KeyRsa_GetExponent(key, eData, &eLen);
        if(rv != 0)
        {
          emsg = "Bad key.";
          rv = GWEN_ERROR_BAD_DATA;
        }
      }
    }
    else
    {
      GWEN_CRYPT_TOKEN *ct = NULL;
      const GWEN_CRYPT_TOKEN_KEYINFO *ki = NULL;
      const GWEN_CRYPT_TOKEN_CONTEXT *ctx = NULL;
      rv = AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(hbci), AH_User_GetTokenType(u), AH_User_GetTokenName(u), &ct);
      if((rv != 0) || !ct)
      {
        emsg = fmtStr(FB, "Could not get crypt token (%d)", rv);
        rv = GWEN_ERROR_GENERIC;
      }
      if(rv == 0)
      {
        rv = GWEN_Crypt_Token_Open(ct, 1, 0);
        if(rv != 0)
        {
          emsg = fmtStr(FB, "Could not open crypt token (%d)", rv);
          rv = GWEN_ERROR_GENERIC;
        }
      }
      if(rv == 0)
      {
        ctx = GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), 0);
        if(!ctx)
        {
          emsg = fmtStr(FB, "User context %d not found on crypt token", AH_User_GetTokenContextId(u));
          rv = GWEN_ERROR_GENERIC;
        }
      }
      if(rv == 0)
      {
        uint32_t kf = 0;
        uint32_t kid = GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
        if(kid)
          ki = GWEN_Crypt_Token_GetKeyInfo(ct, kid, 0, 0);
        if(ki)
          kf = GWEN_Crypt_Token_KeyInfo_GetFlags(ki);
        if(!ki || !(kf & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) | !(kf & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT))
        {
          emsg = "User keys missing, please generate them first.";
          rv = GWEN_ERROR_NOT_FOUND;
        }
      }
      if(rv == 0)
      {
        const uint8_t *e = GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
        const uint8_t *m = GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
        eLen = GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
        mLen = GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
        if(!e || !eLen || !m || !mLen)
        {
          emsg = "Bad key.";
          rv = GWEN_ERROR_BAD_DATA;
        }
        else
        {
          kn = GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki);
          kv = GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki);
          mData = malloc(mLen);
          eData = malloc(eLen);
          memcpy(mData, m, mLen);
          memcpy(eData, e, eLen);
        }
      }
    }
  }

  if(rv == 0)
  {
    switch(cryptMode)
    {
    case AH_CryptMode_Rdh:
      switch(rdhType)
      {
      case 1:
        hLen = 128 * 2;
        hAlgo = GWEN_Crypt_HashAlgoId_Rmd160;
        break;
      case 2:
      case 3:
      case 5:
        hLen = mLen * 2;
        hAlgo = GWEN_Crypt_HashAlgoId_Rmd160;
        break;
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
        // XXX EF_NOTEPAD, HBCI-Version C0=002
        hAlgo = GWEN_Crypt_HashAlgoId_Sha256;
        hLen = mLen * 2;
        break;
      default:;
      }
    case AH_CryptMode_Rah:
      switch(rdhType)
      {
      case 7:
      case 9:
      case 10:
        hAlgo = GWEN_Crypt_HashAlgoId_Sha256;
        hLen = mLen * 2;
        break;
      default:;
      }
    default:;
    }

  }

  if((rv == 0) && ((mLen > (hLen / 2)) || (eLen > (hLen / 2))))
  {
    emsg = fmtStr(FB, "Invalid size for e, m, or h (%ld, %ld, %ld).", (long)eLen, (long)mLen, (long)hLen);
    rv = GWEN_ERROR_GENERIC;
  }

  if(rv == 0)
  {
    if(hLen && eLen && mLen)
      hBuff = GWEN_Buffer_new(0, hLen, 0, 1);
    if(hBuff)
    {
      GWEN_Buffer_FillWithBytes(hBuff, 0, (hLen / 2) - eLen);
      GWEN_Buffer_AppendBytes(hBuff, (const char*)eData, eLen);
      GWEN_Buffer_FillWithBytes(hBuff, 0, (hLen / 2) - mLen);
      GWEN_Buffer_AppendBytes(hBuff, (const char*)mData, mLen);
      GWEN_Buffer_Rewind(hBuff);
      dbgHexOut((unsigned const char*)GWEN_Buffer_GetStart(hBuff), hLen);
     }
  }

  if(rv == 0)
  {
    switch(hAlgo)
    {
    case GWEN_Crypt_HashAlgoId_Rmd160:
      hName = "RMD-160";
      hOutLen = 20;
      rv = AH_Provider__HashRmd160((const uint8_t*)GWEN_Buffer_GetStart(hBuff),
                                 GWEN_Buffer_GetUsedBytes(hBuff), hOutBuff);
      if(rv != 0)
        emsg = fmtStr(FB, "AH_Provider__HashRmd160() failed (%d).", rv);
      break;
    case GWEN_Crypt_HashAlgoId_Sha256:
      hName = "SHA-256";
      hOutLen = 32;
      rv = AH_Provider__HashSha256((const uint8_t*)GWEN_Buffer_GetStart(hBuff),
                                 GWEN_Buffer_GetUsedBytes(hBuff), hOutBuff);
      if(rv != 0)
        emsg = fmtStr(FB, "AH_Provider__HashSha256() failed (%d).", rv);
      break;
    default:
      emsg = "Hash algo not specified.";
      rv = GWEN_ERROR_GENERIC;
    }
    dbgHexOut(hOutBuff, hOutLen);
  }

  if((rv == 0) && (!hLen || !eLen || !eData || !mData || !hBuff || !hOutLen))
  {
    emsg = fmtStr(FB, "Internal error, bank %d, hLen %ld, eLen %ld, mLen %ld, e %p, m %p, hb %p, out %ld.",
                    useBankKey, (long)hLen, (long)eLen, (long)mLen, eData, mData, hBuff, (long)hOutLen);
    rv = GWEN_ERROR_GENERIC;
  }

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "bank %d, hLen %ld, eLen %ld, mLen %ld, e %p, m %p, hb %p, out %ld.",
                    useBankKey, (long)hLen, (long)eLen, (long)mLen, eData, mData, hBuff, (long)hOutLen);

  if(rv == 0)
  {
    struct s_hashOut h;
    uint8_t *eb = malloc(hLen / 2);
    uint8_t *mb = malloc(hLen / 2);
    memcpy(eb, GWEN_Buffer_GetStart(hBuff), hLen / 2);
    memcpy(mb, GWEN_Buffer_GetStart(hBuff) + (hLen / 2), hLen / 2);
    h.pro = pro;
    h.isBankKey = useBankKey;
    h.u = u;
    h.lbuf = lbuf;
    h.kn = kn;
    h.kv = kv;
    h.e = eb;
    h.m = mb;
    h.kl = hLen / 2;
    h.h = hOutBuff;
    h.hl = hOutLen;
    h.hn = hName;
    if(!outHtml)
      rv = IniLetterOutTxt(&h);
    else
      rv = IniLetterOutHtml(&h);
    free(eb);
    free(mb);
  }

  if(hBuff)
    GWEN_Buffer_free(hBuff);
  free(eData);
  free(mData);

  if(rv != 0)
  {
    if(!emsg)
      emsg = "Internal Error";
    DBG_ERROR(AQHBCI_LOGDOMAIN, "%s", emsg);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N(emsg));
    return rv;
  }

  if(!nounmount)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(hbci));
  return 0;
}

int IniLetterOutTxt(struct s_hashOut *h)
{
  char fmtBuff[128];
  int rv = 0;
  GWEN_TIME *ti = GWEN_CurrentTime();
  GWEN_BUFFER *b = h->lbuf;
  AH_HBCI *hbci = AH_Provider_GetHbci(h->pro);
  uint32_t i = 0;

  assert(h);
  assert(ti);

  GWEN_Buffer_AppendString(b, I18N("\n\n\nINI-Letter\n\n"));
  GWEN_Buffer_AppendString(b, I18N("Date            : "));
  GWEN_Time_toString(ti, I18N("YYYY/MM/DD"), b);
  GWEN_Buffer_AppendString(b, "\n");
  GWEN_Buffer_AppendString(b, I18N("Time            : "));
  GWEN_Time_toString(ti, I18N("hh:mm:ss"), b);
  GWEN_Buffer_AppendString(b, "\n");
  if(h->isBankKey)
  {
    GWEN_Buffer_AppendString(b, I18N("Bank Code       : "));
    GWEN_Buffer_AppendString(b, AB_User_GetBankCode(h->u));
  }
  else
  {
    GWEN_Buffer_AppendString(b, I18N("User            : "));
    GWEN_Buffer_AppendString(b, AB_User_GetUserId(h->u));
  }
  GWEN_Buffer_AppendString(b, "\n");
  GWEN_Buffer_AppendString(b, I18N("Key number      : "));
  GWEN_Buffer_AppendString(b, fmtStr(FB, "%d", h->kn));
  GWEN_Buffer_AppendString(b, "\n");
  GWEN_Buffer_AppendString(b, I18N("Key version     : "));
  GWEN_Buffer_AppendString(b, fmtStr(FB, "%d", h->kv));
  GWEN_Buffer_AppendString(b, "\n");
  if(h->isBankKey)
  {
    GWEN_Buffer_AppendString(b, I18N("Customer system: "));
    GWEN_Buffer_AppendString(b, AH_HBCI_GetProductName(hbci));
    GWEN_Buffer_AppendString(b, "\n");
  }
  GWEN_Buffer_AppendString(b, "\n");
  GWEN_Buffer_AppendString(b, I18N("Public key for electronic signature"));
  GWEN_Buffer_AppendString(b, "\n\n");
  GWEN_Buffer_AppendString(b, "  ");
  GWEN_Buffer_AppendString(b, I18N("Exponent"));
  GWEN_Buffer_AppendString(b, "\n\n");
  for(i = 0; i < h->kl; i += 16)
  {
    uint32_t left = h->kl - i;
    if(GWEN_Text_ToHexBuffer((const char*)h->e + i, (left < 16) ? left : 16, b, 2, ' ', 0))
    {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "GWEN_Text_ToHexBuffer() failed.");
      return GWEN_ERROR_GENERIC;
    }
    GWEN_Buffer_AppendString(b, "\n");
  }
  GWEN_Buffer_AppendString(b, "\n  ");
  GWEN_Buffer_AppendString(b, I18N("Modulus"));
  GWEN_Buffer_AppendString(b, "\n\n");
  for(i = 0; i < h->kl; i += 16)
  {
    uint32_t left = h->kl - i;
    if(GWEN_Text_ToHexBuffer((const char*)h->m + i, (left < 16) ? left : 16, b, 2, ' ', 0))
    {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "GWEN_Text_ToHexBuffer() failed.");
      return GWEN_ERROR_GENERIC;
    }
    GWEN_Buffer_AppendString(b, "\n");
  }
  GWEN_Buffer_AppendString(b, "\n  ");
  GWEN_Buffer_AppendString(b, I18N("Hash"));
  GWEN_Buffer_AppendString(b, fmtStr(FB, " (%s)", h->hn));
  GWEN_Buffer_AppendString(b, "\n\n");
  for(i = 0; i < h->hl; i += 16)
  {
    uint32_t left = h->hl - i;
    if(GWEN_Text_ToHexBuffer((const char*)h->h + i, (left < 16) ? left : 16, b, 2, ' ', 0))
    {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "GWEN_Text_ToHexBuffer() failed.");
      return GWEN_ERROR_GENERIC;
    }
    GWEN_Buffer_AppendString(b, "\n");
  }
  if(!h->isBankKey)
  {
    GWEN_Buffer_AppendString(b, "\n\n");
    GWEN_Buffer_AppendString(b, I18N("I confirm that I created the above key "
                                  "for my electronic signature.\n"));
    GWEN_Buffer_AppendString(b, "\n\n");
    GWEN_Buffer_AppendString(b, I18N("____________________________  ____________________________\n"
                                     "Place, date                   Signature\n"));
  }

  return rv;
}

int IniLetterOutHtml(struct s_hashOut *h)
{
  char fmtBuff[128];
  int rv = 0;
  GWEN_TIME *ti = GWEN_CurrentTime();
  GWEN_BUFFER *b = h->lbuf;
  AH_HBCI *hbci = AH_Provider_GetHbci(h->pro);
  uint32_t i = 0;

  assert(h);
  assert(ti);

  GWEN_Buffer_AppendString(b, "<h3>");
  GWEN_Buffer_AppendString(b, I18N("INI-Letter"));
  GWEN_Buffer_AppendString(b, "</h3>\n<table>\n<tr><td>\n");
  GWEN_Buffer_AppendString(b, I18N("Date"));
  GWEN_Buffer_AppendString(b, "</td><td>\n");
  GWEN_Time_toString(ti, I18N("YYYY/MM/DD"), b);
  GWEN_Buffer_AppendString(b, "</td></tr>\n<tr><td>\n");
  GWEN_Buffer_AppendString(b, I18N("Time"));
  GWEN_Buffer_AppendString(b, "</td><td>\n");
  GWEN_Time_toString(ti, I18N("hh:mm:ss"), b);
  GWEN_Buffer_AppendString(b, "</td></tr>\n");
  if(h->isBankKey)
  {
    GWEN_Buffer_AppendString(b, "<tr><td>\n");
    GWEN_Buffer_AppendString(b, I18N("Bank Code"));
    GWEN_Buffer_AppendString(b, "</td><td>\n");
    GWEN_Buffer_AppendString(b, AB_User_GetBankCode(h->u));
    GWEN_Buffer_AppendString(b, "</td></tr>\n");
  }
  else
  {
    GWEN_Buffer_AppendString(b, "<tr><td>\n");
    GWEN_Buffer_AppendString(b, I18N("User"));
    GWEN_Buffer_AppendString(b, "</td><td>\n");
    GWEN_Buffer_AppendString(b, AB_User_GetUserId(h->u));
    GWEN_Buffer_AppendString(b, "</td></tr>\n");
  }
  GWEN_Buffer_AppendString(b, "<tr><td>\n");
  GWEN_Buffer_AppendString(b, I18N("Key number"));
  GWEN_Buffer_AppendString(b, "</td><td>\n");
  GWEN_Buffer_AppendString(b, fmtStr(FB, "%d", h->kn));
  GWEN_Buffer_AppendString(b, "</td></tr>\n");

  GWEN_Buffer_AppendString(b, "<tr><td>\n");
  GWEN_Buffer_AppendString(b, I18N("Key version"));
  GWEN_Buffer_AppendString(b, "</td><td>\n");
  GWEN_Buffer_AppendString(b, fmtStr(FB, "%d", h->kv));
  GWEN_Buffer_AppendString(b, "</td></tr>\n");
  if(h->isBankKey)
  {
    GWEN_Buffer_AppendString(b, "<tr><td>\n");
    GWEN_Buffer_AppendString(b, I18N("Customer system"));
    GWEN_Buffer_AppendString(b, "</td><td>\n");
    GWEN_Buffer_AppendString(b, AH_HBCI_GetProductName(hbci));
    GWEN_Buffer_AppendString(b, "</td></tr>\n");
  }
  GWEN_Buffer_AppendString(b, "</table>\n");

  GWEN_Buffer_AppendString(b, "<h3>");
  GWEN_Buffer_AppendString(b, I18N("Public key for electronic signature"));
  GWEN_Buffer_AppendString(b, "</h3>\n");

  GWEN_Buffer_AppendString(b, "<h4>");
  GWEN_Buffer_AppendString(b, I18N("Exponent"));
  GWEN_Buffer_AppendString(b, "</h4>\n");

  GWEN_Buffer_AppendString(b, "<font face=fixed>\n");
  for(i = 0; i < h->kl; i += 16)
  {
    uint32_t left = h->kl - i;
    if(GWEN_Text_ToHexBuffer((const char*)h->e + i, (left < 16) ? left : 16, b, 2, ' ', 0))
    {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "GWEN_Text_ToHexBuffer() failed.");
      return GWEN_ERROR_GENERIC;
    }
    GWEN_Buffer_AppendString(b, "<br>\n");
  }
  GWEN_Buffer_AppendString(b, "</font>\n<br>\n");

  GWEN_Buffer_AppendString(b, "<h4>");
  GWEN_Buffer_AppendString(b, I18N("Modulus"));
  GWEN_Buffer_AppendString(b, "</h4>\n");

  GWEN_Buffer_AppendString(b, "<font face=fixed>\n");
  for(i = 0; i < h->kl; i += 16)
  {
    uint32_t left = h->kl - i;
    if(GWEN_Text_ToHexBuffer((const char*)h->m + i, (left < 16) ? left : 16, b, 2, ' ', 0))
    {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "GWEN_Text_ToHexBuffer() failed.");
      return GWEN_ERROR_GENERIC;
    }
    GWEN_Buffer_AppendString(b, "<br>\n");
  }
  GWEN_Buffer_AppendString(b, "</font>\n<br>\n");

  GWEN_Buffer_AppendString(b, "<h4>");
  GWEN_Buffer_AppendString(b, I18N("Hash"));
  GWEN_Buffer_AppendString(b, fmtStr(FB, " (%s)", h->hn));
  GWEN_Buffer_AppendString(b, "</h4>\n");

  GWEN_Buffer_AppendString(b, "<font face=fixed>\n");
  for(i = 0; i < h->hl; i += 16)
  {
    uint32_t left = h->hl - i;
    if(GWEN_Text_ToHexBuffer((const char*)h->h + i, (left < 16) ? left : 16, b, 2, ' ', 0))
    {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "GWEN_Text_ToHexBuffer() failed.");
      return GWEN_ERROR_GENERIC;
    }
    GWEN_Buffer_AppendString(b, "<br>\n");
  }
  GWEN_Buffer_AppendString(b, "</font>\n<br>\n");

  if(!h->isBankKey)
  {
    GWEN_Buffer_AppendString(b, "<br><br>\n");
    GWEN_Buffer_AppendString(b, I18N("I confirm that I created the above key for my electronic signature.\n"));
    GWEN_Buffer_AppendString(b, "<br><br>\n");
    GWEN_Buffer_AppendString(b, "<table>\n");
    GWEN_Buffer_AppendString(b, "<tr><td>\n");
    GWEN_Buffer_AppendString(b, "____________________________  ");
    GWEN_Buffer_AppendString(b, "</td><td>\n");
    GWEN_Buffer_AppendString(b, "____________________________  ");
    GWEN_Buffer_AppendString(b, "</td></tr><tr><td>\n");
    GWEN_Buffer_AppendString(b, I18N("Place, date"));
    GWEN_Buffer_AppendString(b, "</td><td>\n");
    GWEN_Buffer_AppendString(b, I18N("Signature"));
    GWEN_Buffer_AppendString(b, "</td></tr></table>\n");
    GWEN_Buffer_AppendString(b, "<br>\n");
  }

  return rv;
}

