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
  const uint8_t *e;
  const uint8_t *m;
  uint32_t kl;
  const uint8_t *h;
  uint32_t hl;
  const char *hn;
};

int IniLetterOutTxt(struct s_hashOut *h);
int IniLetterOutHtml(struct s_hashOut *h);


int AH_Provider_GetIniLetter(AB_PROVIDER *pro, AB_USER *u, uint8_t useBankKey,
                             uint8_t outHtml, GWEN_BUFFER *lbuf, int nounmount)
{
  AH_HBCI *hbci = NULL;
  const char *emsg = NULL;
  struct AH_KEYHASH *kh = NULL;
  int rv = 0;

  assert(pro);
  assert(u);

  hbci = AH_Provider_GetHbci(pro);

  kh = AH_Provider_KeyHash_new();
  if(!kh)
  {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "AH_Provider_KeyHash_new() failed.");
    return GWEN_ERROR_GENERIC;
  }

  if(useBankKey)
  {
    // read bank-keys from config, keys from token may differ.
    // if bank uses ini-letter, that *must* checked against sent key, which is actually in config
    GWEN_CRYPT_KEY *key = AH_User_GetBankPubSignKey(u);
    char kt = 'S';
    if(!key)
    {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "No signkey for bank, using cryptkey.");
      key = AH_User_GetBankPubCryptKey(u);
      kt = 'V';
    }
    if(!key)
    {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Server keys missing, please get them first.");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Server keys missing, please get them first."));
      rv = GWEN_ERROR_NOT_FOUND;
    }
    if(rv == 0)
    {
      rv = AH_Provider_GetKeyHash(AH_HBCI_GetProvider(hbci), u, key, kt, 0, 0, NULL, NULL, 0, NULL, 1, kh);
      if(rv == GWEN_ERROR_NOT_FOUND)
        emsg = "Server keys missing, please get them first.";
    }
  }
  else
  {
    rv = AH_Provider_GetKeyHash(AH_HBCI_GetProvider(hbci), u, NULL, 'S', 0, 0, NULL, NULL, 0, NULL, 0, kh);
    if(rv == GWEN_ERROR_NOT_FOUND)
       emsg = "User keys missing, please generate them first.";
  }
  if(rv == 0)
  {
    struct s_hashOut h;
    h.pro = pro;
    h.isBankKey = useBankKey;
    h.u = u;
    h.lbuf = lbuf;
    AH_Provider_KeyHash_Info(kh, &h.kn, &h.kv, &h.hn);
    h.e = AH_Provider_KeyHash_Exponent(kh, &h.kl);;
    h.m = AH_Provider_KeyHash_Modulus(kh, &h.kl);
    h.h = AH_Provider_KeyHash_Hash(kh, &h.hl);;
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "bank %d, kLen %ld, e %p, m %p, hb %p, out %ld.",
                                                useBankKey, (long)h.kl, h.e, h.m, h.h, (long)h.hl);
    if(!outHtml)
      rv = IniLetterOutTxt(&h);
    else
      rv = IniLetterOutHtml(&h);
  }
  AH_Provider_KeyHash_free(kh);

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

