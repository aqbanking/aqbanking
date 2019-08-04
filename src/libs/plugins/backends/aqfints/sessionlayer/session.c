/***************************************************************************
 begin       : Thu Aug 01 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "./session_p.h"
#include "msglayer/parser/parser.h"
#include "servicelayer/upd/upd_read.h"
#include "servicelayer/bpd/bpd_read.h"


#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */





/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */




GWEN_INHERIT_FUNCTIONS(AQFINTS_SESSION)



AQFINTS_SESSION *AQFINTS_Session_new(AQFINTS_PARSER *parser, AQFINTS_TRANSPORT *trans)
{
  AQFINTS_SESSION *sess;

  GWEN_NEW_OBJECT(AQFINTS_SESSION, sess);
  sess->_refCount=1;
  GWEN_INHERIT_INIT(AQFINTS_SESSION, sess);
  sess->lastMessageNumSent=0;
  sess->lastMessageNumReceived=0;
  sess->hbciVersion=300;

  sess->parser=parser;
  sess->transport=trans;

  return sess;
}



void AQFINTS_Session_free(AQFINTS_SESSION *sess)
{
  if (sess) {
    assert(sess->_refCount);
    if (sess->_refCount==1) {
      GWEN_INHERIT_FINI(AQFINTS_SESSION, sess)
      sess->_refCount=0;

      if (sess->dialogId)
        free(sess->dialogId);

      if (sess->userDataList)
        AQFINTS_UserData_List_free(sess->userDataList);

      if (sess->bpd)
        AQFINTS_Bpd_free(sess->bpd);

      if (sess->transport)
        AQFINTS_Transport_free(sess->transport);

      GWEN_FREE_OBJECT(sess);
    }
    else
      sess->_refCount--;
  }
}



void AQFINTS_Session_Attach(AQFINTS_SESSION *sess)
{
  assert(sess);
  assert(sess->_refCount);
  sess->_refCount++;
}



int AQFINTS_Session_GetHbciVersion(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->hbciVersion;
}



void AQFINTS_Session_SetHbciVersion(AQFINTS_SESSION *sess, int v)
{
  assert(sess);
  sess->hbciVersion=v;
}



const char *AQFINTS_Session_GetSecProfileCode(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->secProfileCode;
}



void AQFINTS_Session_SetSecProfileCode(AQFINTS_SESSION *sess, const char *s)
{
  assert(sess);
  if (sess->secProfileCode)
    free(sess->secProfileCode);
  if (s)
    sess->secProfileCode=strdup(s);
  else
    sess->secProfileCode=NULL;
}



int AQFINTS_Session_GetSecProfileVersion(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->secProfileVersion;
}



void AQFINTS_Session_SetSecProfileVersion(AQFINTS_SESSION *sess, int i)
{
  assert(sess);
  sess->secProfileVersion=i;
}



const char *AQFINTS_Session_GetDialogId(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->dialogId;
}



void AQFINTS_Session_SetDialogId(AQFINTS_SESSION *sess, const char *s)
{
  assert(sess);
  if (sess->dialogId)
    free(sess->dialogId);
  if (s)
    sess->dialogId=strdup(s);
  else
    sess->dialogId=NULL;
}



int AQFINTS_Session_GetLastMessageNumSent(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->lastMessageNumSent;
}



void AQFINTS_Session_SetLastMessageNumSent(AQFINTS_SESSION *sess, int p_src)
{
  assert(sess);
  sess->lastMessageNumSent=p_src;
}



int AQFINTS_Session_GetLastMessageNumReceived(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->lastMessageNumReceived;
}



void AQFINTS_Session_SetLastMessageNumReceived(AQFINTS_SESSION *sess, int p_src)
{
  assert(sess);
  sess->lastMessageNumReceived=p_src;
}



AQFINTS_PARSER *AQFINTS_Session_GetParser(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->parser;
}



AQFINTS_USERDATA_LIST *AQFINTS_Session_GetUserDataList(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->userDataList;
}



void AQFINTS_Session_SetUserDataList(AQFINTS_SESSION *sess, AQFINTS_USERDATA_LIST *userDataList)
{
  assert(sess);
  if (sess->userDataList)
    AQFINTS_UserData_List_free(sess->userDataList);
  sess->userDataList=userDataList;
}



AQFINTS_BPD *AQFINTS_Session_GetBpd(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->bpd;
}



void AQFINTS_Session_SetBpd(AQFINTS_SESSION *sess, AQFINTS_BPD *bpd)
{
  assert(sess);
  if (sess->bpd)
    AQFINTS_Bpd_free(sess->bpd);
  sess->bpd=bpd;
}




int AQFINTS_Session_ExchangeMessages(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut,
                                     AQFINTS_MESSAGE **pMessageIn)
{
  assert(sess);
  if (sess->exchangeMessagesFn)
    return sess->exchangeMessagesFn(sess, messageOut, pMessageIn);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AQFINTS_Session_FilloutKeyname(AQFINTS_SESSION *sess, AQFINTS_KEYNAME *keyName)
{
  assert(sess);
  if (sess->filloutKeynameFn)
    return sess->filloutKeynameFn(sess, keyName);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AQFINTS_Session_Sign(AQFINTS_SESSION *sess, AQFINTS_KEYNAME *keyName, GWEN_BUFFER *dataBuffer)
{
  assert(sess);
  if (sess->signFn)
    return sess->signFn(sess, keyName, dataBuffer);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AQFINTS_Session_Verify(AQFINTS_SESSION *sess, AQFINTS_KEYNAME *keyName, GWEN_BUFFER *dataBuffer,
			   const uint8_t *ptrSignature, uint32_t lenSignature)
{
  assert(sess);
  if (sess->verifyFn)
    return sess->verifyFn(sess, keyName, dataBuffer, ptrSignature, lenSignature);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AQFINTS_Session_Encrypt(AQFINTS_SESSION *sess, AQFINTS_KEYNAME *keyName, GWEN_BUFFER *dataBuffer)
{
  assert(sess);
  if (sess->encryptFn)
    return sess->encryptFn(sess, keyName, dataBuffer);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AQFINTS_Session_Decrypt(AQFINTS_SESSION *sess, AQFINTS_KEYNAME *keyName, GWEN_BUFFER *dataBuffer)
{
  assert(sess);
  if (sess->decryptFn)
    return sess->decryptFn(sess, keyName, dataBuffer);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



AQFINTS_SESSION_EXCHANGEMESSAGES_FN AQFINTS_Session_SetExchangeMessagesFn(AQFINTS_SESSION *sess,
                                                                          AQFINTS_SESSION_EXCHANGEMESSAGES_FN fn)
{
  AQFINTS_SESSION_EXCHANGEMESSAGES_FN oldFn;

  assert(sess);
  oldFn=sess->exchangeMessagesFn;
  sess->exchangeMessagesFn=fn;
  return oldFn;
}



AQFINTS_SESSION_FILLOUT_KEYNAME_FN AQFINTS_Session_SetFilloutKeynameFn(AQFINTS_SESSION *sess,
								       AQFINTS_SESSION_FILLOUT_KEYNAME_FN fn)
{
  AQFINTS_SESSION_FILLOUT_KEYNAME_FN oldFn;

  assert(sess);
  oldFn=sess->filloutKeynameFn;
  sess->filloutKeynameFn=fn;
  return oldFn;
}



AQFINTS_SESSION_SIGN_FN AQFINTS_Session_SetSignFn(AQFINTS_SESSION *sess, AQFINTS_SESSION_SIGN_FN fn)
{
  AQFINTS_SESSION_SIGN_FN oldFn;

  assert(sess);
  oldFn=sess->signFn;
  sess->signFn=fn;
  return oldFn;
}



AQFINTS_SESSION_VERIFY_FN AQFINTS_Session_SetVerifyFn(AQFINTS_SESSION *sess, AQFINTS_SESSION_VERIFY_FN fn)
{
  AQFINTS_SESSION_VERIFY_FN oldFn;

  assert(sess);
  oldFn=sess->verifyFn;
  sess->verifyFn=fn;
  return oldFn;
}



AQFINTS_SESSION_ENCRYPT_FN AQFINTS_Session_SetEncryptFn(AQFINTS_SESSION *sess, AQFINTS_SESSION_ENCRYPT_FN fn)
{
  AQFINTS_SESSION_ENCRYPT_FN oldFn;

  assert(sess);
  oldFn=sess->encryptFn;
  sess->encryptFn=fn;
  return oldFn;
}



AQFINTS_SESSION_DECRYPT_FN AQFINTS_Session_SetDecryptFn(AQFINTS_SESSION *sess, AQFINTS_SESSION_DECRYPT_FN fn)
{
  AQFINTS_SESSION_DECRYPT_FN oldFn;

  assert(sess);
  oldFn=sess->decryptFn;
  sess->decryptFn=fn;
  return oldFn;
}









int AQFINTS_Session_WriteSegmentList(AQFINTS_SESSION *sess, AQFINTS_SEGMENT_LIST *segmentList,
                                     int firstSegNum, int refSegNum, GWEN_BUFFER *destBuffer)
{
  AQFINTS_SEGMENT *segment;
  int segNum=firstSegNum;

  segment=AQFINTS_Segment_List_First(segmentList);
  while(segment) {
    int rv;

    AQFINTS_Segment_SetSegmentNumber(segment, segNum++);
    AQFINTS_Segment_SetRefSegmentNumber(segment, refSegNum);

    rv=AQFINTS_Session_WriteSegment(sess, segment, destBuffer);
    if (rv<0) {
      DBG_INFO(0, "here (%d)", rv);
      return rv;
    }
    segment=AQFINTS_Segment_List_Next(segment);
  }

  return 0;
}



int AQFINTS_Session_WriteSegment(AQFINTS_SESSION *sess, AQFINTS_SEGMENT *segment, GWEN_BUFFER *destBuffer)
{
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbHead;
  const char *sCode;
  int segVersion;
  int segNum;
  int refSegNum;
  int rv;

  sCode=AQFINTS_Segment_GetCode(segment);
  segVersion=AQFINTS_Segment_GetSegmentVersion(segment);
  segNum=AQFINTS_Segment_GetSegmentNumber(segment);
  refSegNum=AQFINTS_Segment_GetRefSegmentNumber(segment);

  db=AQFINTS_Segment_GetDbData(segment);
  if (db==NULL) {
    DBG_ERROR(0, "Segment has no DB data");
    return GWEN_ERROR_INTERNAL;
  }

  /* prepare segment head */
  dbHead=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "head");
  assert(dbHead);
  GWEN_DB_SetCharValue(dbHead, GWEN_DB_FLAGS_OVERWRITE_VARS, "code", sCode);
  GWEN_DB_SetIntValue(dbHead, GWEN_DB_FLAGS_OVERWRITE_VARS, "seq", segNum);
  GWEN_DB_SetIntValue(dbHead, GWEN_DB_FLAGS_OVERWRITE_VARS, "version", segVersion);
  if (refSegNum)
    GWEN_DB_SetIntValue(dbHead, GWEN_DB_FLAGS_OVERWRITE_VARS, "ref", refSegNum);

  rv=AQFINTS_Parser_WriteSegment(sess->parser, segment, destBuffer);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  return 0;
}



void AQFINTS_Session_ExtractBpdAndUpd(AQFINTS_SESSION *sess, AQFINTS_SEGMENT_LIST *segmentList)
{
  AQFINTS_USERDATA_LIST *userDataList;
  AQFINTS_BPD *bpd;

  bpd=AQFINTS_Bpd_SampleBpdFromSegmentList(sess->parser, segmentList, 1);
  if (bpd==NULL) {
    DBG_ERROR(0, "Empty BPD");
  }
  else
    AQFINTS_Session_SetBpd(sess, bpd);

  userDataList=AQFINTS_Upd_SampleUpdFromSegmentList(segmentList, 1);
  if (userDataList==NULL) {
    DBG_ERROR(0, "Empty userDataList");
  }
  else
    AQFINTS_Session_SetUserDataList(sess, userDataList);

}





int AQFINTS_Session_Connect(AQFINTS_SESSION *sess)
{
  assert(sess);
  return AQFINTS_Transport_Connect(sess->transport);
}



int AQFINTS_Session_Disconnect(AQFINTS_SESSION *sess)
{
  assert(sess);
  return AQFINTS_Transport_Disconnect(sess->transport);
}



int AQFINTS_Session_SendMessage(AQFINTS_SESSION *sess, const char *ptrBuffer, int lenBuffer)
{
  assert(sess);
  /* TODO: add logging mechanism */
  DBG_ERROR(0, "Sending this:");
  GWEN_Text_LogString(ptrBuffer, lenBuffer, NULL, GWEN_LoggerLevel_Error);

  return AQFINTS_Transport_SendMessage(sess->transport, ptrBuffer, lenBuffer);
}



int AQFINTS_Session_ReceiveMessage(AQFINTS_SESSION *sess, GWEN_BUFFER *buffer)
{
  int rv;

  /* TODO: add logging mechanism */
  rv=AQFINTS_Transport_ReceiveMessage(sess->transport, buffer);
  if (rv<0) {
    DBG_ERROR(0, "here (%d)", rv);
    return rv;
  }

  DBG_ERROR(0, "Received this:");
  GWEN_Text_LogString(GWEN_Buffer_GetStart(buffer), GWEN_Buffer_GetUsedBytes(buffer), NULL, GWEN_LoggerLevel_Error);
  return rv;
}








