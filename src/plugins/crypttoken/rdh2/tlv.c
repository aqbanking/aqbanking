/***************************************************************************
    begin       : Mon Feb 25 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "tlv_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(TLV, Tlv)


TLV *Tlv_new() {
  TLV *tlv;

  GWEN_NEW_OBJECT(TLV, tlv);
  GWEN_LIST_INIT(TLV, tlv);

  return tlv;
}



void Tlv_free(TLV *tlv) {
  if (tlv) {
    free(tlv->tagData);
    GWEN_LIST_FINI(TLV, tlv);
    GWEN_FREE_OBJECT(tlv);
  }
}



unsigned int Tlv_GetTagType(const TLV *tlv){
  assert(tlv);
  return tlv->tagType;
}



unsigned int Tlv_GetTagLength(const TLV *tlv){
  assert(tlv);
  return tlv->tagLength;
}



unsigned int Tlv_GetTagSize(const TLV *tlv){
  assert(tlv);
  return tlv->tagSize;
}



const void *Tlv_GetTagData(const TLV *tlv){
  assert(tlv);
  return tlv->tagData;
}



TLV *Tlv_fromBuffer(GWEN_BUFFER *mbuf) {
  const char *p;
  unsigned int tagMode;
  unsigned int tagType;
  unsigned int tagLength;
  const char *tagData;
  unsigned int size;
  unsigned int pos;
  unsigned int j;
  TLV *tlv;
  uint32_t startPos;

  if (!GWEN_Buffer_GetBytesLeft(mbuf)) {
    DBG_ERROR(0, "Buffer empty");
    return 0;
  }

  startPos=GWEN_Buffer_GetPos(mbuf);

  tagMode=tagType=tagLength=0;

  p=GWEN_Buffer_GetPosPointer(mbuf);
  pos=0;
  size=GWEN_Buffer_GetBytesLeft(mbuf);

  /* get tag type */
  if (pos+2>=size) {
    DBG_ERROR(0, "Too few bytes for TLV");
    return 0;
  }
  j=((unsigned char)(p[pos]))<<8;
  j|=(unsigned char)(p[pos+1]);
  pos+=2;
  tagType=j;

  /* get length */
  if (pos+2>=size) {
    DBG_ERROR(0, "Too few bytes");
    return 0;
  }
  j=((unsigned char)(p[pos+1]))<<8;
  j|=(unsigned char)(p[pos]);
  pos+=2;
  tagLength=j;
  tagData=p+pos;
  GWEN_Buffer_IncrementPos(mbuf, pos);

  tlv=Tlv_new();
  assert(tlv);
  tlv->tagType=tagType;
  tlv->tagLength=tagLength;
  if (tagLength) {
    tlv->tagData=(void*)malloc(tagLength);
    memmove(tlv->tagData, tagData, tagLength);
  }

  GWEN_Buffer_IncrementPos(mbuf, tagLength);
  tlv->tagSize=GWEN_Buffer_GetPos(mbuf)-startPos;
  return tlv;
}



void Tlv_DirectlyToBuffer(unsigned int tagType,
			  const char *p,
			  int size,
			  GWEN_BUFFER *buf){
  assert(buf);
  if (size==-1) {
    assert(p);
    size=strlen(p);
  }

  GWEN_Buffer_AppendByte(buf, (tagType>>8) & 0xff);
  GWEN_Buffer_AppendByte(buf, tagType & 0xff);
  GWEN_Buffer_AppendByte(buf, size & 0xff);
  GWEN_Buffer_AppendByte(buf, (size>>8)&0xff);
  if (size) {
    assert(p);
    GWEN_Buffer_AppendBytes(buf, p, size);
  }

}








