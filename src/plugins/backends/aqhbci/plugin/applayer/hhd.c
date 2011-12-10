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

#include "hhd_p.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>

#include <ctype.h>




int AH_HHD14_ReadBytesDec(const char *p, int len) {
  int r=0;
  int i;

  for (i=0; i<len; i++) {
    uint8_t c;

    c=*p;
    if (c==0)
      break;
    if (c<'0' || c>'9') {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad char in data (no decimal digit)");
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
  /*unsigned int totalLength;*/
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
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  GWEN_Buffer_free(xbuf);
  return 0;
}







