/***************************************************************************
    begin       : Sat Dec 10 2011
    copyright   : (C) 2011 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "aqhbci_l.h"
#include "job_l.h"

#include "hhd_p.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>

#include <ctype.h>




int AH_HHD14_ReadBytesDec(const char *p, int len) {
  int r=0;
  int i;
  const char *pSave;

  pSave=p;
  for (i=0; i<len; i++) {
    uint8_t c;

    c=*p;
    if (c==0)
      break;
    if (c<'0' || c>'9') {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad char in data (no decimal digit), pos=%d, byte=%02x", i, c);
      GWEN_Text_LogString(pSave, len, AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error);
      return GWEN_ERROR_INVALID;
    }
    c-='0';
    r*=10;
    r+=c;
    p++;
  }

  return r;
}



int AH_HHD14_ReadBytesHex(const char *p, int len) {
  unsigned int r=0;
  int i;

  for (i=0; i<len; i++) {
    uint8_t c;

    c=*p;
    if (c==0)
      break;
    c=toupper(c);
    if ((c>='0' && c<='9') || (c>='A' && c<='F')) {
      c-='0';
      if (c>9)
        c-=7;
      r*=16;
      r+=c;
      p++;
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad char in data (no hexadecimal digit)");
      return GWEN_ERROR_INVALID;
    }
  }

  return (int) r;
}



unsigned int AH_HHD14_Quersumme(unsigned int i) {
  unsigned int qs=0;

  while(i) {
    qs+=i % 10;
    i/=10;
  }
  return qs;
}



int AH_HHD14_ExtractDataForLuhnSum(const char *code, GWEN_BUFFER *xbuf) {
  int rv;
  unsigned int len;
  unsigned int i=0;

  rv=AH_HHD14_ReadBytesHex(code, 2);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d) at [%s]", rv, code);
    return rv;
  }
  len=((unsigned int) rv);
  code+=2;
  if ((strlen(code)+2)<len*2) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Too few bytes in buffer (%d<%d) at [%s]",
              (int)(strlen(code)+2), len*2, code);
    return GWEN_ERROR_INVALID;
  }

  while(i<len-2) {
    unsigned int v;

    rv=AH_HHD14_ReadBytesHex(code, 2);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d) at [%s]", rv, code);
      return rv;
    }
    v=((unsigned int) rv) & 0xf;
    code+=2;
    GWEN_Buffer_AppendBytes(xbuf, code, v*2);
    code+=v*2;
    i+=v+1;
  }

  return 0;
}



int AH_HHD14_CalcLuhnSum(const char *code, int len) {
  int i;
  int sum=0;
  int next;
  int dif;

  if (len % 2) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid number of bytes in payload (%d)", len);
    return GWEN_ERROR_INVALID;
  }

  for (i=0; i<len; i+=2) {
    int rv;
    unsigned int v;

    rv=AH_HHD14_ReadBytesHex(code, 2);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d) at [%s]", rv, code);
      return rv;
    }
    v=(unsigned int) rv;
    sum+=(1*((v>>4) & 0xf)) + (AH_HHD14_Quersumme(2*(v & 0xf)));
    code+=2;
  }

  next=10*((sum+9)/10);

  dif=next-sum;
  return (unsigned int) dif;
}



int AH_HHD14_CalcXorSum(const char *code, int len) {
  int i;
  int sum=0;

  for (i=0; i<len; i++) {
    int rv;
    unsigned int v;

    rv=AH_HHD14_ReadBytesHex(code, 1);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    v=(unsigned int) rv;
    sum^=v;
    code++;
  }

  return (unsigned int) sum;
}



void AH_HHD14_CompressCode(const uint8_t *code, GWEN_BUFFER *cbuf) {
  const uint8_t *p;

  p=code;
  while(*p) {
    uint8_t c;

    c=toupper(*p);
    if ( (c>='0' && c<='9') || (c>='A' && c<='Z') || c==',')
      GWEN_Buffer_AppendByte(cbuf, c);
    p++;
  }
}



void AH_HHD14_ExtractCode(GWEN_BUFFER *cbuf) {
  const char *pStart=NULL;
  const char *pEnd=NULL;

  pStart=GWEN_Text_StrCaseStr(GWEN_Buffer_GetStart(cbuf), "CHLGUC");
  if (pStart) {
    pStart+=10; /* skip "CHLGUC" and following 4 digits */
    pEnd=GWEN_Text_StrCaseStr(pStart, "CHLGTEXT");
  }
  if (pStart && pEnd) {
    GWEN_Buffer_Crop(cbuf, pStart-GWEN_Buffer_GetStart(cbuf), pEnd-pStart);
    GWEN_Buffer_SetPos(cbuf, 0);
    GWEN_Buffer_InsertByte(cbuf, '0');
    GWEN_Buffer_SetPos(cbuf, GWEN_Buffer_GetUsedBytes(cbuf));
  }
}



int AH_HHD14__Translate(const char *code, GWEN_BUFFER *cbuf) {
  /*unsigned int totalLength;*/ /*TODO: handle total length */
  unsigned int inLenAndFlags;
  unsigned int inLen;
  unsigned int outLenAndFlags;
  unsigned int outLen;
  int rv;
  GWEN_BUFFER *xbuf;
  char numbuf[16];
  uint32_t cBufStartPos;
  uint32_t cBufEndPos;
  unsigned int checkSum;

  xbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* read length */
  rv=AH_HHD14_ReadBytesDec(code, 3);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  /*totalLength=(unsigned int) rv;*/
  code+=3;

  /* translate startCode (length is in hex) */
  rv=AH_HHD14_ReadBytesHex(code, 2);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  inLenAndFlags=(unsigned int) rv;
  inLen=inLenAndFlags & 0x3f;
  code+=2;

  outLen=(inLen+1)/2;
  snprintf(numbuf, sizeof(numbuf)-1, "%02x", outLen);
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(xbuf, numbuf);

  /* copy control bytes, if necessary */
  if (inLenAndFlags & 0x80) {
    unsigned int ctrl=0;

    do {
      /* control byte(s) follow (HHD1.4) */
      rv=AH_HHD14_ReadBytesHex(code, 2);
      if (rv<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        GWEN_Buffer_free(xbuf);
        return rv;
      }
      ctrl=(unsigned int) rv;
      /* write to output buffer */
      snprintf(numbuf, sizeof(numbuf)-1, "%02x", ctrl);
      numbuf[sizeof(numbuf)-1]=0;
      GWEN_Buffer_AppendString(xbuf, numbuf);
      code+=2;
    } while (ctrl & 0x80);
  }

  if (inLen) {
    GWEN_Buffer_AppendBytes(xbuf, code, inLen);
    if (inLen % 2)
      /* fill with "F" if necessary */
      GWEN_Buffer_AppendByte(xbuf, 'F');
  }
  code+=inLen;

  /* read DE's */
  while (*code) {
    /* length is in dec and contains flags */
    rv=AH_HHD14_ReadBytesDec(code, 2);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(xbuf);
      return rv;
    }
    inLenAndFlags=(unsigned int) rv;
    inLen=inLenAndFlags & 0x3f;
    code+=2;

    /* check whether we need to switch to ASC */
    if ((inLenAndFlags & 0x40)==0) {
      int i;

      for (i=0; i<inLen; i++) {
        if (code[i]<'0' || code[i]>'9'){
          /* contains something other than digits, use ascii encoding */
          inLenAndFlags|=0x40;
          break;
        }
      }
    }

    /* write to outbuffer */
    if (inLenAndFlags & 0x40) {
      /* ascii */
      //outLen=(inLen+1)/2;
      outLen=inLen;
      outLenAndFlags=outLen | 0x10; /* add encoding flag to length */
      snprintf(numbuf, sizeof(numbuf)-1, "%02x", outLenAndFlags);
      numbuf[sizeof(numbuf)-1]=0;
      GWEN_Buffer_AppendString(xbuf, numbuf);
      if (inLen)
        /* hex encode data */
        GWEN_Text_ToHexBuffer(code, inLen, xbuf, 0, 0, 0);
      code+=inLen;
    }
    else {
      /* bcd */
      outLen=(inLen+1)/2;
      outLenAndFlags=outLen;
      snprintf(numbuf, sizeof(numbuf)-1, "%02x", outLenAndFlags);
      numbuf[sizeof(numbuf)-1]=0;
      GWEN_Buffer_AppendString(xbuf, numbuf);
      if (inLen) {
        /* data is bcd, just copy */
        GWEN_Buffer_AppendBytes(xbuf, code, inLen);
        if (inLen % 2)
          /* fill with "F" if necessary */
          GWEN_Buffer_AppendByte(xbuf, 'F');
      }
      code+=inLen;
    }
  } /* while */

  /* cbuf starts here */
  cBufStartPos=GWEN_Buffer_GetPos(cbuf);

  /* calculate full length (payload plus checksums) */
  outLen=(GWEN_Buffer_GetUsedBytes(xbuf)+2+1)/2;
  snprintf(numbuf, sizeof(numbuf)-1, "%02x", outLen);
  numbuf[sizeof(numbuf)-1]=0;
  /* add length to outbuffer */
  GWEN_Buffer_AppendString(cbuf, numbuf);

  /* add translated buffer to output buffer */
  GWEN_Buffer_AppendBuffer(cbuf, xbuf);

  /* cbuf ends here */
  cBufEndPos=GWEN_Buffer_GetPos(cbuf);

  /* get payload for luhn sum */
  GWEN_Buffer_Reset(xbuf);
  rv=AH_HHD14_ExtractDataForLuhnSum(GWEN_Buffer_GetStart(cbuf)+cBufStartPos, xbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return rv;
  }

  /* calculate luhn sum */
  rv=AH_HHD14_CalcLuhnSum(GWEN_Buffer_GetStart(xbuf), GWEN_Buffer_GetUsedBytes(xbuf));
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  checkSum=(unsigned int) rv;

  /* add luhn sum */
  if (checkSum>9)
    checkSum+=7;
  checkSum+='0';
  GWEN_Buffer_AppendByte(cbuf, checkSum);

  /* calculate XOR sum */
  rv=AH_HHD14_CalcXorSum(GWEN_Buffer_GetStart(cbuf)+cBufStartPos,
                         cBufEndPos-cBufStartPos);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  checkSum=(unsigned int) rv;

  /* add XOR sum */
  if (checkSum>9)
    checkSum+=7;
  checkSum+='0';
  GWEN_Buffer_AppendByte(cbuf, checkSum);

  /* finish */
  GWEN_Buffer_free(xbuf);
  return 0;
}



int AH_HHD14_Translate(const char *code, GWEN_BUFFER *cbuf) {
  GWEN_BUFFER *xbuf;
  int rv;

  xbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AH_HHD14_CompressCode((const uint8_t*) code, xbuf);

  AH_HHD14_ExtractCode(xbuf);

  DBG_ERROR(AQHBCI_LOGDOMAIN, "HHD: Raw data is [%s]", GWEN_Buffer_GetStart(xbuf));

  rv=AH_HHD14__Translate(GWEN_Buffer_GetStart(xbuf), cbuf);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error translating HHD code (%d)", rv);
    GWEN_Text_LogString(code, strlen(code), AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  GWEN_Buffer_free(xbuf);
  return 0;
}








int AH_HHD14_AddChallengeParams_04(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteBankCode,
                                   const char *sRemoteAccountNumber) {
  /* P1: Betrag */
  if (vAmount) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    AH_Job_ValueToChallengeString(vAmount, tbuf);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  /* P2: BLZ Zahler/Empfaenger */
  if (sRemoteBankCode && *sRemoteBankCode)
    AH_Job_AddChallengeParam(j, sRemoteBankCode);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No remote bank code number");
    return GWEN_ERROR_INVALID;
  }

  /* P3: Konto Zahler/Empfaenger */
  if (sRemoteAccountNumber && *sRemoteAccountNumber) {
    int i;
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    i=10-strlen(sRemoteAccountNumber);
    if (i>0)
      GWEN_Buffer_FillWithBytes(tbuf, '0', i);
    GWEN_Buffer_AppendString(tbuf, sRemoteAccountNumber);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No remote account number");
    return GWEN_ERROR_INVALID;
  }

  /* done */
  return 0;
}



int AH_HHD14_AddChallengeParams_05(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteAccountNumber) {
  GWEN_BUFFER *tbuf;

  /* P1: Betrag */
  if (vAmount) {
    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    AB_Value_toHumanReadableString2(vAmount, tbuf, 2, 0); /* TODO: currency needed?? -> apparently not */
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No amount");
    return GWEN_ERROR_INVALID;
  }

  /* P2: Konto Empfaenger */
  if (sRemoteAccountNumber && *sRemoteAccountNumber) {
    int i;
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    i=10-strlen(sRemoteAccountNumber);
    if (i>0)
      GWEN_Buffer_FillWithBytes(tbuf, '0', i);
    GWEN_Buffer_AppendString(tbuf, sRemoteAccountNumber);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No remote account number");
    return GWEN_ERROR_INVALID;
  }

  /* done */
  return 0;
}



int AH_HHD14_AddChallengeParams_09(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteIban) {
  /* P1: Betrag */
  if (vAmount) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    AH_Job_ValueToChallengeString(vAmount, tbuf);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  /* P2: IBAN Empfaenger */
  if (sRemoteIban && *sRemoteIban)
    AH_Job_AddChallengeParam(j, sRemoteIban);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No remote iban");
    return GWEN_ERROR_INVALID;
  }

  /* done */
  return 0;
}



int AH_HHD14_AddChallengeParams_12(AH_JOB *j, int numTransfers, const AB_VALUE *vSumOfAmount,
                                   const char *sLocalAccount, const AB_VALUE *vSumOfRemoteAccounts) {
  char numbuf[16];

  /* P1: number of transfers */
  snprintf(numbuf, sizeof(numbuf)-1, "%d", numTransfers);
  numbuf[sizeof(numbuf)-1]=0;
  AH_Job_AddChallengeParam(j, numbuf);

  /* P2: Betrag */
  if (vSumOfAmount) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    AB_Value_toHumanReadableString2(vSumOfAmount, tbuf, 2, 0);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No amount");
    return GWEN_ERROR_INVALID;
  }

  /* P3: Konto Zahler */
  if (sLocalAccount && *sLocalAccount) {
    int i;
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    i=10-strlen(sLocalAccount);
    if (i>0)
      GWEN_Buffer_FillWithBytes(tbuf, '0', i);
    GWEN_Buffer_AppendString(tbuf, sLocalAccount);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No local account");
    return GWEN_ERROR_INVALID;
  }

  /* P4: Referenzzahl */
  if (vSumOfRemoteAccounts) {
    GWEN_BUFFER *tbuf;
    char *p;

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    AB_Value_toHumanReadableString2(vSumOfRemoteAccounts, tbuf, 0, 0);
    /* remove decimal point */
    p=strchr(GWEN_Buffer_GetStart(tbuf), '.');
    if (p)
      *p=0;
    /* only use first 10 digits */
    GWEN_Buffer_Crop(tbuf, 0, 10);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  /* done */
  return 0;
}



int AH_HHD14_AddChallengeParams_13(AH_JOB *j, int numTransfers, const AB_VALUE *vSumOfAmount, const char *sLocalIban) {
  /* same as 12, but uses IBAN */
  return AH_HHD14_AddChallengeParams_12(j, numTransfers, vSumOfAmount, sLocalIban, NULL);
}



int AH_HHD14_AddChallengeParams_17(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteIban) {
  return AH_HHD14_AddChallengeParams_09(j, vAmount, sRemoteIban);
}



int AH_HHD14_AddChallengeParams_19(AH_JOB *j, int numTransfers, const AB_VALUE *vSumOfAmount,
                                   const char *sLocalAccountNumber, const AB_VALUE *vSumOfRemoteAccounts) {
  /* same as 12 */
  return AH_HHD14_AddChallengeParams_12(j, numTransfers, vSumOfAmount, sLocalAccountNumber, vSumOfRemoteAccounts);
}


int AH_HHD14_AddChallengeParams_20(AH_JOB *j, int numTransfers, const AB_VALUE *vSumOfAmount, const char *sLocalIban) {
  /* same as 12 */
  return AH_HHD14_AddChallengeParams_12(j, numTransfers, vSumOfAmount, sLocalIban, NULL);
}



int AH_HHD14_AddChallengeParams_23(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteIban,
                                   const GWEN_TIME *ti) {
  /* P1: Betrag */
  if (vAmount) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    AH_Job_ValueToChallengeString(vAmount, tbuf);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }

  /* P2: IBAN Empfaenger */
  if (sRemoteIban && *sRemoteIban)
    AH_Job_AddChallengeParam(j, sRemoteIban);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No remote iban");
    return GWEN_ERROR_INVALID;
  }

  /* P3: Termin */
  if (ti) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    GWEN_Time_toString(ti, "YYYYMMDD", tbuf);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No execution date");
    return GWEN_ERROR_INVALID;
  }

  /* done */
  return 0;
}



int AH_HHD14_AddChallengeParams_29(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteIban,
                                   const GWEN_TIME *ti) {
  /* same as 23 */
  return AH_HHD14_AddChallengeParams_23(j, vAmount, sRemoteIban, ti);
}



int AH_HHD14_AddChallengeParams_32(AH_JOB *j,
                                   int transferCount,
                                   const AB_VALUE *vAmount,
                                   const char *sLocalIban,
                                   const GWEN_TIME *ti) {
  char numBuf[32];

  /* P1: Anzahl */
  snprintf(numBuf, sizeof(numBuf)-1, "%d", transferCount);
  numBuf[sizeof(numBuf)-1]=0;
  AH_Job_AddChallengeParam(j, numBuf);

  /* P2: Summe */
  if (vAmount) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    AH_Job_ValueToChallengeString(vAmount, tbuf);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing amount");
    return GWEN_ERROR_INVALID;
  }

  /* P3: Eigene IBAN */
  if (sLocalIban && *sLocalIban)
    AH_Job_AddChallengeParam(j, sLocalIban);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No local iban");
    return GWEN_ERROR_INVALID;
  }

  /* P4: Referenzzahl */
  AH_Job_AddChallengeParam(j, "");

  /* P5: Termin */
  if (ti) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 64, 0, 1);
    GWEN_Time_toString(ti, "YYYYMMDD", tbuf);
    AH_Job_AddChallengeParam(j, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No execution date");
    return GWEN_ERROR_INVALID;
  }

  /* done */
  return 0;
}



int AH_HHD14_AddChallengeParams_35(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteIban) {
  /* same as 09 */
  return AH_HHD14_AddChallengeParams_09(j, vAmount, sRemoteIban);
}








