/***************************************************************************

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobchangekeys_p.h"
#include "../banking/user_l.h"
#include "../banking/provider_l.h"
#include <gwenhywfar/ctplugin.h>
#include <gwenhywfar/ctfile_be.h>

#include <stdarg.h>
#include <unistd.h>


/*
  Moegliche Wechsel:

  medium      type  mode

  datei       RDH   1   karte o. cert RAH 9
  datei       RDH   1   datei     RAH 10
  karte o. cert   RDH   1   karte o. cert RAH 9
  datei       RDH   2   karte o. cert RAH 9
  datei       RDH   2   datei     RAH 10
  karte o. cert   RDH   5   karte o. cert RAH 9

  ohne Schluesselwechsel:
  karte o. cert   RDH   9   karte o. cert RAH 9
  datei       RDH   10    karte o. cert RAH 9
  datei       RDH   10    datei     RAH 10
*/


GWEN_INHERIT(AH_JOB, AH_JOB_CHANGEKEYS)

const char *fmtStr(char *buff, size_t buffLen, const char *fmt, ...)  __attribute__((format(printf, 3, 4)));
const char *fmtStr(char *buff, size_t buffLen, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(buff, buffLen - 1, fmt, args);
  buff[buffLen - 1] = 0;
  va_end(args);
  return buff;
}

#define FB fmtBuff, sizeof(fmtBuff)

int onError(const char *m, int rv)
{
  DBG_ERROR(AQHBCI_LOGDOMAIN, "%s", m);
  GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR,
                      I18N("Change keys: error"), I18N(m), I18N("OK"), NULL, NULL, 0);
  return rv;
}



const char *strUpper(char *s)
{
  size_t i;
  size_t l = strlen(s);
  for (i = 0; i < l; i++) {
    if ((s[i] >= 'a') && (s[i] <= 'z'))
      s[i] -= 0x20;

  }
  return s;
}

int8_t getKeyInfo(AH_HBCI *h, const char *tt, const char *tn, uint32_t cid, GWEN_CRYPT_TOKEN **ct,
                  const GWEN_CRYPT_TOKEN_CONTEXT **ctx,
                  const GWEN_CRYPT_TOKEN_KEYINFO **cryptKeyInfo, const GWEN_CRYPT_TOKEN_KEYINFO **signKeyInfo,
                  const GWEN_CRYPT_TOKEN_KEYINFO **authKeyInfo)
{
  int8_t res = 0;
  uint8_t i;

  uint32_t f = GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS | GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
               GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION | GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER;
  if (!*ct && (((AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h), tt, tn, ct) < 0)) || !*ct)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "AB_Banking_GetCryptToken() failed (tt '%s', tn '%s').", tt, tn);
    return -1;
  }
  if (GWEN_Crypt_Token_Open(*ct, 0, 0) < 0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "GWEN_Crypt_Token_Open() failed.");
    return -1;
  }
  *ctx = GWEN_Crypt_Token_GetContext(*ct, cid, 0);
  if (!*ctx) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "GWEN_Crypt_Token_GetContext() failed (cid %ld).", (long)cid);
    return -1;
  }
  for (i = 0; i < 3; i++) {
    uint32_t kid = 0;
    const GWEN_CRYPT_TOKEN_KEYINFO **ki = NULL;
    char kl = '?';
    switch (i) {
    case 0:
      kid = GWEN_Crypt_Token_Context_GetDecipherKeyId(*ctx);
      ki = cryptKeyInfo, kl = 'V';
      break;
    case 1:
      kid = GWEN_Crypt_Token_Context_GetSignKeyId(*ctx);
      ki = signKeyInfo;
      kl = 'S';
      break;
    case 2:
      kid = GWEN_Crypt_Token_Context_GetAuthSignKeyId(*ctx);
      ki = authKeyInfo;
      kl = 'D';
      break;
    }
    if ((kl == 'D') && !strcmp(tt, "ddvcard"))
      continue;
    if ((*ki = GWEN_Crypt_Token_GetKeyInfo(*ct, kid, f, 0)) == NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "GWEN_Crypt_Token_GetKeyInfo() (%c) failed.", kl);
      res = -1;
      break;
    }
  }
  return res;
}

int8_t tokenHasKeys(GWEN_CRYPT_TOKEN *ct, const GWEN_CRYPT_TOKEN_CONTEXT *ctx)
{
  uint8_t i;
  int8_t res = 2;
  for (i = 0; (res > 0) && (i < 2); i++) {
    const GWEN_CRYPT_TOKEN_KEYINFO *ki = NULL;
    int kn = 0, kv = 0;
    uint32_t flags = 0;
    uint32_t id = (i == 0) ? GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx) : GWEN_Crypt_Token_Context_GetVerifyKeyId(ctx);
    if (res == 2)
      res = 1;
    if (!id)
      res = -1;
    else
      ki = GWEN_Crypt_Token_GetKeyInfo(ct, id, 0, 0);
    if (!ki)
      res = -1;
    else {
      kn = GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki);
      kv = GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki);
      flags = GWEN_Crypt_Token_KeyInfo_GetFlags(ki);
    }
    if ((res >= 0) && (!kn || !kv || !(flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) ||
                       !(flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)))
      res = 0;
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): id %ld %d %d m %d e %d -> %d.", __FUNCTION__, (long)id, kn, kv,
               (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) != 0, (flags & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT) != 0, res);
  }
  return (res == 1) ? 1 : 0;
}

#if 1
int8_t setKeyVersion(GWEN_CRYPT_TOKEN *ct, const GWEN_CRYPT_TOKEN_CONTEXT *ctx, const GWEN_CRYPT_TOKEN_KEYINFO *ki,
                     char t, uint32_t kv)
{
  int8_t ret = 0;
  GWEN_CRYPT_TOKEN_KEYINFO *kin = NULL;
  uint32_t kid = 0;

  switch (t) {
  case 'V':
    kid = GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx);
    break;
  case 'S':
    kid = GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
    break;
  case 'A':
    kid = GWEN_Crypt_Token_Context_GetAuthSignKeyId(ctx);
    break;
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "type %c invalid.", t);
    return -1;
  }

  kin = GWEN_Crypt_Token_KeyInfo_dup(ki);

  if (kv > 999)
    kv = 1;
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): key %ld '%c' set version %ld.", __FUNCTION__, (long)kid, t, (long)kv);
  GWEN_Crypt_Token_KeyInfo_SetKeyVersion(kin, kv);
  if (GWEN_Crypt_Token_SetKeyInfo(ct, kid, kin, 0)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): GWEN_Crypt_Token_SetKeyInfo() failed.", __FUNCTION__);
    ret = -1;
  }
  GWEN_Crypt_Token_KeyInfo_free(kin);
  if (ret)
    return ret;
  if ((ret == 0) && (ki = GWEN_Crypt_Token_GetKeyInfo(ct, kid, 0, 0)) == NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): GWEN_Crypt_Token_GetKeyInfo() (%c) failed.", __FUNCTION__, t);
    ret = -1;
  }
  if (ret)
    return ret;
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): key '%c' version now %d.", __FUNCTION__, t,
             GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki));
  return 0;
}
#endif

#define FJCK_CHMEDIA    1
#define FJCK_CHKEY      2
#define FJCK_CHPROFILE  4
#define FJCK_SRCFILE    8
#define FJCK_DSTFILE    16
#define FJCK_DSTFILE_EXISTS 32

void GWENHYWFAR_CB GWENHYWFAR_CB AH_Job_ChangeKeys_FreeData(void *bp, void *p);
int AH_Job_ChangeKeys_NextMsg(AH_JOB *j);

AH_JOB *AH_Job_ChangeKeys_new(AB_PROVIDER *pro, AB_USER *u, GWEN_DB_NODE *args, uint8_t *canceled)
{
  int res = 0;
  char fmtBuff[256];
  AH_JOB *j = NULL;
  AH_JOB_CHANGEKEYS *jd = NULL;
  AB_USER *uTmp = NULL;
  AH_HBCI *h = AH_Provider_GetHbci(pro);
  uint16_t flags = FJCK_CHKEY | FJCK_CHMEDIA | FJCK_CHPROFILE;
  AH_CRYPT_MODE cryptModeNew = AH_CryptMode_None;
  int cryptTypeNew = -1;
  int tokenCtxIdNew = -1;
  const char *tokenTypeFromToken = NULL;
  const char *tokenNameFromToken = NULL;
  char *wantedTokenType = NULL, *wantedTokenName = NULL;
  const char *wantedCryptMode = NULL;
  GWEN_CRYPT_TOKEN *ct = NULL, *ctNew = NULL;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx = NULL, *ctxNew = NULL;
  const GWEN_CRYPT_TOKEN_KEYINFO *cryptKeyInfo = NULL, *signKeyInfo = NULL, *authKeyInfo = NULL, *kiVNew = NULL,
                                  *kiSNew = NULL, *kiANew = NULL;
  GWEN_PLUGIN *plg = NULL;
  GWEN_PLUGIN_MANAGER *pm = GWEN_PluginManager_FindPluginManager(GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
  const char *cmn = "?", *fm = "?", *fmn = "?";

  assert(h);

  //GWEN_DB_Dump(args, 0);
  wantedTokenType = strdup(GWEN_DB_GetCharValue(args, "tokenType", 0, ""));
  wantedTokenName = strdup(GWEN_DB_GetCharValue(args, "tokenName", 0, ""));
  wantedCryptMode = GWEN_DB_GetCharValue(args, "cryptMode", 0, "");
  cryptTypeNew = GWEN_DB_GetIntValue(args, "cryptType", 0, -1);
  tokenCtxIdNew = GWEN_DB_GetIntValue(args, "context", 0, -1);

  if (wantedCryptMode && *wantedCryptMode) {
    if (!strcasecmp(wantedCryptMode, "RDH"))
      cryptModeNew = AH_CryptMode_Rdh;
    else if (!strcasecmp(wantedCryptMode, "RAH"))
      cryptModeNew = AH_CryptMode_Rah;
  }

  tokenTypeFromToken = AH_User_GetTokenType(u);
  tokenNameFromToken = AH_User_GetTokenName(u);
  if (!strcasecmp(tokenTypeFromToken, "ohbci"))
    flags |= FJCK_SRCFILE;
  if (!*wantedTokenType) {
    free(wantedTokenType);
    wantedTokenType = NULL;
    if (*wantedTokenName && !strchr(wantedTokenName, '/') && !strchr(wantedTokenName, '\\')) { // assume thats a card number
      size_t l = strlen(wantedTokenName);
      size_t i = 0;
      for (; i < l; i++) {
        if ((wantedTokenName[i] < '0') || (wantedTokenName[i] > '9'))
          break;
      }
      if (i == l) {
        wantedTokenType = strdup("card");
        if (l < 10) {
          char *tmp = strdup("0000000000");
          for (i = 0; i < l; i++)
            tmp[9 - i] = wantedTokenName[(l - 1) - i];
          free(wantedTokenName);
          wantedTokenName = tmp;
        }
      }
    }
  }
  if (wantedTokenType && !*wantedTokenType) {
    free(wantedTokenType);
    wantedTokenType = NULL;
  }
  if (!wantedTokenType)
    wantedTokenType = strdup(tokenTypeFromToken);
  if (!strcasecmp(wantedTokenType, "file") || !strcasecmp(wantedTokenType, "ohbci")) {
    flags |= FJCK_DSTFILE;
    free(wantedTokenType);
    wantedTokenType = strdup("ohbci");
  }
  else if (!strstr(wantedTokenType, "card"))
    res = onError(fmtStr(FB, "Invalid token-type '%s'.", wantedTokenType), -1);
  if (!*wantedTokenName) {
    free(wantedTokenName);
    wantedTokenName = strdup(tokenNameFromToken);
    if (!strcasecmp(tokenTypeFromToken, "ohbci"))
      flags |= FJCK_DSTFILE;
  }

  if (res == 0) {
    if (!access(wantedTokenName, F_OK))
      flags |= FJCK_DSTFILE_EXISTS;

    if (!strcasecmp(tokenTypeFromToken, wantedTokenType) && !strcmp(tokenNameFromToken, wantedTokenName)) {
      if (cryptModeNew == AH_CryptMode_None)
        cryptModeNew = AH_User_GetCryptMode(u);
      if (cryptTypeNew < 0)
        cryptTypeNew = AH_User_GetRdhType(u);
    }
    if (cryptModeNew == AH_CryptMode_None)
      res = onError("Crypt-mode must be specified.", -1);
    if ((res == 0) && (cryptTypeNew < 0))
      res = onError("Crypt-typ must specified.", -1);
  }

  if (tokenCtxIdNew < 0)
    tokenCtxIdNew = 1;

  fm = (flags & FJCK_SRCFILE) ? "Keyfile" : "Chipcard";
  fmn = (flags & FJCK_DSTFILE) ? "Keyfile" : "Chipcard";

  DBG_INFO(AQHBCI_LOGDOMAIN, "'%s' '%s' -> '%s' '%s', file %d exists %d'.",
           tokenTypeFromToken, tokenNameFromToken, wantedTokenType, wantedTokenName, (flags & FJCK_DSTFILE) != 0,
           (flags & FJCK_DSTFILE_EXISTS) != 0);

  if (res == 0) {
    if (
      (!((flags & FJCK_SRCFILE)) == !((flags & FJCK_DSTFILE))) &&
      !strcmp(tokenNameFromToken, wantedTokenName)
    )
      res = onError("Keychange without media change is not supported, yet.", -1);
  }

  if (res == 0) {
    switch (cryptModeNew) {
    case AH_CryptMode_Rah:
      cmn = "RAH";
      break;
    case AH_CryptMode_Rdh:
      cmn = "RDH";
      break;
    default:
      cmn = "unknown";
    }
    if (!strcasecmp(tokenTypeFromToken, wantedTokenType) && !strcmp(tokenNameFromToken, wantedTokenName))
      flags &= ~FJCK_CHMEDIA;
    if ((AH_User_GetCryptMode(u) == cryptModeNew) && (AH_User_GetRdhType(u) == cryptTypeNew))
      flags &= ~FJCK_CHPROFILE;
    DBG_INFO(AQHBCI_LOGDOMAIN, "'%s %d' -> '%s' '%s' '%s %d', change: m %d, k %d, p %d.",
             AH_CryptMode_toString(AH_User_GetCryptMode(u)), AH_User_GetRdhType(u),
             wantedTokenType, wantedTokenName, AH_CryptMode_toString(cryptModeNew), cryptTypeNew, (flags & FJCK_CHMEDIA) != 0,
             (flags & FJCK_CHKEY) != 0,
             (flags & FJCK_CHPROFILE) != 0);
    if (flags & FJCK_CHPROFILE) {
      res = -1;
      switch (AH_User_GetCryptMode(u)) {
      case AH_CryptMode_Rdh:
        switch (cryptModeNew) {
        case AH_CryptMode_Rdh:
          onError("Änderung des Schlüsselsprofils nach RDH nicht unterstützt.", -1);
          break;
        default:
          if (!(flags & FJCK_SRCFILE) && !(flags & FJCK_DSTFILE) && (AH_User_GetRdhType(u) == 9) && (cryptTypeNew == 9))
            res = 1;
          else if ((flags & FJCK_SRCFILE) && !(flags & FJCK_DSTFILE) && (AH_User_GetRdhType(u) == 10) && (cryptTypeNew == 9))
            res = 1;
          else if ((flags & FJCK_SRCFILE) && (flags & FJCK_DSTFILE) && (AH_User_GetRdhType(u) == 10) && (cryptTypeNew == 10))
            res = 1;
          if (res == 1) {
            flags &= ~FJCK_CHKEY;
            res = 0;
          }
          else {
            if (flags & FJCK_SRCFILE) {
              switch (AH_User_GetRdhType(u)) {
              case 1:
              case 2:
                if (!(flags & FJCK_DSTFILE) && (cryptTypeNew == 9))
                  res = 0;
                if ((flags & FJCK_DSTFILE) && (cryptTypeNew == 10))
                  res = 0;
                break;
              default:
                ;
              }
            }
            else {
              switch (AH_User_GetRdhType(u)) {
              case 1:
              case 5:
                if (!(flags & FJCK_DSTFILE) && (cryptTypeNew == 9))
                  res = 0;
                break;
              default:
                ;
              }
            }
            if (res)
              onError(fmtStr(FB, "Aenderung des Schluesselsprofils von %s RDH-%d nach %s RAH-%d nicht unterstuetzt.",
                             fm, AH_User_GetRdhType(u), fmn, cryptTypeNew), -1);
          }
        }
        break;
      default:
        onError("Aenderung des Sicherheitsprofils nur von RDH aus unterstuetzt.", -1);
      }
    }
    if (res)
      *canceled = 2;
  }

  if (res == 0) {
    // keyinfo current token
    if (getKeyInfo(h, AH_User_GetTokenType(u), AH_User_GetTokenName(u), AH_User_GetTokenContextId(u), &ct, &ctx,
                   &cryptKeyInfo, &signKeyInfo,
                   &authKeyInfo)
        || !ct || !ctx || !cryptKeyInfo || !signKeyInfo || !authKeyInfo) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "getKeyInfo() ct %p, ctx %p, ki %p %p %p.", ct, ctx, cryptKeyInfo, signKeyInfo, authKeyInfo);
      if (!ct || !ctx)
        res = onError("Could not get token.", -1);
      else
        res = onError("Crypt token type not suitable for this operation.", -1);
    }
  }

  if ((res == 0) && (flags & FJCK_CHMEDIA)) {
    char *tokenNew = NULL;
    if (!strcmp(wantedTokenType, "card")) {
      GWEN_BUFFER *ctn = GWEN_Buffer_new(0, 64, 0, 1);
      GWEN_BUFFER *cmn = GWEN_Buffer_new(0, 64, 0, 1);
      for (; res == 0;) {
        res = AB_Banking_CheckCryptToken(AB_Provider_GetBanking(pro), GWEN_Crypt_Token_Device_Card, ctn, cmn);
        DBG_INFO(AQHBCI_LOGDOMAIN, "card: '%s' '%s'.", GWEN_Buffer_GetStart(ctn), GWEN_Buffer_GetStart(cmn));
        if (res)
          res = onError("AB_Banking_CheckCryptToken() failed.", -1);
        else {
          if (tokenNew)
            free(tokenNew);
          tokenNew = strdup(GWEN_Buffer_GetStart(ctn));
          if (!strcmp(wantedTokenName, GWEN_Buffer_GetStart(cmn)))
            break;
          if (GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO, I18N("Change keys: insert card"),
                                  fmtStr(FB, "Chipcard '%s' needed.", wantedTokenName), I18N("Abort"), I18N("OK"), NULL, 0) != 2)
            res = -2;
        }
      }
      GWEN_Buffer_free(ctn);
      GWEN_Buffer_free(cmn);
      if (tokenNew) {
        free(wantedTokenType);
        wantedTokenType = tokenNew;
        tokenNew = NULL;
      }
    }

    if (tokenNew)
      free(tokenNew);

    if (res == 0)
      plg = GWEN_PluginManager_GetPlugin(pm, wantedTokenType);
    if (!plg && (res == 0))
      res = onError(fmtStr(FB, "Could not get plugin for new tokentype '%s'.", wantedTokenType), -1);
    if (plg && (flags & FJCK_DSTFILE)) {
      // diff. context?
      if ((flags & FJCK_SRCFILE) && !strcmp(tokenNameFromToken, wantedTokenName))
        res = onError(fmtStr(FB, "New and old keyfile must be different."), -1);
      else {
        uint8_t del = 1;
        tokenCtxIdNew = 1;
        if (flags & FJCK_DSTFILE_EXISTS) {
          del = 0;
          res = GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO, I18N("Change keys: confirm"),
                                    fmtStr(FB, "Keyfile '%s' already exists.", wantedTokenName), I18N("Abort"), I18N("Use"), I18N("Delete"), 0);
          if (res == 1)
            res = -1;
          else if (res == 2) { // use
            res = 0;
            //flags &= ~FJCK_CHKEY;
            if (getKeyInfo(h, wantedTokenType, wantedTokenName, tokenCtxIdNew, &ctNew, &ctxNew, &kiVNew, &kiSNew, &kiANew)
                || !ctNew || !ctxNew || !kiVNew || !kiSNew || !kiANew)
              res = onError("Could not get token for new keyfile.", -1);
          }
          else if (GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO, I18N("Change keys: confirm"),
                                       fmtStr(FB, "Really delete keyfile '%s'?", wantedTokenName), I18N("Abort"), I18N("Delete"), NULL, 0) != 2)
            res = -1;
          else {
            res = 0;
            del = 1;
          }
          if (res) {
            res = onError("Canceled.", -1);
            *canceled = 1;
          }
          else if (del)
            unlink(wantedTokenName);
        }
        if ((res == 0) && del) {
          if (GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO, I18N("Change keys: confirm"),
                                  fmtStr(FB, "Schlüsseldatei '%s' wird erzeugt.", wantedTokenName), I18N("Abort"), I18N("OK"), NULL, 0) != 2) {
            res = onError("Canceled.", -1);
            *canceled = 1;
          }
          if (res == 0) {
            ctNew = GWEN_Crypt_Token_Plugin_CreateToken(plg, wantedTokenName);
            if (!ctNew)
              res = onError(fmtStr(FB, "Could not create crypt token '%s'.", wantedTokenName), -1);
            else if (GWEN_Crypt_Token_Create(ctNew, 0) < 0)
              res = onError(fmtStr(FB, "Could not create keyfile '%s'.", GWEN_Crypt_Token_GetTokenName(ctNew)), -1);
            else if (GWEN_Crypt_Token_Close(ctNew, 0, 0))
              res = onError("Could not close token.", -1);
          }
        }
      }
    }
    // keyinfo dest. token
    if (res == 0) {
      if (getKeyInfo(h, wantedTokenType, wantedTokenName, tokenCtxIdNew, &ctNew, &ctxNew, &kiVNew, &kiSNew, &kiANew)
          || !ctNew || !ctxNew || !kiVNew || !kiSNew || !kiANew) {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "getKeyInfo() ct %p, ctx %p, ki %p %p %p.", ct, ctx, cryptKeyInfo, signKeyInfo,
                   authKeyInfo);
        if (!ct || !ctx)
          res = onError("Could not get token.", -1);
        else
          res = onError("Crypt token not suitable for this operation.", -1);
      }
    }
    if ((res == 0) && !(flags & FJCK_DSTFILE)) {
      int tnV = GWEN_Crypt_Token_KeyInfo_GetKeyNumber(kiVNew);
      int tnS = GWEN_Crypt_Token_KeyInfo_GetKeyNumber(kiSNew);
      if ((cryptTypeNew != tnV) || (cryptTypeNew != tnS)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "rdh-type %d, card %d/%d..", cryptTypeNew, tnV, tnS);
        res = onError("Target crypt token not suitable for thius operation.", -1);
      }
    }
  }
#if 1
  if (res == 0) {
    // how get key-pair from token?
    if (!(flags & FJCK_CHKEY) && (flags & FJCK_CHMEDIA))
      res = onError("Change of security profile without change of crypt token not implemented.", -1);
  }
#endif
  if (res == 0) {
    if (flags & FJCK_CHMEDIA) {
      char *cm = strdup(AH_CryptMode_toString(AH_User_GetCryptMode(u)));
      const char *m = fmtStr(FB, "Change crypt token from\n    %s '%s', %s-%d\nto\n    %s '%s', %s-%d?",
                             fm, tokenNameFromToken, strUpper(cm), AH_User_GetRdhType(u), fmn, wantedTokenName, cmn, cryptTypeNew);
      free(cm);
      if (GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO, I18N("Change keys: confirm"),
                              m, I18N("Abort"), I18N("OK"), NULL, 0) != 2) {
        res = -1;
        *canceled = 1;
      }
    }
  }

  if (res == 0) {
    uTmp = AB_Provider_CreateUserObject(pro);
    AH_User_SetCryptMode(uTmp, cryptModeNew);
    AH_User_SetRdhType(uTmp, cryptTypeNew);
    AH_User_SetTokenType(uTmp, wantedTokenType);
    AH_User_SetTokenName(uTmp, wantedTokenName);
    AH_User_SetTokenContextId(uTmp, tokenCtxIdNew);
    AB_User_SetBankCode(uTmp, AB_User_GetBankCode(u));
    AH_User_SetHbciVersion(uTmp, AH_User_GetHbciVersion(u));
    AH_User_SetServerUrl(uTmp, AH_User_GetServerUrl(u));

    DBG_NOTICE(AQHBCI_LOGDOMAIN, "chng k %d p %d m %d, token: open %d ctx %d knr dst %d %d %d.",
               (flags & FJCK_CHKEY) != 0, (flags & FJCK_CHPROFILE) != 0, (flags & FJCK_CHMEDIA) != 0, GWEN_Crypt_Token_IsOpen(ctNew),
               tokenCtxIdNew,
               kiVNew ? GWEN_Crypt_Token_KeyInfo_GetKeyNumber(kiVNew) : - 1,
               kiSNew ? GWEN_Crypt_Token_KeyInfo_GetKeyNumber(kiSNew) : -1,
               kiANew ? GWEN_Crypt_Token_KeyInfo_GetKeyNumber(kiANew) : -1);
  }

  if (res == 0) {
    j = AH_Job_new((flags & FJCK_DSTFILE) ? "JobChangeKeys" : "JobChangeKeysA", pro, u, NULL, 0);
    DBG_INFO(AQHBCI_LOGDOMAIN, "%s(): j %p u %p '%s'.", __FUNCTION__, j, u, j ? AH_Job_GetCode(j) : "-");
    if (!j)
      res = onError("AH_Job_new() failed.", -1);
  }

  if (res == 0) {
    GWEN_NEW_OBJECT(AH_JOB_CHANGEKEYS, jd);
    GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_CHANGEKEYS, j, jd, AH_Job_ChangeKeys_FreeData);

    AH_Job_SetNextMsgFn(j, AH_Job_ChangeKeys_NextMsg);

    args = AH_Job_GetArguments(j);
    assert(args);
  }

  if (res == 0) {
    char *cm = strdup(AH_CryptMode_toString(cryptModeNew));
    uint8_t i;

    strUpper(cm);
    for (i = 0; i < 3; i++) {
      const char *kt = "?";
      GWEN_DB_NODE *db = NULL;

      if (i < 2) {
        switch (i) {
        case 0:
          kt = "V";
          db = GWEN_DB_GetGroup(args, GWEN_DB_FLAGS_DEFAULT, "getcryptKey");
          break;
        case 1:
          kt = "S";
          db = GWEN_DB_GetGroup(args, GWEN_DB_FLAGS_DEFAULT, "getsignKey");
          break;
        }

        // HKISA
        GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "secProfile/code", cm);
        GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "secProfile/version", cryptTypeNew);

        GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/userid", AH_User_GetPeerId(u));
        GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/keyType", kt);
        GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/keynum", 999);
        GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/keyversion", 999);
      }

      switch (i) {
      case 0:
        kt = "V";
        db = GWEN_DB_GetGroup(args, GWEN_DB_FLAGS_DEFAULT, "setcryptKey");
        break;
      case 1:
        kt = "S";
        db = GWEN_DB_GetGroup(args, GWEN_DB_FLAGS_DEFAULT, "setsignKey");
        break;
      case 2:
        kt = "D";
        db = GWEN_DB_GetGroup(args, GWEN_DB_FLAGS_DEFAULT, "setauthKey");
        break;
      }
      // HKSAK
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "secProfile/code", cm);
      GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "secProfile/version", cryptTypeNew);
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/userid", AH_User_GetPeerId(u));
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/keyType", kt);
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "keyName/keyType", kt);
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "keyName/userid", AH_User_GetPeerId(u));
    }
    free(cm);
  }
  if (res) {
    if (j)
      AH_Job_free(j);
    j = NULL;
    if (res == -2)
      *canceled = 2;
  }

  if (jd) {
    jd->flags = flags;
    jd->canceled = canceled;
    jd->pro = pro;
    jd->u = u;
    jd->uTmp = uTmp;
    jd->fm = fmn;
    jd->tokenType = wantedTokenType;
    jd->tokenName = wantedTokenName;
    jd->currentCryptKeyVersion = cryptKeyInfo ? GWEN_Crypt_Token_KeyInfo_GetKeyVersion(cryptKeyInfo) : 0;
    jd->currentSignKeyVersion = signKeyInfo ? GWEN_Crypt_Token_KeyInfo_GetKeyVersion(signKeyInfo) : 0;
    jd->currentAuthKeyVersion = authKeyInfo ? GWEN_Crypt_Token_KeyInfo_GetKeyVersion(authKeyInfo) : 0;
    jd->ct = ctNew;
    jd->ctx = ctxNew;
    jd->tokenCtxId = tokenCtxIdNew;
    jd->cryptKeyInfo = kiVNew;
    jd->signKeyInfo = kiSNew;
    jd->authKeyInfo = kiANew;
    jd->resp = -1;
    jd->emsg = NULL;
  }

  if (ctNew)
    GWEN_Crypt_Token_Close(ctNew, 0, 0);
  if (ct)
    GWEN_Crypt_Token_Close(ct, 0, 0);

  return j;
}

int onServerKeysImported(AH_JOB_CHANGEKEYS *jd)
{
  // serverkeys imported (knowing server-keys length is necessary for some key-types when created)
  // create keys if required

  char fmtBuff[256];
  AH_HBCI *h = AH_Provider_GetHbci(jd->pro);
  int res = 0;
  const char *m = NULL;
  const char *btn1 = NULL, *btn2 = NULL, *btn3 = NULL;
  uint8_t ok = 0;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): flags 0x%04X.", __FUNCTION__, jd->flags);
  if (jd->flags & FJCK_CHKEY) {
    //if(!jd->cryptKeyInfo || !GWEN_Crypt_Token_KeyInfo_GetKeyNumber(jd->cryptKeyInfo) || !jd->signKeyInfo || !GWEN_Crypt_Token_KeyInfo_GetKeyNumber(jd->signKeyInfo))
    if (!tokenHasKeys(jd->ct, jd->ctx)) {
      m = fmtStr(FB, "Creating new keys on %s '%s'.", jd->fm, jd->tokenName);
      btn1 = "Abort";
      btn2 = "OK";
      ok = 2;
    }
    else {
#if 0
      // TODO check keys are compatible with cryptmode
      // if so, ...
      {
        m = fmtStr(FB, "Auf %s '%s' sind Schluessel vorhanden,\nsollen dennoch neue Schluessel erzeugt werden?", fmn, wantedTokenName);
        btn1 = "Abort";
        btn2 = "No";
        btn3 = "Yes";
        ok = 3;
      }
      else {
        m = fmtStr(FB, "Auf %s '%s' vorhandene Schluessel koennen nicht verwendet werden,\nneue Schluessel werden erzeugt.",
                   fmn, wantedTokenName);
        btn1 = "Abort";
        btn2 = "OK";
        ok = 2;
      }
#else
      m = fmtStr(FB, "Auf %s '%s' sind Schluessel vorhanden die verwendet werden koennen,\n"
                 "wenn sie zum gewuehlten Verschluesselungsverfahren passen.\n"
                 "Sollen neue Schluessel erzeugt werden?", jd->fm, jd->tokenName);
      btn1 = "Abort";
      btn2 = "No";
      btn3 = "Yes";
      ok = 3;
#endif
    }
    res = GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO, I18N("Change keys: confirm"),
                              m, I18N(btn1), I18N(btn2), btn3 ? I18N(btn3) : NULL, 0);
    if (res == 1) {
      res = -1;
      *jd->canceled = 1;
    }
    else {
      if (res == ok) {
        res = 0;
        DBG_INFO(AQHBCI_LOGDOMAIN, "creating keys...");
        res = AH_Provider_CreateKeys(jd->pro, jd->uTmp, 1);
        if (res)
          res = onError(fmtStr(FB, "Could not create keys (%d).", res), -1);
        DBG_INFO(AQHBCI_LOGDOMAIN, "creating keys done.");
      }
      else
        res = 0;
    }
  }
  else
    res = -1;
  if ((res == 0) &&
      (getKeyInfo(h, jd->tokenType, jd->tokenName, jd->tokenCtxId, &jd->ct, &jd->ctx, &jd->cryptKeyInfo, &jd->signKeyInfo,
                  &jd->authKeyInfo)
       || !jd->ct || !jd->ctx || !jd->cryptKeyInfo || !jd->signKeyInfo || !jd->authKeyInfo))
    res = onError("Could not get key-info.", -1);

  if (res == 0) {
#if 1
    if (!(jd->flags & FJCK_CHPROFILE)) {
      // set keyversion from current token + 1 on dest.-token
      uint32_t kvV = jd->currentCryptKeyVersion ? (jd->currentCryptKeyVersion + 1) : 0;
      uint32_t kvS = jd->currentSignKeyVersion ? (jd->currentSignKeyVersion + 1) : 0;
      uint32_t kvA = jd->currentAuthKeyVersion ? (jd->currentAuthKeyVersion + 1) : 0;

      DBG_NOTICE(AQHBCI_LOGDOMAIN, "set keyversions %ld -> %ld, %ld -> %ld, %ld -> %ld.",
                 (long)jd->currentCryptKeyVersion, (long)kvV, (long)jd->currentSignKeyVersion, (long)kvS,
                 (long)jd->currentAuthKeyVersion, (long)kvA);
      if (kvV)
        setKeyVersion((GWEN_CRYPT_TOKEN *)jd->ct, jd->ctx, jd->cryptKeyInfo, 'V', kvV);
      if (kvS)
        setKeyVersion((GWEN_CRYPT_TOKEN *)jd->ct, jd->ctx, jd->signKeyInfo, 'S', kvS);
      if (!(jd->flags & FJCK_DSTFILE) && kvA)
        setKeyVersion((GWEN_CRYPT_TOKEN *)jd->ct, jd->ctx, jd->authKeyInfo, 'A', kvA);
    }
#endif
    if (jd->flags & FJCK_CHKEY) {
      if (!jd->signKeyInfo || !GWEN_Crypt_Token_KeyInfo_GetKeyNumber(jd->signKeyInfo)) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Kein Signierschluessel.");
        res = onError("Kein Signierschluessel auf dem Ziel-medium gefunden.", -1);
      }
      else {
        int sc = GWEN_Crypt_Token_KeyInfo_GetSignCounter(jd->signKeyInfo);
        DBG_INFO(AQHBCI_LOGDOMAIN, "%s(): sig counter %d.", __FUNCTION__, sc);
        if (sc > 1) {
          if (!(jd->flags & FJCK_DSTFILE))
            res = onError("Der Sequenzzaehler kann nicht zurueckgesetzt werden.", -1);
          else {
            res = GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO, I18N("Change keys: confirm"),
                                      "Der Sequenzzaehler wird zurueckgesetzt.", I18N("Abort"), I18N("OK"), NULL, 0);
            if (res != 2)
              res = -1;
            else
              res = 0;
          }
          if (res == 0) {
            DBG_INFO(AQHBCI_LOGDOMAIN, "%s(): reset sig counter.", __FUNCTION__);
            GWEN_Crypt_Token_KeyInfo_SetSignCounter((GWEN_CRYPT_TOKEN_KEYINFO *)jd->signKeyInfo, 1);
          }
        }
      }
    }
  }

  if (res == 0) {
    if (getKeyInfo(h, jd->tokenType, jd->tokenName, jd->tokenCtxId, &jd->ct, &jd->ctx, &jd->cryptKeyInfo, &jd->signKeyInfo,
                   &jd->authKeyInfo)
        || !jd->ct || !jd->ctx || !jd->cryptKeyInfo || !jd->signKeyInfo || !jd->authKeyInfo)
      res = onError("Could not get key-info.", -1);
  }

  if ((res == 0) && !jd->ctx)
    res = onError("Missing new ctx.", -1);

  return res;
}

#define RSP_NOSRVRSP  1
#define RSP_WARN    2
#define RSP_ERR     3

int8_t parseResponse(AH_JOB *j)
{
  int8_t res = 0;
  int rc = 0;
  uint8_t gotResp = 0;
  AH_JOB_CHANGEKEYS *jd = GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CHANGEKEYS, j);
  GWEN_DB_NODE *n = AH_Job_GetResponses(j);
  if (AH_Job_GetStatus(j) != AH_JobStatusAnswered)
    rc = -1;
  assert(n);
  jd->emsg = GWEN_Buffer_new(NULL, 2048, 0, 0);
  n = GWEN_DB_GetFirstGroup(n);
  while (n) {
    //GWEN_DB_Dump(n, 0);
    if (!strcmp(GWEN_DB_GroupName(n), "SegResult")) {
      int mn = GWEN_DB_GetIntValue(n, "security/msgnum", 0, -1);
      if (mn == 2) {
        gotResp = 1;
        if ((rc >= 0) && (rc < 9000)) {
          rc = GWEN_DB_GetIntValue(n, "data/SegResult/result/resultcode", 0, -1);
          if (rc == 3250) {
            DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): response %d tells us: no HKEND.", __FUNCTION__, rc);
            res = 1;
            rc = 0;
          }
          else {
            GWEN_Buffer_AppendString(jd->emsg, GWEN_DB_GetCharValue(n, "data/SegResult/result/text", 0, "?"));
            GWEN_Buffer_AppendString(jd->emsg, "\n");
          }
        }
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "result %d.", rc);
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "result '%s'.", GWEN_Buffer_GetStart(jd->emsg));
      }
      else {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): #%d result %d '%s'.", __FUNCTION__,
                   GWEN_DB_GetIntValue(n, "security/msgnum", 0, -1),
                   GWEN_DB_GetIntValue(n, "data/SegResult/result/resultcode", 0, -1),
                   GWEN_DB_GetCharValue(n, "data/SegResult/result/text", 0, "?"));
        if (GWEN_DB_GetIntValue(n, "data/SegResult/result/resultcode", 0, -1) >= 9000) {
          rc = GWEN_DB_GetIntValue(n, "data/SegResult/result/resultcode", 0, -1);
          GWEN_Buffer_AppendString(jd->emsg, GWEN_DB_GetCharValue(n, "data/SegResult/result/text", 0, "?"));
          GWEN_Buffer_AppendString(jd->emsg, "\n");
        }
      }
    }
    n = GWEN_DB_GetNextGroup(n);
  }
  if (!gotResp)
    jd->resp = RSP_NOSRVRSP;
  else
    jd->resp = (rc < 3000) ? 0 : (rc < 9000) ? RSP_WARN : RSP_ERR;
  return res;
}

void GWENHYWFAR_CB GWENHYWFAR_CB AH_Job_ChangeKeys_FreeData(void *bp, void *p)
{
  AH_JOB_CHANGEKEYS *jd = (AH_JOB_CHANGEKEYS *)p;
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): %p uTmp %p.", __FUNCTION__, jd, jd->uTmp);

  if (jd->uTmp)
    AB_User_free(jd->uTmp);
  if (jd->tokenType)
    free(jd->tokenType);
  if (jd->tokenName)
    free(jd->tokenName);
  if (jd->emsg)
    GWEN_Buffer_free(jd->emsg);

  GWEN_FREE_OBJECT(jd);
}

int AH_Job_ChangeKeys_NextMsg(AH_JOB *j)
{
  int rv = 0;
  int mn = -1;
  unsigned int jmn = 0;
  GWEN_DB_NODE *dbr = NULL;
  AH_JOB_CHANGEKEYS *jd;
  assert(j);
  jmn = AH_Job_GetMsgNum(j);
  dbr = AH_Job_GetResponses(j);
  dbr = GWEN_DB_GetFirstGroup(dbr);
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): jmn %d.", __FUNCTION__, jmn);
  while (dbr) {
    //GWEN_DB_Dump(dbr, 0);
    rv = AH_Job_CheckEncryption(j, dbr);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): AH_Job_CheckEncryption() failed (%d).", __FUNCTION__, rv);
      return -1;
    }
    rv = AH_Job_CheckSignature(j, dbr);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): AH_Job_CheckSignature() failed (%d).", __FUNCTION__, rv);
      return -1;
    }
    if (!strcasecmp(GWEN_DB_GroupName(dbr), "MsgTail")) {
      mn = GWEN_DB_GetIntValue(dbr, "security/msgnum", 0, -1);
      if (mn < 0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "%s: find msgnum failed.", __FUNCTION__);
      }
      else if ((mn == 1) && (jmn == mn)) {
        const GWEN_CRYPT_KEY *bk = NULL;
        GWEN_CRYPT_KEY *bkCurrV = NULL, *bkCurrS = NULL;
        jd = GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CHANGEKEYS, j);
        assert(jd);
        // get actual serverkeys to restore later
        bk = AH_User_GetBankPubCryptKey(jd->u);
        if (bk)
          bkCurrV = GWEN_Crypt_KeyRsa_dup(bk);
        bk = AH_User_GetBankPubSignKey(jd->u);
        if (bk)
          bkCurrS = GWEN_Crypt_KeyRsa_dup(bk);
        rv = AH_Job_CommitSystemData(j, 0);
        if (rv != 0) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): AH_Job_CommitSystemData() failed(%d).", __FUNCTION__, rv);
        }
        else if (!GWEN_Crypt_Token_IsOpen(jd->ct) && (GWEN_Crypt_Token_Open(jd->ct, 0, 0) < 0))
          rv = onError("GWEN_Crypt_Token_Open() failed.", -1);
        else {
          uint8_t i;
          GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Serverkeys imported."));
          DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): serverkeys should be imported now.", __FUNCTION__);
          // some cryptmodes need length of serverkeys
          // AH_Provider_CreateKeys() reads the length from user-token,
          // which is the temporary user with the new token,
          // so store server-keys from actual user (the now imported) on the new token
          //DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): ctx %p.", __FUNCTION__, jd->ctx);
          jd->ctx = GWEN_Crypt_Token_GetContext(jd->ct, jd->tokenCtxId, 0);
          //DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): -> ctx %p.", __FUNCTION__, jd->ctx);
          for (i = 0; (rv == 0) && (i < 2); i++) {
            const GWEN_CRYPT_TOKEN_KEYINFO *tmp = NULL;
            uint8_t *m = NULL, *e = NULL;
            uint32_t ml = 0, el = 0;
            uint32_t id = 0;
            bk = NULL;
            switch (i) {
            case 0:
              bk = AH_User_GetBankPubCryptKey(jd->u);
              id = GWEN_Crypt_Token_Context_GetEncipherKeyId(jd->ctx);
              AH_User_SetBankPubCryptKey(jd->uTmp, (GWEN_CRYPT_KEY *)bk);
              break;
            case 1:
              bk = AH_User_GetBankPubSignKey(jd->u);
              id = GWEN_Crypt_Token_Context_GetVerifyKeyId(jd->ctx);
              AH_User_SetBankPubSignKey(jd->uTmp, (GWEN_CRYPT_KEY *)bk);
              break;
            }

            if (!bk) {
              if (i == 0) {
                DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): get bankkey failed.", __FUNCTION__);
                rv = -1;
              }
              else
                DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): bank has no signkey.", __FUNCTION__);
              continue;
            }

            ml = GWEN_Crypt_Key_GetKeySize(bk);
            el = 3;

            if (ml) {
              m = malloc(ml);
              e = malloc(el);
              GWEN_Crypt_KeyRsa_GetModulus(bk, m, &ml);
              GWEN_Crypt_KeyRsa_GetExponent(bk, e, &el);
            }
            tmp = GWEN_Crypt_Token_GetKeyInfo(jd->ct, id, 0, 0);
            DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): k %ld m %ld %p e %ld %p.", __FUNCTION__, (long)id, (long)ml, m, (long)el, e);
            if (!tmp) {
              DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): damn, get keyinfo '%c' failed.", __FUNCTION__, (i == 0) ? 'V' : 'S');
              rv = onError("GWEN_Crypt_Token_GetKeyInfo() failed.", -1);
            }
            if (rv == 0) {
              GWEN_CRYPT_TOKEN_KEYINFO *ki = GWEN_Crypt_Token_KeyInfo_dup(tmp);
              uint32_t flags = GWEN_Crypt_Token_KeyInfo_GetFlags(ki);
              GWEN_Crypt_Token_KeyInfo_SetFlags(ki,
                                                flags | GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS | GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT);
              GWEN_Crypt_Token_KeyInfo_SetModulus(ki, m, ml);
              GWEN_Crypt_Token_KeyInfo_SetExponent(ki, e, el);
              GWEN_Crypt_Token_KeyInfo_SetKeySize(ki, ml);
              GWEN_Crypt_Token_KeyInfo_SetKeyNumber(ki, GWEN_Crypt_Key_GetKeyNumber(bk));
              GWEN_Crypt_Token_KeyInfo_SetKeyVersion(ki, GWEN_Crypt_Key_GetKeyVersion(bk));
              if (GWEN_Crypt_Token_SetKeyInfo(jd->ct, id, ki, 0)) {
                DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): GWEN_Crypt_Token_SetKeyInfo() failed.", __FUNCTION__);
                rv = -1;
              }
              else
                ki = NULL;
              if (ki)
                GWEN_Crypt_Token_KeyInfo_free(ki);
            }
            if (m)
              free(m);
            if (e)
              free(e);
          }
          if (rv == 0) {
            rv = onServerKeysImported(jd);
            if (rv != 0)
              DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): onServerKeysImported() failed.", __FUNCTION__);
          }
          // serverkeys set in job_commit() must restored
          if (bkCurrV) {
            AH_User_SetBankPubCryptKey(jd->u, bkCurrV);
            DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): serverkey 'V' restored.", __FUNCTION__);
            GWEN_Crypt_Key_free(bkCurrV);
          }
          if (bkCurrS) {
            AH_User_SetBankPubSignKey(jd->u, bkCurrS);
            DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): serverkey 'S' restored.", __FUNCTION__);
            GWEN_Crypt_Key_free(bkCurrS);
          }
          if (rv == 0) {
            uint8_t i;

            // update segment-data
            GWEN_DB_NODE *args = AH_Job_GetArguments(j);
            assert(args);
            for (i = 0; (rv == 0) && (i < 3); i++) {
              GWEN_DB_NODE *db = NULL;
              const GWEN_CRYPT_TOKEN_KEYINFO *ki = NULL;
              const uint8_t *kd = NULL;
              uint32_t kdsz = 0;
              const char *kt = "?";
              int kn = 0, kv = 0;

              switch (i) {
              case 0:
                kt = "V";
                ki = jd->cryptKeyInfo;
                db = GWEN_DB_GetGroup(args, GWEN_DB_FLAGS_DEFAULT, "setcryptKey");
                break;
              case 1:
                kt = "S";
                ki = jd->signKeyInfo;
                db = GWEN_DB_GetGroup(args, GWEN_DB_FLAGS_DEFAULT, "setsignKey");
                break;
              case 2:
                kt = "D";
                ki = jd->authKeyInfo;
                db = GWEN_DB_GetGroup(args, GWEN_DB_FLAGS_DEFAULT, "setauthKey");
                break;
              }

              if (!ki) {
                if (i < 2) {
                  DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): get keyinfo '%s' failed.", __FUNCTION__, kt);
                  rv = -1;
                }
                else
                  DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): token has no authkey.", __FUNCTION__);
                continue;
              }

              kd = GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
              kdsz = GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
              if (!kd || !kdsz) {
                DBG_ERROR(AQHBCI_LOGDOMAIN, "No modulus in '%s' key.", kt);
                rv = -1;
                break;
              }
              GWEN_DB_SetBinValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/modulus", kd, kdsz);

              kd = GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
              kdsz = GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
              if (!kd || !kdsz) {
                DBG_ERROR(AQHBCI_LOGDOMAIN, "No exponent in '%s' key.", kt);
                rv = -1;
                break;
              }
              GWEN_DB_SetBinValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/exponent", kd, kdsz);

              kd = GWEN_Crypt_Token_KeyInfo_GetCertificateData(ki);
              kdsz = GWEN_Crypt_Token_KeyInfo_GetCertificateLen(ki);
              if (kd && kdsz) {
                GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "cert/type", GWEN_Crypt_Token_KeyInfo_GetCertType(ki));
                GWEN_DB_SetBinValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "cert/cert", kd, kdsz);
              }
              else
                DBG_NOTICE(AQHBCI_LOGDOMAIN, "No cert for '%s' on token.", kt);

              kn = GWEN_Crypt_Token_KeyInfo_GetKeyNumber(ki);
              kv = GWEN_Crypt_Token_KeyInfo_GetKeyVersion(ki);
              GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/keynum", kn);
              GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "key/keyversion", kv);
              GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "keyName/keynum", kn);
              GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "keyName/keyversion", kv);
            }
          }
        }
      }
      else if ((mn == 2) && (jmn == mn)) {
        int8_t resp = parseResponse(j);
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s: resp %d, %s HKEND.", __FUNCTION__, resp, (resp == 1) ? "no" : "next");
        if (resp == 1) // since 5.99.25 no chance to prevent HKEND :-(
          return 0;
      }
    }
    dbr = GWEN_DB_GetNextGroup(dbr);
  }
  return (rv == 0) ? 1 : -1;
}

int AH_Job_ChangeKeys_finish(AB_PROVIDER *pro, AH_JOB *job, int res)
{
  AH_JOB_CHANGEKEYS *jd = NULL;
  AB_USER *u = NULL;
  AB_USER *uTmp = NULL;
  if (!job)
    return res;
  jd = GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_CHANGEKEYS, job);
  if (*jd->canceled) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): user canceled.", __FUNCTION__);
    jd->resp = -1;
  }
  u = jd->u;
  uTmp = jd->uTmp;

  res = -1;
  if (jd->resp >= 0) {
    char fmtBuff[1024];
    const char *m = NULL;
    const char *btn1 = I18N("Abort");
    const char *btn2 = I18N("Finish");
    if (jd->resp == RSP_NOSRVRSP) {
      GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO, I18N("Change keys: Error"),
                          "Der Bankserver hat keine Antwort zur Schluesselaenderung gesendet.", I18N("OK"), NULL, NULL, 0);
      res = -1;
    }
    else {
      if ((jd->resp == RSP_WARN) || (jd->resp == RSP_ERR)) {
        m = strdup(fmtStr(FB, "Der Bankserver meldet %s:\n\n'%s'\n\n"
                          "Es ist moeglich, dass der neue Schluessel dennoch angenommenn wurde.\n"
                          "Neue Schluessel / neues Medium uebernehmen?", (jd->resp == RSP_ERR) ? "Fehler" : "Warnungen",
                          GWEN_Buffer_GetStart(jd->emsg)));
        btn1 = I18N("No");
        btn2 = fmtStr(FB, "%s, %s", I18N("Yes"), I18N("finish"));
      }
      else {
        m = strdup(fmtStr(FB, "Die Uebermittlung der Schluessel ergab keinen Fehler,\n"
                          "aus den Meldungen des Bankservers sollte eine Uebernahme ersichtlich sein:\n\n%s\nNeue Schluessel / neues Medium\n"
                          "wird uebernommen.", GWEN_Buffer_GetStart(jd->emsg)));
      }
      if (GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO, I18N("Change keys: confirm"),
                              m, btn1, btn2, NULL, 0) == 2)
        res = 0;
    }
    if (m)
      free((char *)m);
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "%s(): %d/%d %p %p.", __FUNCTION__, jd->resp, res, u, uTmp);
  if (uTmp) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "%s: tokenTypeFromToken '%s' tokenNameFromToken '%s' rdh %d cm %d ctx-id %d.", __FUNCTION__,
             AH_User_GetTokenType(uTmp), AH_User_GetTokenName(uTmp), AH_User_GetRdhType(uTmp),
             AH_User_GetCryptMode(uTmp), AH_User_GetTokenContextId(uTmp));
    if (res == 0) {
      if (AB_Provider_BeginExclUseUser(pro, u)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): AB_Provider_BeginExclUseUser() failed.", __FUNCTION__);
        res = -1;
      }
      else {
        AH_User_SetBankPubCryptKey(u, AH_User_GetBankPubCryptKey(uTmp));
        AH_User_SetBankPubSignKey(u, AH_User_GetBankPubSignKey(uTmp));
        AH_User_SetTokenType(u, AH_User_GetTokenType(uTmp));
        AH_User_SetTokenName(u, AH_User_GetTokenName(uTmp));
        AH_User_SetRdhType(u, AH_User_GetRdhType(uTmp));
        AH_User_SetCryptMode(u, AH_User_GetCryptMode(uTmp));
        AH_User_SetTokenContextId(u, AH_User_GetTokenContextId(uTmp));
        if (AB_Provider_EndExclUseUser(pro, u, 0)) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "%s(): AB_Provider_EndExclUseUser() failed.", __FUNCTION__);
          res = -1;
        }
      }
    }
    AB_Provider_DeleteUser(pro, AB_User_GetUniqueId(uTmp));
  }

  return res;
}
