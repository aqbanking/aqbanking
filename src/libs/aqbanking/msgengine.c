/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "msgengine_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>


#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(GWEN_MSGENGINE, AB_MSGENGINE)



GWEN_MSGENGINE *AB_MsgEngine_new(void){
  GWEN_MSGENGINE *e;
  AB_MSGENGINE *le;

  e=GWEN_MsgEngine_new();
  GWEN_NEW_OBJECT(AB_MSGENGINE, le);
  GWEN_INHERIT_SETDATA(GWEN_MSGENGINE, AB_MSGENGINE,
                       e, le, AB_MsgEngine_FreeData);
  GWEN_MsgEngine_SetTypeReadFunction(e, AB_MsgEngine_TypeRead);
  GWEN_MsgEngine_SetTypeWriteFunction(e, AB_MsgEngine_TypeWrite);
  GWEN_MsgEngine_SetTypeCheckFunction(e, AB_MsgEngine_TypeCheck);
  GWEN_MsgEngine_SetBinTypeReadFunction(e, AB_MsgEngine_BinTypeRead);
  GWEN_MsgEngine_SetBinTypeWriteFunction(e, AB_MsgEngine_BinTypeWrite);
  GWEN_MsgEngine_SetGetCharValueFunction(e, AB_MsgEngine_GetCharValue);
  GWEN_MsgEngine_SetGetIntValueFunction(e, AB_MsgEngine_GetIntValue);
  GWEN_MsgEngine_SetEscapeChar(e, '?');
  GWEN_MsgEngine_SetDelimiters(e, "");

  return e;
}



void GWENHYWFAR_CB AB_MsgEngine_FreeData(void *bp, void *p){
  //GWEN_MSGENGINE *e;
  AB_MSGENGINE *le;

  //e=(GWEN_MSGENGINE*)bp;
  le=(AB_MSGENGINE*)p;

  /* free all objects inside AB_MsgEngine */

  GWEN_FREE_OBJECT(le);
}



uint32_t AB_MsgEngine__FromBCD(uint32_t value) {
  uint32_t rv;

  rv=0;
  rv+=((value>>28)&0xf)*10000000;
  rv+=((value>>24)&0xf)*1000000;
  rv+=((value>>20)&0xf)*100000;
  rv+=((value>>16)&0xf)*10000;
  rv+=((value>>12)&0xf)*1000;
  rv+=((value>>8)&0xf)*100;
  rv+=((value>>4)&0xf)*10;
  rv+=((value)&0xf);

  return rv;
}



uint32_t AB_MsgEngine__ToBCD(uint32_t value) {
  uint32_t rv;

  rv=0;
  rv+=value/10000000;
  value%=10000000;
  rv<<=4;

  rv+=value/1000000;
  value%=1000000;
  rv<<=4;

  rv+=value/100000;
  value%=100000;
  rv<<=4;

  rv+=value/10000;
  value%=10000;
  rv<<=4;

  rv+=value/1000;
  value%=1000;
  rv<<=4;

  rv+=value/100;
  value%=100;
  rv<<=4;

  rv+=value/10;
  value%=10;
  rv<<=4;

  rv+=value;

  return rv;
}





int AB_MsgEngine_TypeRead(GWEN_MSGENGINE *e,
                          GWEN_BUFFER *msgbuf,
                          GWEN_XMLNODE *node,
                          GWEN_BUFFER *vbuf,
                          char escapeChar,
                          const char *delimiters){
  AB_MSGENGINE *le;
  const char *type;

  assert(e);
  le=GWEN_INHERIT_GETDATA(GWEN_MSGENGINE, AB_MSGENGINE, e);
  assert(le);

  if (!GWEN_Buffer_GetBytesLeft(msgbuf)) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Buffer empty");
    return 0;
  }
  type=GWEN_XMLNode_GetProperty(node, "type","");
  if (strcasecmp(type, "byte")==0) {
    int isBCD;
    int c;
    char numbuf[32];
    unsigned int value;

    isBCD=atoi(GWEN_XMLNode_GetProperty(node, "bcd", "0"));
    c=GWEN_Buffer_ReadByte(msgbuf);
    if (c==-1)
      return -1;
    value=c&0xff;
    if (isBCD)
      value=AB_MsgEngine__FromBCD(value);
    snprintf(numbuf, sizeof(numbuf), "%d", (unsigned int)value);
    if (GWEN_Buffer_AppendString(vbuf, numbuf)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    return 0;
  }
  else if (strcasecmp(type, "word")==0) {
    int bigEndian;
    int isBCD;
    unsigned int value;
    int c;
    char numbuf[32];

    bigEndian=atoi(GWEN_XMLNode_GetProperty(node, "bigEndian", "1"));
    isBCD=atoi(GWEN_XMLNode_GetProperty(node, "bcd", "0"));
    value=0;
    if (bigEndian) {
      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      value=(((unsigned char)(c&0xff)<<8));
      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      value|=(unsigned char)((c&0xff));
    } /* if bigEndian */
    else {
      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      value=(unsigned char)((c&0xff));
      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      value|=(unsigned char)(((c&0xff)<<8));
    }
    if (isBCD)
      value=AB_MsgEngine__FromBCD(value);
    snprintf(numbuf, sizeof(numbuf), "%d", (unsigned int)value);
    if (GWEN_Buffer_AppendString(vbuf, numbuf)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    return 0;
  } /* if word */
  else if (strcasecmp(type, "dword")==0) {
    int bigEndian;
    int isBCD;
    uint32_t value;
    int c;
    char numbuf[32];

    bigEndian=atoi(GWEN_XMLNode_GetProperty(node, "bigEndian", "1"));
    isBCD=atoi(GWEN_XMLNode_GetProperty(node, "bcd", "0"));
    value=0;
    if (bigEndian) {
      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      value=(((unsigned char)(c&0xff)<<24));
      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      value|=(((unsigned char)(c&0xff)<<16));
      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      value|=(((unsigned char)(c&0xff)<<8));
      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      value|=(unsigned char)((c&0xff));
    } /* if bigEndian */
    else {
      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      value=(unsigned char)((c&0xff));
      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      value|=(unsigned char)(((c&0xff)<<8));
      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      value|=(unsigned char)(((c&0xff)<<16));
      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      value|=(unsigned char)(((c&0xff)<<24));
    }
    if (isBCD)
      value=AB_MsgEngine__FromBCD(value);
    snprintf(numbuf, sizeof(numbuf), "%d", (unsigned int)value);
    if (GWEN_Buffer_AppendString(vbuf, numbuf)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    return 0;
  } /* if word */
  else if (strcasecmp(type, "bytes")==0) {
    int size;

    if (1!=sscanf(GWEN_XMLNode_GetProperty(node, "size", "-1"),
                  "%i", &size)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad number of bytes");
      return -1;
    }
    if (size==-1) {
      size=GWEN_Buffer_GetBytesLeft(msgbuf);
      if (size==0) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "No bytes found");
        return 0;
      }
    }
    else {
      if (size>GWEN_Buffer_GetBytesLeft(msgbuf)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes in message (%d>%d)",
                  size, GWEN_Buffer_GetBytesLeft(msgbuf));
        return -1;
      }
    }
    if (GWEN_Buffer_AppendBytes(vbuf,
                                GWEN_Buffer_GetPosPointer(msgbuf),
                                size)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    if (GWEN_Buffer_IncrementPos(msgbuf, size)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    return 0;
  }
  else if (strcasecmp(type, "bcd")==0) {
    int size;
    int skipLeadingZeroes;

    skipLeadingZeroes=atoi(GWEN_XMLNode_GetProperty(node,
                                                    "skipZeroes", "0"));
    if (1!=sscanf(GWEN_XMLNode_GetProperty(node, "size", "-1"),
                  "%i", &size)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad number of bytes");
      return -1;
    }
    if (size==-1) {
      size=GWEN_Buffer_GetBytesLeft(msgbuf);
      if (size==0) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "No bytes found");
        return 0;
      }
    }
    else {
      if (size>GWEN_Buffer_GetBytesLeft(msgbuf)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes in message (%d>%d)",
                  size, GWEN_Buffer_GetBytesLeft(msgbuf));
        return -1;
      }
    }

    if (GWEN_Text_ToBcdBuffer(GWEN_Buffer_GetPosPointer(msgbuf),
			      size,
			      vbuf,
                              0, 0, skipLeadingZeroes)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error parsing BCD string");
      return -1;
    }

    if (GWEN_Buffer_IncrementPos(msgbuf, size)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    return 0;
  }

  else if (strcasecmp(type, "ascii")==0) {
    int size;
    int condense;
    int kvk;
    //uint32_t vpos=0;

    kvk=atoi(GWEN_XMLNode_GetProperty(node, "kvk", "0"));
    condense=atoi(GWEN_XMLNode_GetProperty(node, "condense", "1"));
    if (1!=sscanf(GWEN_XMLNode_GetProperty(node, "size", "-1"),
                  "%i", &size)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad number of bytes");
      return -1;
    }
    if (size==-1) {
      /* if no fixed size given let GWEN handle this */
      return 1;
    }
    else {
      if (size>GWEN_Buffer_GetBytesLeft(msgbuf)) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes in message (%d>%d)",
                  size, GWEN_Buffer_GetBytesLeft(msgbuf));
        return -1;
      }
    }

    if (kvk) {
//      vpos=GWEN_Buffer_GetPos(vbuf);
    }
    if (condense) {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, size, 0, 1);
      GWEN_Buffer_AppendBytes(tbuf, GWEN_Buffer_GetPosPointer(msgbuf), size);
      GWEN_Text_CondenseBuffer(tbuf);
      GWEN_Buffer_Rewind(tbuf);
      if (GWEN_Buffer_GetUsedBytes(tbuf)==0) {
	/* just to fool the caller */
        GWEN_Buffer_AppendByte(tbuf, 0);
      }
      if (GWEN_Buffer_AppendBuffer(vbuf, tbuf)) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "here");
        GWEN_Buffer_free(tbuf);
	return -1;
      }
      GWEN_Buffer_free(tbuf);
    }
    else {
      if (GWEN_Buffer_AppendBytes(vbuf,
				  GWEN_Buffer_GetPosPointer(msgbuf),
				  size)) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "here");
	return -1;
      }
    }

    if (GWEN_Buffer_IncrementPos(msgbuf, size)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    return 0;
  }

  else if (strcasecmp(type, "tlv")==0) {
    int isBerTlv;
    const char *p;
    unsigned int size;
    unsigned int pos;
    unsigned int j;

    p=GWEN_Buffer_GetPosPointer(msgbuf);
    pos=0;
    size=GWEN_Buffer_GetBytesLeft(msgbuf);
    isBerTlv=(strcasecmp(GWEN_XMLNode_GetProperty(node,
                                                  "tlvtype",
                                                  "bertlv"),
                         "BER")==0);
    /* get tag type */
    if (size<2) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes for BER-TLV");
      return -1;
    }
    j=(unsigned char)(p[pos]);
    if (isBerTlv) {
      if ((j & 0x1f)==0x1f) {
        pos++;
        if (pos>=size) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes");
          return -1;
        }
        j=(unsigned char)(p[pos]);
      }
      else
        j&=0x1f;
    }
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Tag type %02x%s", j,
	      isBerTlv?" (BER-TLV)":"");

    /* get length */
    pos++;
    if (pos>=size) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes");
      return -1;
    }
    j=(unsigned char)(p[pos]);
    if (isBerTlv) {
      if (j & 0x80) {
        if (j==0x81) {
          pos++;
          if (pos>=size) {
            DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes");
            return -1;
          }
          j=(unsigned char)(p[pos]);
        } /* 0x81 */
        else if (j==0x82) {
          if (pos+1>=size) {
            DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes");
            return -1;
          }
          pos++;
          j=((unsigned char)(p[pos]))<<8;
          pos++;
          j+=(unsigned char)(p[pos]);
        } /* 0x82 */
        else {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Unexpected tag length modifier %02x", j);
          return -1;
        }
      } /* if tag length modifier */
    }
    else {
      if (j==255) {
        if (pos+2>=size) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes");
          return -1;
        }
        pos++;
        j=((unsigned char)(p[pos]))<<8;
        pos++;
        j+=(unsigned char)(p[pos]);
      }
    }
    pos++;
    pos+=j;

    if (pos>size) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes");
      return -1;
    }
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Tag data length is %d (total %d)", j, pos);
    if (GWEN_Buffer_AppendBytes(vbuf,
                                GWEN_Buffer_GetPosPointer(msgbuf),
                                pos)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    if (GWEN_Buffer_IncrementPos(msgbuf, pos)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }

    return 0;
  }

  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Type \"%s\" not supported by AB_MsgEngine", type);
    return 1;
  }
}



int AB_MsgEngine_TypeWrite(GWEN_MSGENGINE *e,
                           GWEN_BUFFER *gbuf,
                           GWEN_BUFFER *data,
                           GWEN_XMLNODE *node){
  AB_MSGENGINE *le;
  const char *type;

  assert(e);
  le=GWEN_INHERIT_GETDATA(GWEN_MSGENGINE, AB_MSGENGINE, e);
  assert(le);

  type=GWEN_XMLNode_GetProperty(node, "type","");
  if (strcasecmp(type, "byte")==0) {
    int value;
    int isBCD;

    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Supporting type \"byte\"");
    isBCD=atoi(GWEN_XMLNode_GetProperty(node, "bcd", "0"));
    if (1!=sscanf(GWEN_Buffer_GetPosPointer(data), "%i", &value)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad number");
      return -1;
    }

    if (value>255 || value<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Number out of range (%d)", value);
      return -1;
    }

    if (isBCD)
      value=AB_MsgEngine__ToBCD(value);
    if (GWEN_Buffer_AppendByte(gbuf, (unsigned char)value)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
      return -1;
    }
    return 0;
  } /* byte */
  else if (strcasecmp(type, "word")==0) {
    int bigEndian;
    int value;
    int isBCD;

    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Supporting type \"word\"");
    isBCD=atoi(GWEN_XMLNode_GetProperty(node, "bcd", "0"));
    if (1!=sscanf(GWEN_Buffer_GetPosPointer(data), "%i", &value)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad number");
      return -1;
    }

    if (value>0xffff || value<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Number out of range (%d)", value);
      return -1;
    }

    if (isBCD)
      value=AB_MsgEngine__ToBCD(value);
    bigEndian=atoi(GWEN_XMLNode_GetProperty(node, "bigEndian", "1"));

    if (bigEndian) {
      if (GWEN_Buffer_AppendByte(gbuf,
                                 (unsigned char)((value>>8)&0xff))){
        DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
        return -1;
      }
      if (GWEN_Buffer_AppendByte(gbuf,
                                 (unsigned char)(value&0xff))){
        DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
        return -1;
      }
    }
    else {
      if (GWEN_Buffer_AppendByte(gbuf,
                                 (unsigned char)(value&0xff))){
        DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
        return -1;
      }
      if (GWEN_Buffer_AppendByte(gbuf,
                                 (unsigned char)((value>>8)&0xff))){
        DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
        return -1;
      }
    }
    return 0;
  } /* word */
  else if (strcasecmp(type, "dword")==0) {
    int bigEndian;
    int isBCD;
    uint32_t value;

    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Supporting type \"dword\"");
    isBCD=atoi(GWEN_XMLNode_GetProperty(node, "bcd", "0"));
    if (1!=sscanf(GWEN_Buffer_GetPosPointer(data), "%i", &value)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad number");
      return -1;
    }
    if (isBCD)
      value=AB_MsgEngine__ToBCD(value);

    bigEndian=atoi(GWEN_XMLNode_GetProperty(node, "bigEndian", "1"));

    if (bigEndian) {
      if (GWEN_Buffer_AppendByte(gbuf,
                                 (unsigned char)((value>>24)&0xff))){
        DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
        return -1;
      }
      if (GWEN_Buffer_AppendByte(gbuf,
                                 (unsigned char)((value>>16)&0xff))){
        DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
        return -1;
      }
      if (GWEN_Buffer_AppendByte(gbuf,
                                 (unsigned char)((value>>8)&0xff))){
        DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
        return -1;
      }
      if (GWEN_Buffer_AppendByte(gbuf,
                                 (unsigned char)(value&0xff))){
        DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
        return -1;
      }
    }
    else {
      if (GWEN_Buffer_AppendByte(gbuf,
                                 (unsigned char)(value&0xff))){
        DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
        return -1;
      }
      if (GWEN_Buffer_AppendByte(gbuf,
                                 (unsigned char)((value>>8)&0xff))){
        DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
        return -1;
      }
      if (GWEN_Buffer_AppendByte(gbuf,
                                 (unsigned char)((value>>16)&0xff))){
        DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
        return -1;
      }
      if (GWEN_Buffer_AppendByte(gbuf,
                                 (unsigned char)((value>>24)&0xff))){
        DBG_INFO(AQBANKING_LOGDOMAIN, "called from here");
        return -1;
      }
    }
    return 0;
  } /* word */

  else if (strcasecmp(type, "bytes")==0) {
    if (GWEN_Buffer_GetUsedBytes(data)) {
      if (GWEN_Buffer_AppendBytes(gbuf,
                                  GWEN_Buffer_GetStart(data),
                                  GWEN_Buffer_GetUsedBytes(data))) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "here");
        return -1;
      }
    }
    return 0;
  }

  else if (strcasecmp(type, "bcd")==0) {
    if (GWEN_Text_FromBcdBuffer(GWEN_Buffer_GetStart(data), gbuf)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    return 0;
  }

  else if (strcasecmp(type, "tlv")==0) {
    int size;

    size=GWEN_Buffer_GetUsedBytes(data);
    if (size) {
      if (GWEN_Buffer_AppendBytes(gbuf,
                                  GWEN_Buffer_GetStart(data),
                                  size)) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "here");
        return -1;
      }
      if (GWEN_Buffer_IncrementPos(data, size)) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "here");
        return -1;
      }
    }
    return 0;
  }

  else if (strcasecmp(type, "ascii")==0) {
    int size;

    if (1!=sscanf(GWEN_XMLNode_GetProperty(node, "size", "-1"),
                  "%i", &size)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad number of bytes");
      return -1;
    }
    if (size==-1) {
      size=GWEN_Buffer_GetUsedBytes(data);
    }
    else {
      if (size>GWEN_Buffer_GetUsedBytes(data)) {
        int lfiller;
        const char *lfs;

        /* check for left-filler, fill left if needed */
        lfs=GWEN_XMLNode_GetProperty(node, "lfiller", 0);
        if (lfs) {
          if (1!=sscanf(lfs, "%i", &lfiller)) {
            DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad value for property lfiller");
            return -1;
          }
          GWEN_Buffer_FillWithBytes(gbuf, (unsigned char)lfiller,
                                    size-GWEN_Buffer_GetUsedBytes(data));
        }
      }
    }

    if (GWEN_Buffer_AppendBytes(gbuf,
                                GWEN_Buffer_GetStart(data),
                                GWEN_Buffer_GetUsedBytes(data))) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here");
      return -1;
    }
    return 0;
  }

  else {
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Type \"%s\" not supported by AB_MsgEngine", type);
    return 1;
  }
}



GWEN_DB_NODE_TYPE AB_MsgEngine_TypeCheck(GWEN_MSGENGINE *e,
					 const char *tname){
  AB_MSGENGINE *le;

  assert(e);
  le=GWEN_INHERIT_GETDATA(GWEN_MSGENGINE, AB_MSGENGINE, e);
  assert(le);

  if (strcasecmp(tname, "byte")==0 ||
      strcasecmp(tname, "word")==0 ||
      strcasecmp(tname, "dword")==0)
    return GWEN_DB_NodeType_ValueInt;
  else if (strcasecmp(tname, "bytes")==0 ||
           strcasecmp(tname, "tlv")==0)
    return GWEN_DB_NodeType_ValueBin;
  else if (strcasecmp(tname, "bcd")==0)
    return GWEN_DB_NodeType_ValueChar;
  else
    return GWEN_DB_NodeType_Unknown;
}



const char *AB_MsgEngine_GetCharValue(GWEN_MSGENGINE *e,
                                      const char *name,
                                      const char *defValue){
  AB_MSGENGINE *le;

  assert(e);
  le=GWEN_INHERIT_GETDATA(GWEN_MSGENGINE, AB_MSGENGINE, e);
  assert(le);

  return defValue;
}



int AB_MsgEngine_GetIntValue(GWEN_MSGENGINE *e,
                             const char *name,
                             int defValue){
  AB_MSGENGINE *le;

  assert(e);
  le=GWEN_INHERIT_GETDATA(GWEN_MSGENGINE, AB_MSGENGINE, e);
  assert(le);

  return defValue;
}



int AB_MsgEngine_BinTypeRead(GWEN_MSGENGINE *e,
                             GWEN_XMLNODE *node,
                             GWEN_DB_NODE *gr,
                             GWEN_BUFFER *vbuf){
  const char *typ;

  typ=GWEN_XMLNode_GetProperty(node, "type", "");
  if (strcasecmp(typ, "tlv")==0) {
    int isBerTlv;
    const char *p;
    unsigned int tagType;
    unsigned int tagLength;
    //const char *tagData;
    unsigned int size;
    unsigned int pos;
    unsigned int j;
    GWEN_XMLNODE *tlvNode;
    GWEN_DB_NODE *ngr;
    const char *name;

    GWEN_Buffer_Rewind(vbuf);
    if (!GWEN_Buffer_GetBytesLeft(vbuf)) {
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "Buffer empty");
      return 0;
    }

    DBG_VERBOUS(AQBANKING_LOGDOMAIN, "Entering BinTypeRead with this:");
    if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Verbous)
      GWEN_Buffer_Dump(vbuf, 2);

    p=GWEN_Buffer_GetStart(vbuf);
    pos=0;
    size=GWEN_Buffer_GetBytesLeft(vbuf);
    isBerTlv=(strcasecmp(GWEN_XMLNode_GetProperty(node,
                                                  "tlvtype",
                                                  "bertlv"),
                         "BER")==0);
    /* get tag type */
    if (size<2) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes for BER-TLV");
      return -1;
    }
    j=(unsigned char)(p[pos]);
    if (isBerTlv) {
      if ((j & 0x1f)==0x1f) {
        pos++;
        if (pos>=size) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes");
          return -1;
        }
        j=(unsigned char)(p[pos]);
      }
      else
        j&=0x1f;
    }
    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Tag type %02x%s", j,
	      isBerTlv?" (BER-TLV)":"");
    tagType=j;

    /* get length */
    pos++;
    if (pos>=size) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes");
      return -1;
    }
    j=(unsigned char)(p[pos]);
    if (isBerTlv) {
      if (j & 0x80) {
        if (j==0x81) {
          pos++;
          if (pos>=size) {
            DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes");
            return -1;
          }
          j=(unsigned char)(p[pos]);
        } /* 0x81 */
        else if (j==0x82) {
          if (pos+1>=size) {
            DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes");
            return -1;
          }
          pos++;
          j=((unsigned char)(p[pos]))<<8;
          pos++;
          j+=(unsigned char)(p[pos]);
        } /* 0x82 */
        else {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Unexpected tag length modifier %02x", j);
          return -1;
        }
      } /* if tag length modifier */
    }
    else {
      if (j==255) {
        if (pos+2>=size) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes");
          return -1;
        }
        pos++;
        j=((unsigned char)(p[pos]))<<8;
        pos++;
        j+=(unsigned char)(p[pos]);
      }
    }
    pos++;
    tagLength=j;
    //tagData=p+pos;
    GWEN_Buffer_SetPos(vbuf, pos);

    DBG_DEBUG(AQBANKING_LOGDOMAIN, "Tag: %02x (%d bytes)", tagType, tagLength);
    if (pos+j>size) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes");
      return -1;
    }

    /* parse TLV data */
    tlvNode=GWEN_XMLNode_FindFirstTag(node, "tlv", 0, 0);
    while (tlvNode) {
      int ltagType;

      if (1!=sscanf(GWEN_XMLNode_GetProperty(tlvNode, "id", "-1"),
                    "%i", &ltagType)) {
        DBG_WARN(AQBANKING_LOGDOMAIN, "Bad tag id in XML file");
      }
      else {
        if (ltagType==tagType) {
          DBG_DEBUG(AQBANKING_LOGDOMAIN, "Tag %02x found in XML file", ltagType);
          name=GWEN_XMLNode_GetProperty(node, "name", 0);
          ngr=gr;
          if (name) {
            if (*name) {
	      ngr=GWEN_DB_GetGroup(gr,
				   GWEN_DB_FLAGS_DEFAULT,
				   name);
              assert(ngr);
            }
          }
          name=GWEN_XMLNode_GetProperty(tlvNode, "name", 0);
          if (name) {
            if (*name) {
	      ngr=GWEN_DB_GetGroup(ngr,
				   GWEN_DB_FLAGS_DEFAULT |
                                   GWEN_PATH_FLAGS_CREATE_GROUP,
				   name);
              assert(ngr);
            }
          }
	  if (tagLength) {
            if (GWEN_MsgEngine_ParseMessage(e,
                                            tlvNode,
                                            vbuf,
                                            ngr,
                                            GWEN_MSGENGINE_READ_FLAGS_DEFAULT)){
              DBG_INFO(AQBANKING_LOGDOMAIN, "here");
              return -1;
            }
	  }
          return 0;
        } /* if tag id matches */
      } /* if id is ok */
      tlvNode=GWEN_XMLNode_FindNextTag(tlvNode, "tlv", 0, 0);
    } /* while */

    DBG_INFO(AQBANKING_LOGDOMAIN, "Tag \"%02x\" not found", tagType);
    name=GWEN_XMLNode_GetProperty(node, "name", 0);
    ngr=gr;
    if (name) {
      if (*name) {
        ngr=GWEN_DB_GetGroup(gr,
                             GWEN_DB_FLAGS_DEFAULT,
                             name);
        assert(ngr);
      }
    }
    ngr=GWEN_DB_GetGroup(ngr,
                         GWEN_PATH_FLAGS_CREATE_GROUP,
                         "UnknownTag");
    assert(ngr);
    GWEN_DB_SetIntValue(ngr, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "tag", tagType);
    GWEN_DB_SetBinValue(ngr, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "data",
                        GWEN_Buffer_GetPosPointer(vbuf),
                        GWEN_Buffer_GetBytesLeft(vbuf));

    return 0;
  }
  else {
    return 1;
  }
}



int AB_MsgEngine_BinTypeWrite(GWEN_MSGENGINE *e,
                              GWEN_XMLNODE *node,
                              GWEN_DB_NODE *gr,
                              GWEN_BUFFER *dbuf){
  return 1;
}












