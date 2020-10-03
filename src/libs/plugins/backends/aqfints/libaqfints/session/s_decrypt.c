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

#include "session/s_decrypt.h"
#include "session/pintan/s_decrypt_pintan.h"
#include "session/hbci/s_decrypt_hbci.h"
#include "session/s_decode.h"
#include "parser/parser.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _decryptMessage(AQFINTS_SESSION *sess,
                           AQFINTS_SEGMENT *segCryptHead,
                           AQFINTS_SEGMENT *segCryptData,
                           AQFINTS_MESSAGE *message);
static int _decryptSegmentDdv(AQFINTS_SESSION *sess,
                              AQFINTS_SEGMENT *segCryptHead,
                              AQFINTS_SEGMENT *segCryptData,
                              const AQFINTS_CRYPTPARAMS *cryptParams,
                              const AQFINTS_KEYDESCR *keyDescr,
                              AQFINTS_SEGMENT_LIST *segmentList);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Session_DecryptMessage(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *message)
{
  AQFINTS_SEGMENT_LIST *segmentList;
  AQFINTS_SEGMENT *segment;
  const char *sCode;

  segmentList=AQFINTS_Message_GetSegmentList(message);
  segment=AQFINTS_Segment_List_First(segmentList);
  if (segment==NULL)
    return GWEN_ERROR_NO_DATA;
  sCode=AQFINTS_Segment_GetCode(segment);
  if (sCode==NULL || *sCode==0 || strcasecmp(sCode, "HNHBK")) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No message head");
    return GWEN_ERROR_INVALID;
  }

  segment=AQFINTS_Segment_List_Next(segment);
  if (segment==NULL)
    return GWEN_ERROR_NO_DATA;
  sCode=AQFINTS_Segment_GetCode(segment);
  if (sCode && *sCode && strcasecmp(sCode, "HNVSK")==0) {
    AQFINTS_SEGMENT *segCryptHead=NULL;
    AQFINTS_SEGMENT *segCryptData=NULL;
    AQFINTS_SEGMENT *segMsgTail=NULL;
    int rv;

    segCryptHead=segment;

    segment=AQFINTS_Segment_List_Next(segment);
    if (segment==NULL)
      return GWEN_ERROR_NO_DATA;
    sCode=AQFINTS_Segment_GetCode(segment);
    if (sCode==NULL || *sCode==0 || strcasecmp(sCode, "HNVSD")) {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "No message data");
      return GWEN_ERROR_INVALID;
    }
    segCryptData=segment;

    segment=AQFINTS_Segment_List_Next(segment);
    if (segment==NULL)
      return GWEN_ERROR_NO_DATA;
    sCode=AQFINTS_Segment_GetCode(segment);
    if (sCode==NULL || *sCode==0 || strcasecmp(sCode, "HNHBS")) {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "No message tail");
      return GWEN_ERROR_INVALID;
    }
    segMsgTail=segment;

    if (AQFINTS_Segment_List_Next(segment)!=NULL) {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "Unexpected segment behind MessageTail");
      return GWEN_ERROR_INVALID;
    }

    rv=_decryptMessage(sess, segCryptHead, segCryptData, message);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    /* move cryptHead to end of list */
    AQFINTS_Segment_List_Del(segCryptHead);
    AQFINTS_Segment_List_Add(segCryptHead, segmentList);

    /* move cryptData to end of list */
    AQFINTS_Segment_List_Del(segCryptData);
    AQFINTS_Segment_List_Add(segCryptData, segmentList);

    /* move messageTail to end of list */
    AQFINTS_Segment_List_Del(segMsgTail);
    AQFINTS_Segment_List_Add(segMsgTail, segmentList);
  }
  else {
    segment=AQFINTS_Segment_List_Last(segmentList);
    if (segment==NULL)
      return GWEN_ERROR_NO_DATA;
    sCode=AQFINTS_Segment_GetCode(segment);
    if (sCode==NULL || *sCode==0 || strcasecmp(sCode, "HNHBS")) {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "No message tail");
      return GWEN_ERROR_INVALID;
    }
  }

  return 0;
}



int _decryptMessage(AQFINTS_SESSION *sess,
                    AQFINTS_SEGMENT *segCryptHead,
                    AQFINTS_SEGMENT *segCryptData,
                    AQFINTS_MESSAGE *message)
{
  AQFINTS_SEGMENT_LIST *segmentList;
  GWEN_DB_NODE *dbCryptHead;
  AQFINTS_KEYDESCR *keyDescr;
  const char *securityProfileName;
  int securityProfileVersion;

  segmentList=AQFINTS_Message_GetSegmentList(message);
  dbCryptHead=AQFINTS_Segment_GetDbData(segCryptHead);
  assert(dbCryptHead);

  keyDescr=AQFINTS_Session_ReadKeyDescrFromDbHead(dbCryptHead);

  securityProfileName=GWEN_DB_GetCharValue(dbCryptHead, "secProfile/code", 0, NULL);
  securityProfileVersion=GWEN_DB_GetIntValue(dbCryptHead, "secProfile/version", 0, 0);

  /* hack for hibiscus */
  if (securityProfileVersion==0) {
    if (securityProfileName && strcasecmp(securityProfileName, "RDH")==0)
      securityProfileVersion=10;
  }


  if (securityProfileName && *securityProfileName) {
    int rv;
    const AQFINTS_CRYPTPARAMS *cryptParams;

    DBG_INFO(AQFINTS_LOGDOMAIN, "Selected security profile is \"%s\" (version %d)", securityProfileName,
             securityProfileVersion);
    cryptParams=AQFINTS_CryptParams_GetParamsForSecurityProfile(securityProfileName, securityProfileVersion);
    if (cryptParams==NULL) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "Security profile \"%s\" (version %d) no found", securityProfileName,
               securityProfileVersion);
      AQFINTS_KeyDescr_free(keyDescr);
      return GWEN_ERROR_GENERIC;
    }
    if (strcasecmp(securityProfileName, "PIN")==0)
      rv=AQFINTS_Session_DecryptSegmentPinTan(sess, segCryptHead, segCryptData, cryptParams, keyDescr, segmentList);
    else if (strcasecmp(securityProfileName, "RDH")==0)
      rv=AQFINTS_Session_DecryptSegmentHbci(sess, segCryptHead, segCryptData, cryptParams, keyDescr, segmentList);
    else if (strcasecmp(securityProfileName, "RAH")==0)
      rv=AQFINTS_Session_DecryptSegmentHbci(sess, segCryptHead, segCryptData, cryptParams, keyDescr, segmentList);
    else if (strcasecmp(securityProfileName, "DDV")==0)
      rv=_decryptSegmentDdv(sess, segCryptHead, segCryptData, cryptParams, keyDescr, segmentList);
    else {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "Invalid security profile \"%s\"", securityProfileName);
      AQFINTS_KeyDescr_free(keyDescr);
      return GWEN_ERROR_BAD_DATA;
    }
    if (rv<0) {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      AQFINTS_KeyDescr_free(keyDescr);
      return rv;
    }
    AQFINTS_Message_SetCrypter(message, keyDescr);
    return 0;
  }
  else {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Missing security profile code");
    AQFINTS_KeyDescr_free(keyDescr);
    return GWEN_ERROR_BAD_DATA;
  }
}



int _decryptSegmentDdv(AQFINTS_SESSION *sess,
                       AQFINTS_SEGMENT *segCryptHead,
                       AQFINTS_SEGMENT *segCryptData,
                       const AQFINTS_CRYPTPARAMS *cryptParams,
                       const AQFINTS_KEYDESCR *keyDescr,
                       AQFINTS_SEGMENT_LIST *segmentList)
{
  return GWEN_ERROR_NOT_IMPLEMENTED;
}


