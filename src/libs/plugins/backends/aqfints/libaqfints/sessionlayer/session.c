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
#include "sessionlayer/s_encode.h"
#include "sessionlayer/s_decode.h"
#include "parser/parser.h"
#include "parser/parser_dump.h"
#include "servicelayer/upd/upd_read.h"
#include "servicelayer/bpd/bpd_read.h"


#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static AQFINTS_MESSAGE* GWENHYWFAR_CB _exchangeMessagesInternal(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut);



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

  sess->exchangeMessagesFn=_exchangeMessagesInternal;

  return sess;
}



void AQFINTS_Session_free(AQFINTS_SESSION *sess)
{
  if (sess) {
    assert(sess->_refCount);
    if (sess->_refCount==1) {
      GWEN_INHERIT_FINI(AQFINTS_SESSION, sess)
      sess->_refCount=0;

      free(sess->logFile);
      free(sess->appRegKey);
      free(sess->appVersion);
      free(sess->dialogId);

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



int AQFINTS_Session_GetIsServer(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->isServer;
}



void AQFINTS_Session_SetIsServer(AQFINTS_SESSION *sess, int v)
{
  assert(sess);
  sess->isServer=v?1:0;
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


const char *AQFINTS_Session_GetLogFile(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->logFile;
}



void AQFINTS_Session_SetLogFile(AQFINTS_SESSION *sess, const char *s)
{
  assert(sess);
  if (sess->logFile)
    free(sess->logFile);
  if (s)
    sess->logFile=strdup(s);
  else
    sess->logFile=NULL;
}



const char *AQFINTS_Session_GetAppRegKey(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->appRegKey;
}



void AQFINTS_Session_SetAppRegKey(AQFINTS_SESSION *sess, const char *s)
{
  assert(sess);
  if (sess->appRegKey)
    free(sess->appRegKey);
  if (s)
    sess->appRegKey=strdup(s);
  else
    sess->appRegKey=NULL;
}



const char *AQFINTS_Session_GetAppVersion(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->appVersion;
}



void AQFINTS_Session_SetAppVersion(AQFINTS_SESSION *sess, const char *s)
{
  assert(sess);
  if (sess->appVersion)
    free(sess->appVersion);
  if (s)
    sess->appVersion=strdup(s);
  else
    sess->appVersion=NULL;
}











AQFINTS_MESSAGE *AQFINTS_Session_ExchangeMessages(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut)
{
  assert(sess);
  if (sess->exchangeMessagesFn)
    return sess->exchangeMessagesFn(sess, messageOut);
  else
    return NULL;
}



AQFINTS_MESSAGE *_exchangeMessagesInternal(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut)
{
  return AQFINTS_Session_DirectlyExchangeMessages(sess, messageOut);
}



int AQFINTS_Session_FilloutKeyname(AQFINTS_SESSION *sess, AQFINTS_KEYDESCR *keyDescr, int mode)
{
  assert(sess);
  if (sess->filloutKeynameFn)
    return sess->filloutKeynameFn(sess, keyDescr, mode);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AQFINTS_Session_DecryptSessionKey(AQFINTS_SESSION *sess,
                                      AQFINTS_KEYDESCR *keyDescr,
                                      GWEN_CRYPT_PADDALGO *paddAlgo,
                                      const uint8_t *pInData,
                                      uint32_t inLen,
                                      uint8_t *pOutData,
                                      uint32_t *pOutLen)
{
  assert(sess);
  if (sess->decryptSessionKeyFn)
      return sess->decryptSessionKeyFn(sess, keyDescr, paddAlgo, pInData, inLen, pOutData, pOutLen);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AQFINTS_Session_VerifyPin(AQFINTS_SESSION *sess, const AQFINTS_KEYDESCR *keyDescr, const char *pin)
{
  assert(sess);
  if (sess->verifyPinFn)
    return sess->verifyPinFn(sess, keyDescr, pin);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AQFINTS_Session_Sign(AQFINTS_SESSION *sess,
                         const AQFINTS_KEYDESCR *keyDescr,
                         const AQFINTS_CRYPTPARAMS *cryptParams,
                         const uint8_t *pInData,
                         uint32_t inLen,
                         uint8_t *pSignatureData,
                         uint32_t *pSignatureLen)
{
  assert(sess);
  if (sess->signFn)
    return sess->signFn(sess, keyDescr, cryptParams, pInData, inLen, pSignatureData, pSignatureLen);
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



AQFINTS_SESSION_FILLOUT_KEYDESCR_FN AQFINTS_Session_SetFilloutKeynameFn(AQFINTS_SESSION *sess,
                                                                       AQFINTS_SESSION_FILLOUT_KEYDESCR_FN fn)
{
  AQFINTS_SESSION_FILLOUT_KEYDESCR_FN oldFn;

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



AQFINTS_SESSION_DECRYPT_SKEY_FN AQFINTS_Session_SetDecryptKeySessionFn(AQFINTS_SESSION *sess,
                                                                       AQFINTS_SESSION_DECRYPT_SKEY_FN fn)
{
  AQFINTS_SESSION_DECRYPT_SKEY_FN oldFn;

  assert(sess);
  oldFn=sess->decryptSessionKeyFn;
  sess->decryptSessionKeyFn=fn;
  return oldFn;
}








int AQFINTS_Session_WriteSegmentList(AQFINTS_SESSION *sess, AQFINTS_SEGMENT_LIST *segmentList)
{
  AQFINTS_SEGMENT *segment;

  segment=AQFINTS_Segment_List_First(segmentList);
  while (segment) {
    int rv;

    rv=AQFINTS_Session_WriteSegment(sess, segment);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    segment=AQFINTS_Segment_List_Next(segment);
  }

  return 0;
}



int AQFINTS_Session_WriteSegment(AQFINTS_SESSION *sess, AQFINTS_SEGMENT *segment)
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
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Segment has no DB data");
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

  rv=AQFINTS_Parser_WriteSegment(sess->parser, segment);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Error writing segment [%s] (%d)", AQFINTS_Segment_GetCode(segment), rv);
    AQFINTS_Parser_DumpSegment(segment, 2);
    return rv;
  }

  return 0;
}



int AQFINTS_Session_SampleAllowedTanMethods(int *ptrIntArray, int sizeIntArray, AQFINTS_SEGMENT_LIST *segmentList)
{
  AQFINTS_SEGMENT *segment;
  int numMethodsAdded=0;

  segment=AQFINTS_Segment_List_First(segmentList);
  while (segment) {
    AQFINTS_SEGMENT *nextSegment;
    const char *sCode;

    nextSegment=AQFINTS_Segment_List_Next(segment);

    sCode=AQFINTS_Segment_GetCode(segment);
    if (sCode && *sCode && strcasecmp(sCode, "HIRMS")==0) { /* check result */
      GWEN_DB_NODE *db;

      db=AQFINTS_Segment_GetDbData(segment);
      if (db) {
        GWEN_DB_NODE *dbResult;

        dbResult=GWEN_DB_FindFirstGroup(db, "result");
        while (dbResult) {
          int resultCode;
          const char *resultText;

          resultCode=GWEN_DB_GetIntValue(dbResult, "resultcode", 0, 0);
          resultText=GWEN_DB_GetCharValue(dbResult, "text", 0, 0);
          DBG_NOTICE(0, "Segment result: %d (%s)", resultCode, resultText?resultText:"<none>");
          if (resultCode==3920) {
            int i;

            /* reset array */
            for (i=0; i<sizeIntArray; i++)
              ptrIntArray[i]=0;

            for (i=0; i<sizeIntArray; i++) {
              int j;

              j=GWEN_DB_GetIntValue(dbResult, "param", i, 0);
              if (j==0)
                break;
	      DBG_NOTICE(0, "Adding allowed TAN method %d", j);
	      ptrIntArray[i]=j;
	      numMethodsAdded++;
            } /* for */
	  }
          dbResult=GWEN_DB_FindNextGroup(dbResult, "result");
        } /* while dbResult */
      } /* if db */
    }

    segment=nextSegment;
  } /* while segment */
  if (numMethodsAdded<1) {
    /* add single step if empty list */
    DBG_INFO(AQFINTS_LOGDOMAIN, "No allowed TAN method reported, assuming 999");
    ptrIntArray[0]=999;
    numMethodsAdded=1;
  }

  return numMethodsAdded;
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
  int rv;

  assert(sess);
  /* TODO: add logging mechanism */
  DBG_ERROR(AQFINTS_LOGDOMAIN, "Sending this:");
  GWEN_Text_LogString(ptrBuffer, lenBuffer, NULL, GWEN_LoggerLevel_Error);

  rv=AQFINTS_Transport_SendMessage(sess->transport, ptrBuffer, lenBuffer);
  sess->lastMessageNumSent++;
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}



int AQFINTS_Session_ReceiveMessage(AQFINTS_SESSION *sess, GWEN_BUFFER *buffer)
{
  int rv;

  /* TODO: add logging mechanism */
  rv=AQFINTS_Transport_ReceiveMessage(sess->transport, buffer);
  if (rv<0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  DBG_ERROR(AQFINTS_LOGDOMAIN, "Received this:");
  GWEN_Text_LogString(GWEN_Buffer_GetStart(buffer), GWEN_Buffer_GetUsedBytes(buffer), NULL, GWEN_LoggerLevel_Error);
  sess->lastMessageNumReceived++;
  return rv;
}




AQFINTS_BPD *AQFINTS_Session_ExtractBpdFromSegmentList(AQFINTS_SESSION *sess, AQFINTS_SEGMENT_LIST *segmentList)
{
  AQFINTS_BPD *bpd;

  bpd=AQFINTS_Bpd_SampleBpdFromSegmentList(sess->parser, segmentList, 1);
  if (bpd==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Empty BPD");
    return NULL;
  }

  return bpd;
}



AQFINTS_USERDATA_LIST *AQFINTS_Session_ExtractUpdFromSegmentList(AQFINTS_SESSION *sess,
                                                                 AQFINTS_SEGMENT_LIST *segmentList)
{
  AQFINTS_USERDATA_LIST *userDataList;

  userDataList=AQFINTS_Upd_SampleUpdFromSegmentList(segmentList, 1);
  if (userDataList==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Empty userDataList");
    return NULL;
  }

  return userDataList;
}





AQFINTS_MESSAGE *AQFINTS_Session_DirectlyExchangeMessages(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut)
{
  GWEN_BUFFER *msgBuffer;
  AQFINTS_MESSAGE *message;
  int rv;


  msgBuffer=AQFINTS_Session_EncodeMessage(sess, messageOut);
  if (msgBuffer==NULL) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here");
    return NULL;
  }

  rv=AQFINTS_Session_SendMessage(sess, GWEN_Buffer_GetStart(msgBuffer), GWEN_Buffer_GetUsedBytes(msgBuffer));
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here");
    GWEN_Buffer_free(msgBuffer);
    return NULL;
  }
  GWEN_Buffer_Reset(msgBuffer);

  rv=AQFINTS_Session_ReceiveMessage(sess, msgBuffer);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here");
    GWEN_Buffer_free(msgBuffer);
    return NULL;
  }

  message=AQFINTS_Session_DecodeMessage(sess, (const uint8_t*) GWEN_Buffer_GetStart(msgBuffer), GWEN_Buffer_GetUsedBytes(msgBuffer));
  if (message==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Could not decode this message:");
    GWEN_Buffer_Dump(msgBuffer, 2);
    GWEN_Buffer_free(msgBuffer);
    return NULL;
  }
  GWEN_Buffer_free(msgBuffer);

  return message;
}




