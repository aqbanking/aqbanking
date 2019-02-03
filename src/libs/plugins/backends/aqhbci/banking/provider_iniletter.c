/***************************************************************************
    begin       : Tue Jun 03 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/*
 * This file is included by provider.c
 */






int AH_Provider_GetIniLetterTxt(AB_PROVIDER *pro,
                                AB_USER *u,
                                int useBankKey,
                                int variant,
                                GWEN_BUFFER *lbuf,
                                int nounmount)
{
  if (variant==0) {
    switch (AH_User_GetRdhType(u)) {
    case 0:
    case 1:
      variant=1;
      break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      variant=2;
      break;
    case 8:
    case 9:
      variant=2;
      break;
    case 10:
      variant=2;
      break;
    default:
      DBG_ERROR(AQHBCI_LOGDOMAIN, "RDH mode %d not supported", AH_User_GetRdhType(u));
      return GWEN_ERROR_INVALID;
    }
  }

  switch (variant) {
  case 1:
    return AH_Provider_GetIniLetterTxt1(pro, u, useBankKey, lbuf, nounmount);
  case 2:
    return AH_Provider_GetIniLetterTxt2(pro, u, useBankKey, lbuf, nounmount);
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Variant %d not supported", variant);
    return GWEN_ERROR_INVALID;
  }
}


int AH_Provider_GetIniLetterHtml(AB_PROVIDER *pro,
                                 AB_USER *u,
                                 int useBankKey,
                                 int variant,
                                 GWEN_BUFFER *lbuf,
                                 int nounmount)
{
  if (variant==0) {
    switch (AH_User_GetRdhType(u)) {
    case 0:
    case 1:
      variant=1;
      break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      variant=2;
      break;
    case 8:
    case 9:
      variant=2;
      break;
    case 10:
      variant=2;
      break;
    default:
      DBG_ERROR(AQHBCI_LOGDOMAIN, "RDH mode %d not supported", AH_User_GetRdhType(u));
      return GWEN_ERROR_INVALID;
    }
  }

  switch (variant) {
  case 1:
    return AH_Provider_GetIniLetterHtml1(pro, u, useBankKey, lbuf, nounmount);
  case 2:
    return AH_Provider_GetIniLetterHtml2(pro, u, useBankKey, lbuf, nounmount);
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Variant %d not supported", variant);
    return GWEN_ERROR_INVALID;
  }
}



int AH_Provider_GetIniLetterTxt1(AB_PROVIDER *pro,
                                 AB_USER *u,
                                 int useBankKey,
                                 GWEN_BUFFER *lbuf,
                                 int nounmount)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  const void *p;
  unsigned int l;
  GWEN_BUFFER *bbuf;
  GWEN_BUFFER *keybuf;
  int i;
  GWEN_TIME *ti;
  char numbuf[32];
  char hashbuffer[21];
  AH_PROVIDER *hp;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *cctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki=NULL;
  uint32_t kid;
  int rv;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
                              AH_User_GetTokenType(u),
                              AH_User_GetTokenName(u),
                              &ct);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not get crypt token (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error getting crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* open crypt token */
  rv=GWEN_Crypt_Token_Open(ct, 1, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not open crypt token (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error opening crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* get context */
  cctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), 0);
  if (!cctx) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User context not found on crypt token");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("User context not found on crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_NOT_FOUND;
  }

  if (useBankKey) {
    /* get sign key info */
    kid=GWEN_Crypt_Token_Context_GetVerifyKeyId(cctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                     0);
    }
    if (!ki ||
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      kid=GWEN_Crypt_Token_Context_GetEncipherKeyId(cctx);
      if (kid) {
        ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                       0);
      }
    }
    if (!ki ||
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Server keys missing, please get them first");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("Server keys missing, "
                                "please get them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    /* get sign key info */
    kid=GWEN_Crypt_Token_Context_GetSignKeyId(cctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                     0);
    }
    if (!ki ||
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      DBG_ERROR(AQHBCI_LOGDOMAIN, "User keys missing, please generate them first");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("User keys missing, "
                                "please generate them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }


  keybuf=GWEN_Buffer_new(0, 257, 0, 1);

  /* prelude */
  GWEN_Buffer_AppendString(lbuf,
                           I18N("\n\n\nINI-Letter\n\n"));
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Date           : "));
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, I18N("YYYY/MM/DD"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Time           : "));
  GWEN_Time_toString(ti, I18N("hh:mm:ss"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");

  if (useBankKey) {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Bank Code      : "));
    GWEN_Buffer_AppendString(lbuf, AB_User_GetBankCode(u));
  }
  else {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("User           : "));
    GWEN_Buffer_AppendString(lbuf, AB_User_GetUserId(u));
  }

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Key number     : "));
  snprintf(numbuf, sizeof(numbuf), "%d",
           GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki));
  GWEN_Buffer_AppendString(lbuf, numbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");

  GWEN_Buffer_AppendString(lbuf,
                           I18N("Key version    : "));
  snprintf(numbuf, sizeof(numbuf), "%d",
           GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki));
  GWEN_Buffer_AppendString(lbuf, numbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Customer system: "));
    GWEN_Buffer_AppendString(lbuf, AH_HBCI_GetProductName(h));
    GWEN_Buffer_AppendString(lbuf, "\n");
  }

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Public key for electronic signature"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");

  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Exponent"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");

  /* exponent */
  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  l=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (!p || !l) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad key.");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         I18N("Bad key"));
    return GWEN_ERROR_BAD_DATA;
  }

  bbuf=GWEN_Buffer_new(0, 97, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  if (l<96)
    GWEN_Buffer_FillLeftWithBytes(bbuf, 0, 96-l);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<6; i++) {
    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
      abort();
    }
    p+=16;
    GWEN_Buffer_AppendString(lbuf, "\n");
  }

  GWEN_Buffer_FillWithBytes(keybuf, 0, 128-l);
  GWEN_Buffer_AppendBuffer(keybuf, bbuf);
  GWEN_Buffer_free(bbuf);

  /* modulus */
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Modulus"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");
  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  l=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (!p || !l) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad key.");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         I18N("Bad key"));
    return GWEN_ERROR_BAD_DATA;
  }

  bbuf=GWEN_Buffer_new(0, 97, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  if (l<96)
    GWEN_Buffer_FillLeftWithBytes(bbuf, 0, 96-l);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<6; i++) {
    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
      abort();
    }
    p+=16;
    GWEN_Buffer_AppendString(lbuf, "\n");
  }

  GWEN_Buffer_FillWithBytes(keybuf, 0, 128-l);
  GWEN_Buffer_AppendBuffer(keybuf, bbuf);
  GWEN_Buffer_free(bbuf);

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Hash"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");
  rv=AH_Provider__HashRmd160((const uint8_t *)GWEN_Buffer_GetStart(keybuf),
                             GWEN_Buffer_GetUsedBytes(keybuf),
                             (uint8_t *)hashbuffer);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error hashing (%d)", rv);
    abort();
  }
  GWEN_Buffer_free(keybuf);

  GWEN_Buffer_AppendString(lbuf, "  ");
  if (GWEN_Text_ToHexBuffer(hashbuffer, 20, lbuf, 2, ' ', 0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
    abort();
  }
  GWEN_Buffer_AppendString(lbuf, "\n");

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "\n\n");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("I confirm that I created the above key "
                                  "for my electronic signature.\n"));
    GWEN_Buffer_AppendString(lbuf, "\n\n");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("____________________________  "
                                  "____________________________\n"
                                  "Place, date                   "
                                  "Signature\n"));
  }

  return 0;
}



int AH_Provider_GetIniLetterHtml1(AB_PROVIDER *pro,
                                  AB_USER *u,
                                  int useBankKey,
                                  GWEN_BUFFER *lbuf,
                                  int nounmount)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  const void *p;
  unsigned int l;
  GWEN_BUFFER *bbuf;
  GWEN_BUFFER *keybuf;
  int i;
  GWEN_TIME *ti;
  char numbuf[32];
  char hashbuffer[21];
  AH_PROVIDER *hp;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *cctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki=NULL;
  uint32_t kid;
  int rv;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
                              AH_User_GetTokenType(u),
                              AH_User_GetTokenName(u),
                              &ct);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not get crypt token (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error getting crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* open crypt token */
  rv=GWEN_Crypt_Token_Open(ct, 1, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not open crypt token (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error opening crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* get context */
  cctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), 0);
  if (!cctx) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User context not found on crypt token");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("User context not found on crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_NOT_FOUND;
  }

  if (useBankKey) {
    /* get sign key info */
    kid=GWEN_Crypt_Token_Context_GetVerifyKeyId(cctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                     0);
    }
    if (!ki ||
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      kid=GWEN_Crypt_Token_Context_GetEncipherKeyId(cctx);
      if (kid) {
        ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                       0);
      }
    }
    if (!ki ||
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Server keys missing, please get them first");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("Server keys missing, "
                                "please get them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    /* get sign key info */
    kid=GWEN_Crypt_Token_Context_GetSignKeyId(cctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                     0);
    }
    if (!ki ||
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      DBG_ERROR(AQHBCI_LOGDOMAIN, "User keys missing, please generate them first");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("User keys missing, "
                                "please generate them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }

  keybuf=GWEN_Buffer_new(0, 257, 0, 1);

  /* prelude */
  GWEN_Buffer_AppendString(lbuf, "<h3>");
  GWEN_Buffer_AppendString(lbuf, I18N("INI-Letter"));
  GWEN_Buffer_AppendString(lbuf, "</h3>\n");
  GWEN_Buffer_AppendString(lbuf, "<table>\n");
  GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
  GWEN_Buffer_AppendString(lbuf, I18N("Date"));
  GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, I18N("YYYY/MM/DD"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");

  GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
  GWEN_Buffer_AppendString(lbuf, I18N("Time"));
  GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
  GWEN_Time_toString(ti, I18N("hh:mm:ss"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");

  if (useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("Bank Code"));
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, AB_User_GetBankCode(u));
    GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");
  }
  else {
    GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("User"));
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, AB_User_GetUserId(u));
    GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");
  }

  GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
  GWEN_Buffer_AppendString(lbuf, I18N("Key number"));
  GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
  snprintf(numbuf, sizeof(numbuf), "%d",
           GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki));
  GWEN_Buffer_AppendString(lbuf, numbuf);
  GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");

  GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
  GWEN_Buffer_AppendString(lbuf, I18N("Key version"));
  GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
  snprintf(numbuf, sizeof(numbuf), "%d",
           GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki));
  GWEN_Buffer_AppendString(lbuf, numbuf);
  GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("Customer system"));
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, AH_HBCI_GetProductName(h));
    GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");
  }
  GWEN_Buffer_AppendString(lbuf, "</table>\n");

  GWEN_Buffer_AppendString(lbuf, "<h3>");
  GWEN_Buffer_AppendString(lbuf, I18N("Public key for electronic signature"));
  GWEN_Buffer_AppendString(lbuf, "</h3>\n");

  GWEN_Buffer_AppendString(lbuf, "<h4>");
  GWEN_Buffer_AppendString(lbuf, I18N("Exponent"));
  GWEN_Buffer_AppendString(lbuf, "</h4>\n");

  /* exponent */
  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  l=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (!p || !l) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad key.");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         I18N("Bad key"));
    return GWEN_ERROR_BAD_DATA;
  }

  GWEN_Buffer_AppendString(lbuf, "<font face=fixed>\n");
  bbuf=GWEN_Buffer_new(0, 97, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  if (l<96)
    GWEN_Buffer_FillLeftWithBytes(bbuf, 0, 96-l);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<6; i++) {
    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
      abort();
    }
    p+=16;
    GWEN_Buffer_AppendString(lbuf, "<br>\n");
  }

  GWEN_Buffer_FillWithBytes(keybuf, 0, 128-l);
  GWEN_Buffer_AppendBuffer(keybuf, bbuf);
  GWEN_Buffer_free(bbuf);
  GWEN_Buffer_AppendString(lbuf, "</font>\n");
  GWEN_Buffer_AppendString(lbuf, "<br>\n");

  /* modulus */
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "<h4>");
  GWEN_Buffer_AppendString(lbuf, I18N("Modulus"));
  GWEN_Buffer_AppendString(lbuf, "</h4>\n");
  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  l=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (!p || !l) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad key.");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         I18N("Bad key"));
    return GWEN_ERROR_BAD_DATA;
  }

  GWEN_Buffer_AppendString(lbuf, "<font face=fixed>\n");
  bbuf=GWEN_Buffer_new(0, 97, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  if (l<96)
    GWEN_Buffer_FillLeftWithBytes(bbuf, 0, 96-l);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<6; i++) {
    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
      abort();
    }
    p+=16;
    GWEN_Buffer_AppendString(lbuf, "<br>\n");
  }

  GWEN_Buffer_FillWithBytes(keybuf, 0, 128-l);
  GWEN_Buffer_AppendBuffer(keybuf, bbuf);
  GWEN_Buffer_free(bbuf);
  GWEN_Buffer_AppendString(lbuf, "</font>\n");
  GWEN_Buffer_AppendString(lbuf, "<br>\n");

  GWEN_Buffer_AppendString(lbuf, "<h4>");
  GWEN_Buffer_AppendString(lbuf, I18N("Hash"));
  GWEN_Buffer_AppendString(lbuf, "</h4>\n");
  GWEN_Buffer_AppendString(lbuf, "<font face=fixed>\n");
  rv=AH_Provider__HashRmd160((const uint8_t *)GWEN_Buffer_GetStart(keybuf),
                             GWEN_Buffer_GetUsedBytes(keybuf),
                             (uint8_t *)hashbuffer);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error hashing (%d)", rv);
    abort();
  }
  GWEN_Buffer_free(keybuf);

  GWEN_Buffer_AppendString(lbuf, "  ");
  if (GWEN_Text_ToHexBuffer(hashbuffer, 20, lbuf, 2, ' ', 0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
    abort();
  }
  GWEN_Buffer_AppendString(lbuf, "</font>\n");
  GWEN_Buffer_AppendString(lbuf, "<br>\n");

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "<br><br>\n");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("I confirm that I created the above key "
                                  "for my electronic signature.\n"));
    GWEN_Buffer_AppendString(lbuf, "<br><br>\n");
    GWEN_Buffer_AppendString(lbuf, "<table>\n");
    GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, "____________________________  ");
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, "____________________________  ");
    GWEN_Buffer_AppendString(lbuf, "</td></tr><tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("Place, date"));
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("Signature"));
    GWEN_Buffer_AppendString(lbuf, "</td></tr></table>\n");
    GWEN_Buffer_AppendString(lbuf, "<br>\n");
  }

  return 0;
}



int AH_Provider_GetIniLetterTxt2(AB_PROVIDER *pro,
                                 AB_USER *u,
                                 int useBankKey,
                                 GWEN_BUFFER *lbuf,
                                 int nounmount)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  const void *p;
  unsigned int l;
  unsigned int modLen;
  GWEN_BUFFER *bbuf;
  GWEN_BUFFER *keybuf;
  int i;
  GWEN_TIME *ti;
  char numbuf[32];
  char hashbuffer[33];
  AH_PROVIDER *hp;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *cctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki=NULL;
  uint32_t kid;
  int rv;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
                              AH_User_GetTokenType(u),
                              AH_User_GetTokenName(u),
                              &ct);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not get crypt token (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error getting crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* open crypt token */
  rv=GWEN_Crypt_Token_Open(ct, 1, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not open crypt token (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error opening crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* get context */
  cctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), 0);
  if (!cctx) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User context not found on crypt token");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("User context not found on crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_NOT_FOUND;
  }

  if (useBankKey) {
    /* get sign key info */
    kid=GWEN_Crypt_Token_Context_GetVerifyKeyId(cctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                     0);
    }
    if (!ki ||
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      kid=GWEN_Crypt_Token_Context_GetEncipherKeyId(cctx);
      if (kid) {
        ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                       0);
      }
    }
    if (!ki ||
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Server keys missing, please get them first");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("Server keys missing, "
                                "please get them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    /* get sign key info */
    kid=GWEN_Crypt_Token_Context_GetSignKeyId(cctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                     0);
    }
    if (!ki ||
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      DBG_ERROR(AQHBCI_LOGDOMAIN, "User keys missing, please generate them first");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("User keys missing, "
                                "please generate them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }

#if 0
  modLen=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
#else
  /* use the real modulus length */
  modLen=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
#endif

  keybuf=GWEN_Buffer_new(0, (modLen*2)+1, 0, 1);

  /* prelude */
  GWEN_Buffer_AppendString(lbuf,
                           I18N("\n\n\nINI-Letter\n\n"));
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Date           : "));
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, I18N("YYYY/MM/DD"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Time           : "));
  GWEN_Time_toString(ti, I18N("hh:mm:ss"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");

  if (useBankKey) {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Bank Code      : "));
    GWEN_Buffer_AppendString(lbuf, AB_User_GetBankCode(u));
  }
  else {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("User           : "));
    GWEN_Buffer_AppendString(lbuf, AB_User_GetUserId(u));
  }

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Key number     : "));
  snprintf(numbuf, sizeof(numbuf), "%d",
           GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki));
  GWEN_Buffer_AppendString(lbuf, numbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");

  GWEN_Buffer_AppendString(lbuf,
                           I18N("Key version    : "));
  snprintf(numbuf, sizeof(numbuf), "%d",
           GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki));
  GWEN_Buffer_AppendString(lbuf, numbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Customer system: "));
    GWEN_Buffer_AppendString(lbuf, AH_HBCI_GetProductName(h));
    GWEN_Buffer_AppendString(lbuf, "\n");
  }

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Public key for electronic signature"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");

  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Exponent"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");

  /* exponent */
  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  l=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (!p || !l) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad key.");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         I18N("Bad key"));
    return GWEN_ERROR_BAD_DATA;
  }

  bbuf=GWEN_Buffer_new(0, modLen+1, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  if (l<modLen)
    GWEN_Buffer_FillLeftWithBytes(bbuf, 0, modLen-l);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<modLen; i+=16) {
    int rl=modLen-i;
    if (rl>16)
      rl=16;

    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, rl, lbuf, 2, ' ', 0)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
      abort();
    }
    p+=rl;
    GWEN_Buffer_AppendString(lbuf, "\n");
  }

  GWEN_Buffer_AppendBuffer(keybuf, bbuf);
  GWEN_Buffer_free(bbuf);

  /* modulus */
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Modulus"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");
  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  l=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (!p || !l) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad key.");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         I18N("Bad key"));
    return GWEN_ERROR_BAD_DATA;
  }

  bbuf=GWEN_Buffer_new(0, modLen+1, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<modLen; i+=16) {
    int rl=modLen-i;
    if (rl>16)
      rl=16;

    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, rl, lbuf, 2, ' ', 0)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
      abort();
    }
    p+=rl;
    GWEN_Buffer_AppendString(lbuf, "\n");
  }

  GWEN_Buffer_AppendBuffer(keybuf, bbuf);
  GWEN_Buffer_free(bbuf);

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Hash (RMD-160)"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");
  rv=AH_Provider__HashRmd160((const uint8_t *)GWEN_Buffer_GetStart(keybuf),
                             GWEN_Buffer_GetUsedBytes(keybuf),
                             (uint8_t *)hashbuffer);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error hashing (%d)", rv);
    abort();
  }

  GWEN_Buffer_AppendString(lbuf, "  ");
  if (GWEN_Text_ToHexBuffer(hashbuffer, 10, lbuf, 2, ' ', 0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
    abort();
  }
  GWEN_Buffer_AppendString(lbuf, "\n  ");
  if (GWEN_Text_ToHexBuffer(hashbuffer+10, 10, lbuf, 2, ' ', 0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
    abort();
  }
  GWEN_Buffer_AppendString(lbuf, "\n");

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Hash (SHA-256)"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");
  rv=AH_Provider__HashSha256((const uint8_t *)GWEN_Buffer_GetStart(keybuf),
                             GWEN_Buffer_GetUsedBytes(keybuf),
                             (uint8_t *)hashbuffer);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error hashing (%d)", rv);
    abort();
  }
  GWEN_Buffer_free(keybuf);

  GWEN_Buffer_AppendString(lbuf, "  ");
  if (GWEN_Text_ToHexBuffer(hashbuffer, 16, lbuf, 2, ' ', 0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
    abort();
  }
  GWEN_Buffer_AppendString(lbuf, "\n  ");
  if (GWEN_Text_ToHexBuffer(hashbuffer+16, 16, lbuf, 2, ' ', 0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
    abort();
  }
  GWEN_Buffer_AppendString(lbuf, "\n");

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "\n\n");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("I confirm that I created the above key "
                                  "for my electronic signature.\n"));
    GWEN_Buffer_AppendString(lbuf, "\n\n");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("____________________________  "
                                  "____________________________\n"
                                  "Place, date                   "
                                  "Signature\n"));
  }

  return 0;
}


int AH_Provider_GetIniLetterHtml2(AB_PROVIDER *pro,
                                  AB_USER *u,
                                  int useBankKey,
                                  GWEN_BUFFER *lbuf,
                                  int nounmount)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  const void *p;
  unsigned int l;
  GWEN_BUFFER *bbuf;
  GWEN_BUFFER *keybuf;
  int i;
  GWEN_TIME *ti;
  char numbuf[32];
  char hashbuffer[33];
  AH_PROVIDER *hp;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *cctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki=NULL;
  uint32_t kid;
  int rv;
  int modLen;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
                              AH_User_GetTokenType(u),
                              AH_User_GetTokenName(u),
                              &ct);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not get crypt token (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error getting crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* open crypt token */
  rv=GWEN_Crypt_Token_Open(ct, 1, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not open crypt token (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error opening crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* get context */
  cctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), 0);
  if (!cctx) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User context not found on crypt token");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("User context not found on crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_NOT_FOUND;
  }

  if (useBankKey) {
    /* get sign key info */
    kid=GWEN_Crypt_Token_Context_GetVerifyKeyId(cctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                     0);
    }
    if (!ki ||
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      kid=GWEN_Crypt_Token_Context_GetEncipherKeyId(cctx);
      if (kid) {
        ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                       GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                       0);
      }
    }
    if (!ki ||
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Server keys missing, please get them first");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("Server keys missing, "
                                "please get them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    /* get sign key info */
    kid=GWEN_Crypt_Token_Context_GetSignKeyId(cctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                     0);
    }
    if (!ki ||
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
        !(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      DBG_ERROR(AQHBCI_LOGDOMAIN, "User keys missing, please generate them first");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("User keys missing, "
                                "please generate them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }

#if 0
  modLen=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
#else
  /* use the real modulus length */
  modLen=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
#endif

  keybuf=GWEN_Buffer_new(0, 257, 0, 1);

  /* prelude */
  GWEN_Buffer_AppendString(lbuf, "<h3>");
  GWEN_Buffer_AppendString(lbuf, I18N("INI-Letter"));
  GWEN_Buffer_AppendString(lbuf, "</h3>\n");
  GWEN_Buffer_AppendString(lbuf, "<table>\n");
  GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
  GWEN_Buffer_AppendString(lbuf, I18N("Date"));
  GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, I18N("YYYY/MM/DD"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");

  GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
  GWEN_Buffer_AppendString(lbuf, I18N("Time"));
  GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
  GWEN_Time_toString(ti, I18N("hh:mm:ss"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");

  if (useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("Bank Code"));
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, AB_User_GetBankCode(u));
    GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");
  }
  else {
    GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("User"));
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, AB_User_GetUserId(u));
    GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");
  }

  GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
  GWEN_Buffer_AppendString(lbuf, I18N("Key number"));
  GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
  snprintf(numbuf, sizeof(numbuf), "%d",
           GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki));
  GWEN_Buffer_AppendString(lbuf, numbuf);
  GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");

  GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
  GWEN_Buffer_AppendString(lbuf, I18N("Key version"));
  GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
  snprintf(numbuf, sizeof(numbuf), "%d",
           GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki));
  GWEN_Buffer_AppendString(lbuf, numbuf);
  GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("Customer system"));
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, AH_HBCI_GetProductName(h));
    GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");
  }
  GWEN_Buffer_AppendString(lbuf, "</table>\n");

  GWEN_Buffer_AppendString(lbuf, "<h3>");
  GWEN_Buffer_AppendString(lbuf, I18N("Public key for electronic signature"));
  GWEN_Buffer_AppendString(lbuf, "</h3>\n");

  GWEN_Buffer_AppendString(lbuf, "<h4>");
  GWEN_Buffer_AppendString(lbuf, I18N("Exponent"));
  GWEN_Buffer_AppendString(lbuf, "</h4>\n");

  /* exponent */
  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  l=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (!p || !l) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad key.");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         I18N("Bad key"));
    return GWEN_ERROR_BAD_DATA;
  }

  GWEN_Buffer_AppendString(lbuf, "<font face=fixed>\n");
  bbuf=GWEN_Buffer_new(0, modLen+1, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  if (l<modLen)
    GWEN_Buffer_FillLeftWithBytes(bbuf, 0, modLen-l);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<modLen; i+=16) {
    int rl=modLen-i;
    if (rl>16)
      rl=16;

    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, rl, lbuf, 2, ' ', 0)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
      abort();
    }
    p+=rl;
    GWEN_Buffer_AppendString(lbuf, "<br>\n");
  }

  GWEN_Buffer_AppendBuffer(keybuf, bbuf);
  GWEN_Buffer_free(bbuf);
  GWEN_Buffer_AppendString(lbuf, "</font>\n");
  GWEN_Buffer_AppendString(lbuf, "<br>\n");

  /* modulus */
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "<h4>");
  GWEN_Buffer_AppendString(lbuf, I18N("Modulus"));
  GWEN_Buffer_AppendString(lbuf, "</h4>\n");
  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  l=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (!p || !l) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad key.");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         I18N("Bad key"));
    return GWEN_ERROR_BAD_DATA;
  }

  GWEN_Buffer_AppendString(lbuf, "<font face=fixed>\n");
  bbuf=GWEN_Buffer_new(0, modLen+1, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<modLen; i+=16) {
    int rl=modLen-i;
    if (rl>16)
      rl=16;

    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, rl, lbuf, 2, ' ', 0)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
      abort();
    }
    p+=rl;
    GWEN_Buffer_AppendString(lbuf, "<br>\n");
  }

  GWEN_Buffer_AppendBuffer(keybuf, bbuf);
  GWEN_Buffer_free(bbuf);
  GWEN_Buffer_AppendString(lbuf, "</font>\n");
  GWEN_Buffer_AppendString(lbuf, "<br>\n");

  GWEN_Buffer_AppendString(lbuf, "<h4>");
  GWEN_Buffer_AppendString(lbuf, I18N("Hash (RMD-160)"));
  GWEN_Buffer_AppendString(lbuf, "</h4>\n");
  GWEN_Buffer_AppendString(lbuf, "<font face=fixed>\n");
  rv=AH_Provider__HashRmd160((const uint8_t *)GWEN_Buffer_GetStart(keybuf),
                             GWEN_Buffer_GetUsedBytes(keybuf),
                             (uint8_t *)hashbuffer);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error hashing (%d)", rv);
    abort();
  }

  GWEN_Buffer_AppendString(lbuf, "  ");
  if (GWEN_Text_ToHexBuffer(hashbuffer, 20, lbuf, 2, ' ', 0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
    abort();
  }
  GWEN_Buffer_AppendString(lbuf, "</font>\n");
  GWEN_Buffer_AppendString(lbuf, "<br>\n");

  GWEN_Buffer_AppendString(lbuf, "<h4>");
  GWEN_Buffer_AppendString(lbuf, I18N("Hash (SHA-256)"));
  GWEN_Buffer_AppendString(lbuf, "</h4>\n");
  GWEN_Buffer_AppendString(lbuf, "<font face=fixed>\n");
  rv=AH_Provider__HashSha256((const uint8_t *)GWEN_Buffer_GetStart(keybuf),
                             GWEN_Buffer_GetUsedBytes(keybuf),
                             (uint8_t *)hashbuffer);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error hashing (%d)", rv);
    abort();
  }
  GWEN_Buffer_free(keybuf);

  GWEN_Buffer_AppendString(lbuf, "  ");
  if (GWEN_Text_ToHexBuffer(hashbuffer, 32, lbuf, 2, ' ', 0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error converting to hex??");
    abort();
  }
  GWEN_Buffer_AppendString(lbuf, "</font>\n");
  GWEN_Buffer_AppendString(lbuf, "<br>\n");


  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "<br><br>\n");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("I confirm that I created the above key "
                                  "for my electronic signature.\n"));
    GWEN_Buffer_AppendString(lbuf, "<br><br>\n");
    GWEN_Buffer_AppendString(lbuf, "<table>\n");
    GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, "____________________________  ");
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, "____________________________  ");
    GWEN_Buffer_AppendString(lbuf, "</td></tr><tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("Place, date"));
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("Signature"));
    GWEN_Buffer_AppendString(lbuf, "</td></tr></table>\n");
    GWEN_Buffer_AppendString(lbuf, "<br>\n");
  }

  return 0;
}



