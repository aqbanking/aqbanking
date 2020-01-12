/***************************************************************************
    begin       : Sun Jun 02 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "tan_chiptan_opt.h"


#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/gui.h>

#include <ctype.h> /* for toupper() */


#include <gwenhywfar/ct.h>       /* for USB_TAN */
#include <gwenhywfar/ctplugin.h> /* for USB_TAN */


/* forward declarations */

static int _readBytesDec(const char *p, int len);
static int _readBytesHex(const char *p, int len);
static unsigned int _quersumme(unsigned int i);
static int _extractDataForLuhnSum(const char *code, GWEN_BUFFER *xbuf);
static int _calcLuhnSum(const char *code, int len);
static int _calcXorSum(const char *code, int len);
static int __translate(const char *code, GWEN_BUFFER *cbuf);
static int _translate(const char *code, GWEN_BUFFER *cbuf);
static int __translateWithLen(const char *code, GWEN_BUFFER *cbuf, int sizeLen);
static int _getTan(AH_TAN_MECHANISM *tanMechanism,
                   AB_USER *u,
                   const char *title,
                   const char *text,
                   const uint8_t *challengePtr,
                   uint32_t challengeLen,
                   char *passwordBuffer,
                   int passwordMinLen,
                   int passwordMaxLen);

static int _getTanUSB(AH_TAN_MECHANISM *tanMechanism,
                      AB_USER *u,
                      const char *title,
                      const char *text,
                      const uint8_t *challengePtr,
                      uint32_t challengeLen,
                      char *passwordBuffer,
                      int passwordMinLen,
                      int passwordMaxLen);

typedef int(*GetTanfromUSB_GeneratorFn)(unsigned char* HHDCommand, int fullHHD_Len, int* pATC, char* pGeneratedTAN, uint32_t maxTanLen, char* pCardnummber, char* pEndDate, char* IssueDate);



/* implementation */



AH_TAN_MECHANISM *AH_TanMechanism_ChipTanUSB_new(const AH_TAN_METHOD *tanMethod, int tanMethodId)
{
	AH_TAN_MECHANISM *tanMechanism;

	tanMechanism = AH_TanMechanism_new(tanMethod, tanMethodId);
	assert(tanMechanism);

	AH_TanMechanism_SetGetTanFn(tanMechanism, _getTanUSB);
	return tanMechanism;
}



AH_TAN_MECHANISM *AH_TanMechanism_ChipTanOpt_new(const AH_TAN_METHOD *tanMethod, int tanMethodId)
{
  AH_TAN_MECHANISM *tanMechanism;

  tanMechanism=AH_TanMechanism_new(tanMethod, tanMethodId);
  assert(tanMechanism);

  AH_TanMechanism_SetGetTanFn(tanMechanism, _getTan);
  return tanMechanism;
}





int _getTanUSB(AH_TAN_MECHANISM *tanMechanism,
               AB_USER *u,
               const char *title,
               const char *text,
               const uint8_t *challengePtr,
               uint32_t challengeLen,
               char *passwordBuffer,
               int passwordMinLen,
               int passwordMaxLen)
{
  int rv;
  const AH_TAN_METHOD *tanMethod;
  GWEN_BUFFER *cbuf;

  tanMethod=AH_TanMechanism_GetTanMethod(tanMechanism);
  assert(tanMethod);

  /* translate challenge string to flicker code */
  cbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=_translate((const char *) challengePtr, cbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(cbuf);
    return rv;
  }
  else {
    GWEN_DB_NODE *dbMethodParams;
    GWEN_BUFFER *bufToken;
    GWEN_DB_NODE *dbTanMethod;
    /* USB_TAN specific start */
    static char cardPrefix[] = { 0,0,0,0,1,0,0 };
    unsigned char HHDCommand[120];
    int HHD_Generator_Len;
    unsigned char* pHHDDest;
    char* pHHDSrc;
    int fullHHD_Len;
    int i;

    GWEN_PLUGIN_MANAGER *pm;
    GWEN_PLUGIN *pl;
    GWEN_LIBLOADER* ll;
    void *p;

    int ATC;
    char Cardnummber[11];
    char EndDate[5];
    char IssueDate[7];
    /* USB_TAN specific end */

#if 0
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Will use this challenge:");
    GWEN_Buffer_Dump(cbuf, 2);
#endif

    dbMethodParams=GWEN_DB_Group_new("methodParams");

    GWEN_DB_SetIntValue(dbMethodParams, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "tanMethodId", AH_TanMechanism_GetTanMethodId(tanMechanism));

    GWEN_DB_SetCharValue(dbMethodParams, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "challenge", GWEN_Buffer_GetStart(cbuf));

    dbTanMethod=GWEN_DB_GetGroup(dbMethodParams, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "tanMethod");
    AH_TanMethod_toDb(tanMethod, dbTanMethod);

    bufToken=GWEN_Buffer_new(0, 256, 0, 1);
    AH_User_MkTanName(u, (const char *) challengePtr, bufToken);

    rv = GWEN_Gui_ShowBox(0, title, text, 0);

    /* TODO: hmm, this should probably be changed to a more typesafe way...
     * Maybe we need to add a virtual function for CryptTokens...
     */
    pm = GWEN_PluginManager_FindPluginManager("ct");
    if (pm == 0) {
      DBG_ERROR(0, "Plugin manager not found");
      return GWEN_ERROR_NOT_FOUND;
    }

    pl = GWEN_PluginManager_GetPlugin(pm, "chiptanusb");
    if (pl == 0) {
      DBG_ERROR(0, "Plugin not found");
      return GWEN_ERROR_NOT_FOUND;
    }
    DBG_INFO(0, "Plugin found");

    ll = GWEN_Plugin_GetLibLoader(pl);

    rv = GWEN_LibLoader_Resolve(ll, "GetTanfromUSB_Generator", &p);
    if (rv<0) {
      return rv;
    }

    /* Generate the binary input for the card */
    pHHDDest = &HHDCommand[8];
    HHD_Generator_Len = GWEN_Buffer_GetUsedBytes(cbuf) / 2;
    fullHHD_Len = HHD_Generator_Len + sizeof(cardPrefix) + 1;
    pHHDSrc = GWEN_Buffer_GetStart(cbuf);
    memcpy(HHDCommand, cardPrefix, sizeof(cardPrefix));
    HHDCommand[sizeof(cardPrefix)] = HHD_Generator_Len;

    for (i = 0; i < HHD_Generator_Len; i++) {
      *pHHDDest++ = (unsigned char)_readBytesHex(pHHDSrc, 2);
      pHHDSrc++;
      pHHDSrc++;
    }

    rv = ((GetTanfromUSB_GeneratorFn)p)(HHDCommand, fullHHD_Len, &ATC,
					passwordBuffer, passwordMaxLen,
					&Cardnummber[0], &EndDate[0], &IssueDate[0]);
    GWEN_Gui_HideBox(0);

    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(bufToken);
      GWEN_Buffer_free(cbuf);
      GWEN_DB_Group_free(dbMethodParams);
      return rv;
    }

    GWEN_Buffer_free(bufToken);
    GWEN_Buffer_free(cbuf);
    GWEN_DB_Group_free(dbMethodParams);
    return 0;
  }
}




int _getTan(AH_TAN_MECHANISM *tanMechanism,
            AB_USER *u,
            const char *title,
            const char *text,
            const uint8_t *challengePtr,
            uint32_t challengeLen,
            char *passwordBuffer,
            int passwordMinLen,
            int passwordMaxLen)
{
  int rv;
  const AH_TAN_METHOD *tanMethod;
  GWEN_BUFFER *cbuf;

  tanMethod=AH_TanMechanism_GetTanMethod(tanMechanism);
  assert(tanMethod);

  /* translate challenge string to flicker code */
  cbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=_translate((const char *) challengePtr, cbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(cbuf);
    return rv;
  }
  else {
    GWEN_DB_NODE *dbMethodParams;
    GWEN_BUFFER *bufToken;
    GWEN_DB_NODE *dbTanMethod;

    DBG_ERROR(AQHBCI_LOGDOMAIN, "Will use this challenge:");
    GWEN_Buffer_Dump(cbuf, 2);

    dbMethodParams=GWEN_DB_Group_new("methodParams");

    GWEN_DB_SetIntValue(dbMethodParams, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "tanMethodId", AH_TanMechanism_GetTanMethodId(tanMechanism));

    GWEN_DB_SetCharValue(dbMethodParams, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "challenge", GWEN_Buffer_GetStart(cbuf));

    dbTanMethod=GWEN_DB_GetGroup(dbMethodParams, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "tanMethod");
    AH_TanMethod_toDb(tanMethod, dbTanMethod);

    bufToken=GWEN_Buffer_new(0, 256, 0, 1);
    AH_User_MkTanName(u, (const char *) challengePtr, bufToken);

    rv=GWEN_Gui_GetPassword(GWEN_GUI_INPUT_FLAGS_TAN | GWEN_GUI_INPUT_FLAGS_SHOW | GWEN_GUI_INPUT_FLAGS_DIRECT,
                            GWEN_Buffer_GetStart(bufToken),
                            title,
                            text,
                            passwordBuffer,
                            passwordMinLen,
                            passwordMaxLen,
                            GWEN_Gui_PasswordMethod_OpticalHHD,
                            dbMethodParams,
                            0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(bufToken);
      GWEN_Buffer_free(cbuf);
      GWEN_DB_Group_free(dbMethodParams);
      return rv;
    }


    GWEN_Buffer_free(bufToken);
    GWEN_Buffer_free(cbuf);
    GWEN_DB_Group_free(dbMethodParams);
    return 0;
  }
}





int _translate(const char *code, GWEN_BUFFER *cbuf)
{
  int rv;

  DBG_ERROR(AQHBCI_LOGDOMAIN, "HHD: Raw data is [%s]", code);

  rv=__translate(code, cbuf);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error translating HHD code (%d)", rv);
    GWEN_Text_LogString(code, strlen(code), AQHBCI_LOGDOMAIN, GWEN_LoggerLevel_Error);
    return rv;
  }
  return 0;
}



int __translate(const char *code, GWEN_BUFFER *cbuf)
{
  GWEN_BUFFER *tmpBuf;
  int rv;

  tmpBuf=GWEN_Buffer_new(0, 256, 0, 1);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Trying 3 bytes length");
  rv=__translateWithLen(code, tmpBuf, 3);
  if (rv<0) {
    GWEN_Buffer_Reset(tmpBuf);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Trying 2 bytes length");
    rv=__translateWithLen(code, tmpBuf, 2);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Invalid challenge data (%d)", rv);
      GWEN_Buffer_free(tmpBuf);
      return rv;
    }
  }

  GWEN_Buffer_AppendBuffer(cbuf, tmpBuf);
  GWEN_Buffer_free(tmpBuf);
  return rv;
}



int __translateWithLen(const char *code, GWEN_BUFFER *cbuf, int sizeLen)
{
  unsigned int totalLength; /*handle total length */
  unsigned int inLenAndFlags;
  unsigned int inLen;
  unsigned int outLenAndFlags;
  unsigned int outLen;
  unsigned int codeLen;
  /* preset bit masks for HHD 1.4 */
  unsigned int maskLen = 0x3f;
  unsigned int maskAscFlag = 0x40;
  unsigned int maskCtlFlag = 0x80;

  int rv;
  GWEN_BUFFER *xbuf;
  char numbuf[16];
  uint32_t cBufStartPos;
  uint32_t cBufEndPos;
  unsigned int checkSum;

  assert(code);
  codeLen=strlen(code);

  xbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* read length */
  rv=_readBytesDec(code, sizeLen);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  totalLength=(unsigned int) rv;
  if ((totalLength+sizeLen)>codeLen) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Total length exceeds length of given code (%d+%d > %d)", totalLength, sizeLen, codeLen);
    GWEN_Buffer_free(xbuf);
    return GWEN_ERROR_BAD_DATA;
  }
  code+=sizeLen;

  /* translate startCode (length is in hex) */
  rv=_readBytesHex(code, 2);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  inLenAndFlags=(unsigned int) rv;
  inLen=inLenAndFlags & maskLen;
  code+=2;

  outLen=(inLen+1)/2;
  outLenAndFlags=outLen | (inLenAndFlags & maskCtlFlag);
  snprintf(numbuf, sizeof(numbuf)-1, "%02x", outLenAndFlags);
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(xbuf, numbuf);

  /* copy control bytes, if necessary */
  if (inLenAndFlags & maskCtlFlag) {
    unsigned int ctrl=0;

    do {
      /* control byte(s) follow (HHD1.4) */
      rv=_readBytesHex(code, 2);
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
    }
    while (ctrl & maskCtlFlag);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "no control bytes fallback to HHD 1.3.2");
    maskLen = 0xf;
    maskAscFlag = 0x10;
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
    /* input length is in dec usually no AscFlag for DE's is provided */
    rv=_readBytesDec(code, 2);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(xbuf);
      return rv;
    }
    inLenAndFlags=(unsigned int) rv;
    inLen=inLenAndFlags & maskLen;
    code+=2;

    /* so we have to check whether we need to switch to ASC */
    if ((inLenAndFlags & maskAscFlag)==0) {
      int i;

      for (i=0; i<inLen; i++) {
        if (code[i]<'0' || code[i]>'9') {
          /* contains something other than digits, use ascii encoding */
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Switched to ASCII");
          inLenAndFlags|=maskAscFlag;
          break;
        }
      }
    }

    /* write to outbuffer */
    if (inLenAndFlags & maskAscFlag) {
      /* ascii */
      outLen=inLen;
      outLenAndFlags=outLen | maskAscFlag; /* add encoding flag to length (bit 6 or 4) */
      snprintf(numbuf, sizeof(numbuf)-1, "%02x", outLenAndFlags);
      numbuf[sizeof(numbuf)-1]=0;
      GWEN_Buffer_AppendString(xbuf, numbuf);
      if (inLen)
        /* hex encode data */
        GWEN_Text_ToHexBuffer(code, inLen, xbuf, 0, 0, 0);
      code+=inLen;
    }
    else {
      /* bcd, pack 2 digits into 1 Byte */
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
  rv=_extractDataForLuhnSum(GWEN_Buffer_GetStart(cbuf)+cBufStartPos, xbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return rv;
  }

  /* calculate luhn sum */
  rv=_calcLuhnSum(GWEN_Buffer_GetStart(xbuf), GWEN_Buffer_GetUsedBytes(xbuf));
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
  rv=_calcXorSum(GWEN_Buffer_GetStart(cbuf)+cBufStartPos, cBufEndPos-cBufStartPos);
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





int _readBytesDec(const char *p, int len)
{
  int r=0;
  int i;
  const char *pSave;

  pSave=p;
  for (i=0; i<len; i++) {
    uint8_t c;

    c=*p;
    if (c==0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Premature end of string");
      return GWEN_ERROR_PARTIAL;
    }
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



int _readBytesHex(const char *p, int len)
{
  unsigned int r=0;
  int i;

  for (i=0; i<len; i++) {
    uint8_t c;

    c=*p;
    if (c==0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Premature end of string");
      return GWEN_ERROR_PARTIAL;
    }
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



unsigned int _quersumme(unsigned int i)
{
  unsigned int qs=0;

  while (i) {
    qs+=i % 10;
    i/=10;
  }
  return qs;
}



int _extractDataForLuhnSum(const char *code, GWEN_BUFFER *xbuf)
{
  int rv;
  unsigned int len;
  unsigned int i=0;
  unsigned int LSandFlags;
  unsigned int numCtrlBytes;
  unsigned int moreCtrlBytes;
  unsigned int numBytes;
  /* preset bit masks for HHD 1.4 */
  unsigned int maskLen = 0x3f;

  /* read LC */
  rv=_readBytesHex(code, 2);
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

  /* read LS */
  rv=_readBytesHex(code, 2);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d) at [%s]", rv, code);
    return rv;
  }
  code+=2;

  /* add control bytes and start code */
  LSandFlags = (unsigned int) rv;
  numCtrlBytes = 0;
  moreCtrlBytes = LSandFlags & 0x80;

  while (moreCtrlBytes) {
    rv=_readBytesHex(code+numCtrlBytes*2, 2);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d) at [%s]", rv, code);
      return rv;
    }
    numCtrlBytes++;
    moreCtrlBytes = (unsigned int) rv & 0x80;
  }
  if (numCtrlBytes == 0) {
    /* set bit mask for HHD 1.3.2 */
    maskLen = 0x0f;
  }
  numBytes = (LSandFlags & maskLen) + numCtrlBytes;
  GWEN_Buffer_AppendBytes(xbuf, code, numBytes*2);
  code += numBytes*2;
  i += numBytes + 2;         /* add length of LC and LS */

  /* read LDE1, DE1, LDE2, DE2, ... */
  while (i<len-1) {
    unsigned int v;

    rv=_readBytesHex(code, 2);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d) at [%s]", rv, code);
      return rv;
    }
    /*    v=((unsigned int) rv) & 0xf; */
    v=((unsigned int) rv) & maskLen;
    code+=2;
    if (i+v+1 > len) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "try to read past the end of code (%d) at [%s]", v, code);
      return GWEN_ERROR_INVALID;
    }
    GWEN_Buffer_AppendBytes(xbuf, code, v*2);
    code+=v*2;
    i+=v+1;
  }

  return 0;
}



int _calcLuhnSum(const char *code, int len)
{
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

    rv=_readBytesHex(code, 2);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d) at [%s]", rv, code);
      return rv;
    }
    v=(unsigned int) rv;
    sum+=(1*((v>>4) & 0xf)) + (_quersumme(2*(v & 0xf)));
    code+=2;
  }

  next=10*((sum+9)/10);

  dif=next-sum;
  return (unsigned int) dif;
}



int _calcXorSum(const char *code, int len)
{
  int i;
  int sum=0;

  for (i=0; i<len; i++) {
    int rv;
    unsigned int v;

    rv=_readBytesHex(code, 1);
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





