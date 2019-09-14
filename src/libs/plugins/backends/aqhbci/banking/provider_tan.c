/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/* This file is included by provider.c */


#include "tanmechanism.h"

#include <gwenhywfar/text.h>

#include <ctype.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int _addTextWithoutTags(const char *s, GWEN_BUFFER *obuf);
static void _addPhrasePleaseEnterTanForUser(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *bufGuiText);

static int _extractChallengeAndText(const char *sChallengeHhd,
                                    const char *sChallenge,
                                    GWEN_BUFFER *bufChallenge,
                                    GWEN_BUFFER *bufGuiText);
static void _copyCompressedCodeIntoBuffer(const char *code, GWEN_BUFFER *cbuf);
static void _keepHhdBytes(GWEN_BUFFER *cbuf);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_Provider_InputTanWithChallenge(AB_PROVIDER *pro,
                                      AB_USER *u,
                                      const AH_TAN_METHOD *tanMethodDescription,
                                      const char *sChallenge,
                                      const char *sChallengeHhd,
                                      char *passwordBuffer,
                                      int passwordMinLen,
                                      int passwordMaxLen)
{
  int rv;
  GWEN_BUFFER *bufGuiText;
  GWEN_BUFFER *bufChallenge;
  AH_TAN_MECHANISM *tanMechanism;

  assert(tanMethodDescription);

  bufGuiText=GWEN_Buffer_new(0, 256, 0, 1);
  _addPhrasePleaseEnterTanForUser(pro, u, bufGuiText);

  bufChallenge=GWEN_Buffer_new(0, 256, 0, 1);

  rv=_extractChallengeAndText(sChallengeHhd, sChallenge, bufChallenge, bufGuiText);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufChallenge);
    GWEN_Buffer_free(bufGuiText);
    return rv;
  }

  tanMechanism=AH_TanMechanism_Factory(tanMethodDescription);
  if (tanMechanism==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not determine TAN mechanism to use");
    return rv;
  }

  rv=AH_TanMechanism_GetTan(tanMechanism,
                            u,
                            I18N("TAN Entry"),
                            GWEN_Buffer_GetStart(bufGuiText),
                            (const uint8_t *) GWEN_Buffer_GetStart(bufChallenge),
                            GWEN_Buffer_GetUsedBytes(bufChallenge),
                            passwordBuffer,
                            passwordMinLen,
                            passwordMaxLen);

  AH_TanMechanism_free(tanMechanism);

  GWEN_Buffer_free(bufChallenge);
  GWEN_Buffer_free(bufGuiText);

  return rv;
}



int _extractChallengeAndText(const char *sChallengeHhd,
                             const char *sChallenge,
                             GWEN_BUFFER *bufChallenge,
                             GWEN_BUFFER *bufGuiText)
{
  if (sChallengeHhd && *sChallengeHhd) {
    int rv;

    DBG_ERROR(AQHBCI_LOGDOMAIN, "ChallengeHHD is [%s]", sChallengeHhd);

    /* use hex-encoded challenge */
    rv=GWEN_Text_FromHexBuffer(sChallengeHhd, bufChallenge);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Created challenge HHUD is:");
    GWEN_Text_LogString(GWEN_Buffer_GetStart(bufChallenge), GWEN_Buffer_GetUsedBytes(bufChallenge),
                        AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error);

    /* get text */
    if (GWEN_Buffer_GetUsedBytes(bufGuiText)>0)
      GWEN_Buffer_AppendString(bufGuiText, "\n");
    if (sChallenge && *sChallenge)
      GWEN_Buffer_AppendString(bufGuiText, sChallenge);
    else
      GWEN_Buffer_AppendString(bufGuiText, I18N("Please enter the TAN from the device."));
    GWEN_Buffer_AppendString(bufGuiText, "\n");
  }
  else if (sChallenge && *sChallenge) {
    const char *s;

    /* look for "CHLGUC" */
    s=GWEN_Text_StrCaseStr(sChallenge, "CHLGUC");
    if (s) {
      GWEN_BUFFER *cbuf;

      /* extract challenge */
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Challenge contains CHLGUC");
      cbuf=GWEN_Buffer_new(0, 256, 0, 1);
      _copyCompressedCodeIntoBuffer(sChallenge, cbuf);
      _keepHhdBytes(cbuf);

      GWEN_Buffer_AppendString(bufChallenge, GWEN_Buffer_GetStart(cbuf));
      GWEN_Buffer_free(cbuf);

      DBG_ERROR(AQHBCI_LOGDOMAIN, "Will use this challenge:");
      GWEN_Buffer_Dump(bufChallenge, 2);

      /* extract text */
      if (GWEN_Buffer_GetUsedBytes(bufGuiText)>0)
        GWEN_Buffer_AppendString(bufGuiText, "\n");

      s=GWEN_Text_StrCaseStr(sChallenge, "CHLGTEXT");
      if (s) {
        /* skip "CHLGTEXT" and 4 digits */
        s+=12;
        /* add rest of the message (replace HTML tags, if any) */
        _addTextWithoutTags(s, bufGuiText);
      }
      else {
        /* create own text */
        GWEN_Buffer_AppendString(bufGuiText, I18N("Please enter the TAN from the device."));
      }
    }
    else {
      /* no optical challenge */
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Challenge contains no optical data");
      GWEN_Buffer_AppendString(bufGuiText, I18N("The server provided the following challenge:"));
      GWEN_Buffer_AppendString(bufGuiText, "\n");
      GWEN_Buffer_AppendString(bufGuiText, sChallenge);
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No challenge data given.");
    return GWEN_ERROR_BAD_DATA;
  }

  return 0;
}



void _copyCompressedCodeIntoBuffer(const char *code, GWEN_BUFFER *cbuf)
{
  const uint8_t *p;

  p=(const uint8_t *)code;
  while (*p) {
    uint8_t c;

    c=toupper(*p);
    if ((c>='0' && c<='9') || (c>='A' && c<='Z') || c==',')
      GWEN_Buffer_AppendByte(cbuf, c);
    p++;
  }
}



void _keepHhdBytes(GWEN_BUFFER *cbuf)
{
  const char *pStart=NULL;
  const char *pEnd=NULL;

  pStart=GWEN_Text_StrCaseStr(GWEN_Buffer_GetStart(cbuf), "CHLGUC");
  if (pStart) {
    pStart+=10; /* skip "CHLGUC" and following 4 digits */
    pEnd=GWEN_Text_StrCaseStr(pStart, "CHLGTEXT");
    if (pStart && pEnd) {
      GWEN_Buffer_Crop(cbuf, pStart-GWEN_Buffer_GetStart(cbuf), pEnd-pStart);
      GWEN_Buffer_SetPos(cbuf, 0);
      GWEN_Buffer_InsertByte(cbuf, '0');
      GWEN_Buffer_SetPos(cbuf, GWEN_Buffer_GetUsedBytes(cbuf));
    }
  }
}



void _addPhrasePleaseEnterTanForUser(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *bufGuiText)
{
  AB_BANKING *ab;
  char buffer[1024];
  const char *sUserName;
  const char *sBankName=NULL;
  AB_BANKINFO *bankInfo;

  assert(u);
  sUserName=AB_User_GetUserId(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  /* find bank name */
  bankInfo=AB_Banking_GetBankInfo(ab, "de", "*", AB_User_GetBankCode(u));
  if (bankInfo)
    sBankName=AB_BankInfo_GetBankName(bankInfo);
  if (!sBankName)
    sBankName=AB_User_GetBankCode(u);

  snprintf(buffer, sizeof(buffer)-1,
           I18N("Please enter the TAN for user %s at %s.\n"), sUserName, sBankName);
  buffer[sizeof(buffer)-1]=0;
  GWEN_Buffer_AppendString(bufGuiText, buffer);
  AB_BankInfo_free(bankInfo);
}



int _addTextWithoutTags(const char *s, GWEN_BUFFER *obuf)
{
  while (*s) {
    if (*s=='<') {
      const char *s2;
      int l;

      s2=s;
      s2++;
      while (*s2 && *s2!='>')
        s2++;
      l=s2-s-2;
      if (l>0) {
        const char *s3;

        s3=s;
        s3++;
        if (l==2) {
          if (strncasecmp(s3, "br", 2)==0)
            GWEN_Buffer_AppendString(obuf, "\n");
        }
        else if (l==3) {
          if (strncasecmp(s3, "br/", 3)==0)
            GWEN_Buffer_AppendString(obuf, "\n");
        }
      }
      s=s2; /* set s to position of closing bracket */
    }
    else
      GWEN_Buffer_AppendByte(obuf, *s);
    /* next char */
    s++;
  }

  return 0;
}
