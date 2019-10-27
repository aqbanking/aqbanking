/***************************************************************************
 begin       : Sun Oct 27 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "s_pintan.h"
#include "s_pintan_sign.h"
#include "s_pintan_crypt.h"

#include <gwenhywfar/debug.h>




static int GWENHYWFAR_CB _exchangeMessages(AQFINTS_SESSION *sess,
                                           AQFINTS_MESSAGE *messageOut,
                                           AQFINTS_MESSAGE **pMessageIn);
GWEN_BUFFER *_encodeMessage(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *msg);





AQFINTS_SESSION *AQFINTS_SessionPinTan_new(AQFINTS_PARSER *parser, AQFINTS_TRANSPORT *trans)
{
  AQFINTS_SESSION *sess;

  sess=AQFINTS_Session_new(parser, trans);
  assert(sess);

  AQFINTS_Session_SetExchangeMessagesFn(sess, _exchangeMessages);

  return sess;
}






int _exchangeMessages(AQFINTS_SESSION *sess,
                      AQFINTS_MESSAGE *messageOut,
                      AQFINTS_MESSAGE **pMessageIn)
{
  GWEN_BUFFER *msgBuffer;


  msgBuffer=_encodeMessage(sess, messageOut);
  if (msgBuffer==NULL) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }


  return GWEN_ERROR_NOT_IMPLEMENTED;
}



GWEN_BUFFER *_encodeMessage(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *msg)
{
  AQFINTS_SEGMENT_LIST *segmentList;
  GWEN_BUFFER *msgBuffer;
  int firstSegNum;
  int lastSegNum;
  uint32_t flags;
  int rv;

  AQFINTS_Message_Reenumerate(msg);
  firstSegNum=AQFINTS_Message_GetFirstSegNum(msg);
  lastSegNum=AQFINTS_Message_GetLastSegNum(msg);

  segmentList=AQFINTS_Message_GetSegmentList(msg);
  assert(segmentList);

  flags=AQFINTS_Segment_List_SampleFlags(segmentList);

  msgBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  AQFINTS_Message_WriteSegments(msg, msgBuffer);

  if (flags & AQFINTS_SEGMENT_FLAGS_SIGN) {
    AQFINTS_KEYNAME_LIST *signerKeyNameList;

    signerKeyNameList=AQFINTS_Message_GetSignerList(msg);
    if (signerKeyNameList==NULL || AQFINTS_KeyName_List_GetCount(signerKeyNameList)==0) {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "Signing requested but no signer in list");
      GWEN_Buffer_free(msgBuffer);
      return NULL;
    }
    rv=AQFINTS_SessionPinTan_WrapSignatures(sess, signerKeyNameList, firstSegNum, lastSegNum, msgBuffer);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(msgBuffer);
      return NULL;
    }
    /* adjust segment numbers, since seignature also have segment numbers */
    firstSegNum-=AQFINTS_KeyName_List_GetCount(signerKeyNameList);;
    lastSegNum+=AQFINTS_KeyName_List_GetCount(signerKeyNameList);
  }

  if (flags & AQFINTS_SEGMENT_FLAGS_CRYPT) {
    AQFINTS_KEYNAME *crypterKeyName;
    GWEN_BUFFER *tmpBuffer;

    crypterKeyName=AQFINTS_Message_GetCrypter(msg);
    if (crypterKeyName==NULL) {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "Encryption requested but no crypter given");
      GWEN_Buffer_free(msgBuffer);
      return NULL;
    }

    tmpBuffer=GWEN_Buffer_new(0, 256, 0, 1);
    rv=AQFINTS_SessionPinTan_WrapCrypt(sess, crypterKeyName,
                                       (const uint8_t*) GWEN_Buffer_GetStart(msgBuffer),
                                       GWEN_Buffer_GetUsedBytes(msgBuffer),
                                       tmpBuffer);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tmpBuffer);
      GWEN_Buffer_free(msgBuffer);
      return NULL;
    }
    GWEN_Buffer_free(msgBuffer);
    msgBuffer=tmpBuffer;
  }

  rv=AQFINTS_Session_WrapMessageHeadAndTail(sess,
                                            AQFINTS_Message_GetMessageNumber(msg),
                                            AQFINTS_Message_GetRefMessageNumber(msg),
                                            lastSegNum,
                                            msgBuffer);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(msgBuffer);
    return NULL;
  }

  return msgBuffer;
}





