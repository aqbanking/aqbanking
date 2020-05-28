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

#include "sessionlayer/hbci/s_sign_hbci.h"
#include "sessionlayer/s_decode.h"
#include "sessionlayer/cryptparams.h"
#include "parser/parser.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/paddalgo.h>

#include <time.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static AQFINTS_SEGMENT *_createSigHead(AQFINTS_SESSION *sess, const AQFINTS_CRYPTPARAMS *cryptParams,
				       int segNum, const AQFINTS_KEYDESCR *keyDescr, const char *ctrlRef);
static AQFINTS_SEGMENT *_createSigTail(AQFINTS_SESSION *sess,
                                       const AQFINTS_CRYPTPARAMS *cryptParams,
                                       int segNum,
                                       const AQFINTS_KEYDESCR *keyDescr,
                                       const char *ctrlRef,
                                       const uint8_t *ptrSignature, uint32_t lenSignature);


static int _createCtrlRef(char *ptrBuf, size_t lenBuf);
static int _prepareSignSeg(AQFINTS_SESSION *sess,
			   const AQFINTS_CRYPTPARAMS *cryptParams,
                           const AQFINTS_KEYDESCR *keyDescr,
                           const char *ctrlRef,
                           GWEN_DB_NODE *cfg);

static int _signSegments(AQFINTS_SESSION *sess,
                         const AQFINTS_CRYPTPARAMS *cryptParams,
                         const AQFINTS_KEYDESCR *keyDescr,
                         AQFINTS_SEGMENT *segSigHead,
                         AQFINTS_SEGMENT *segFirstToSign,
                         AQFINTS_SEGMENT *segLastToSign,
                         GWEN_BUFFER *sigBuf);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Session_SignSegmentHbci(AQFINTS_SESSION *sess,
                                    AQFINTS_MESSAGE *message,
				    AQFINTS_KEYDESCR *keyDescr,
                                    AQFINTS_SEGMENT *segFirstToSign,
                                    AQFINTS_SEGMENT *segLastToSign,
                                    int sigHeadNum,
                                    int sigTailNum)
{
  int rv;
  AQFINTS_SEGMENT_LIST *segmentList;
  char ctrlref[15];
  AQFINTS_SEGMENT *segment;
  const char *securityProfileName;
  int securityProfileVersion;
  const AQFINTS_CRYPTPARAMS *cryptParams;
  GWEN_BUFFER *sigBuf;

  securityProfileName=AQFINTS_KeyDescr_GetSecurityProfileName(keyDescr);
  securityProfileVersion=AQFINTS_KeyDescr_GetSecurityProfileVersion(keyDescr);

  /* hack for hibiscus */
  if (securityProfileVersion==0) {
    if (securityProfileName && strcasecmp(securityProfileName, "RDH")==0)
      securityProfileVersion=10;
  }

  cryptParams=AQFINTS_CryptParams_GetParamsForSecurityProfile(securityProfileName, securityProfileVersion);
  if (cryptParams==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No crypt params for [%s:%d]", securityProfileName?securityProfileName:"<empty>", securityProfileVersion);
    return GWEN_ERROR_INVALID;
  }

  segmentList=AQFINTS_Message_GetSegmentList(message);

  rv=_createCtrlRef(ctrlref, sizeof(ctrlref));
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=AQFINTS_Session_FilloutKeyname(sess, keyDescr, AQFINTS_SESSION_CRYPTOP_SIGN);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  segment=_createSigHead(sess, cryptParams, sigHeadNum, keyDescr, ctrlref);
  if (segment==NULL) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here");
    return GWEN_ERROR_GENERIC;
  }
  AQFINTS_Segment_List_Insert(segment, segmentList);

  sigBuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=_signSegments(sess, cryptParams, keyDescr, segment, segFirstToSign, segLastToSign, sigBuf);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(sigBuf);
    return rv;
  }

  segment=_createSigTail(sess, cryptParams, sigTailNum, keyDescr, ctrlref,
                         (const uint8_t*) GWEN_Buffer_GetStart(sigBuf),
                         GWEN_Buffer_GetUsedBytes(sigBuf));
  if (segment==NULL) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here");
    GWEN_Buffer_free(sigBuf);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(sigBuf);
  AQFINTS_Segment_List_Add(segment, segmentList);

  return 0;
}



int _signSegments(AQFINTS_SESSION *sess,
		  const AQFINTS_CRYPTPARAMS *cryptParams,
		  const AQFINTS_KEYDESCR *keyDescr,
		  AQFINTS_SEGMENT *segSigHead,
		  AQFINTS_SEGMENT *segFirstToSign,
		  AQFINTS_SEGMENT *segLastToSign,
		  GWEN_BUFFER *sigBuf)
{
  uint32_t signLen;
  int rv;
  GWEN_BUFFER *hashSrcBuffer;

  hashSrcBuffer=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=AQFINTS_Session_SampleDataToHash(segSigHead, segFirstToSign, segLastToSign, hashSrcBuffer);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  signLen=GWEN_Buffer_GetMaxUnsegmentedWrite(sigBuf);
  GWEN_Buffer_AllocRoom(sigBuf, 256);
  rv=AQFINTS_Session_Sign(sess, keyDescr, cryptParams,
                          (const uint8_t*) GWEN_Buffer_GetStart(hashSrcBuffer),
			  GWEN_Buffer_GetUsedBytes(hashSrcBuffer),
			  (uint8_t *)GWEN_Buffer_GetStart(sigBuf),
			  &signLen);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hashSrcBuffer);
    return rv;
  }
  GWEN_Buffer_free(hashSrcBuffer);
  GWEN_Buffer_IncrementPos(sigBuf, signLen);
  GWEN_Buffer_AdjustUsedBytes(sigBuf);

  return 0;
}





AQFINTS_SEGMENT *_createSigHead(AQFINTS_SESSION *sess,
                                const AQFINTS_CRYPTPARAMS *cryptParams,
                                int segNum, const AQFINTS_KEYDESCR *keyDescr, const char *ctrlRef)
{
  GWEN_DB_NODE *dbSegment;
  AQFINTS_PARSER *parser;
  int hbciVersion;
  AQFINTS_SEGMENT *defSegment;
  AQFINTS_SEGMENT *segment;
  int rv;

  parser=AQFINTS_Session_GetParser(sess);
  hbciVersion=AQFINTS_Session_GetHbciVersion(sess);

  /* HNSHA */
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HNSHK", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No matching definition segment found for HNSHK (proto=%d)", hbciVersion);
    return NULL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  AQFINTS_Segment_SetSegmentNumber(segment, segNum);
  dbSegment=GWEN_DB_Group_new("sigHead");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  rv=_prepareSignSeg(sess, cryptParams, keyDescr, ctrlRef, dbSegment);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    AQFINTS_Segment_free(segment);
    return NULL;
  }

  rv=AQFINTS_Session_WriteSegment(sess, segment);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    AQFINTS_Segment_free(segment);
  }

  return segment;
}



AQFINTS_SEGMENT *_createSigTail(AQFINTS_SESSION *sess,
				const AQFINTS_CRYPTPARAMS *cryptParams,
				int segNum,
                                const AQFINTS_KEYDESCR *keyDescr,
                                const char *ctrlRef,
                                const uint8_t *ptrSignature, uint32_t lenSignature)
{
  GWEN_DB_NODE *dbSegment;
  AQFINTS_PARSER *parser;
  int hbciVersion;
  AQFINTS_SEGMENT *defSegment;
  AQFINTS_SEGMENT *segment;
  int rv;
  const char *s;

  parser=AQFINTS_Session_GetParser(sess);
  hbciVersion=AQFINTS_Session_GetHbciVersion(sess);

  /* HNSHA */
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HNSHA", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No matching definition segment found for HNSHA (proto=%d)", hbciVersion);
    return NULL;
  }

  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  AQFINTS_Segment_SetSegmentNumber(segment, segNum);

  dbSegment=GWEN_DB_Group_new("sigTail");
  AQFINTS_Segment_SetDbData(segment, dbSegment);

  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_DEFAULT, "ctrlref", ctrlRef);

  if (ptrSignature && lenSignature)
    GWEN_DB_SetBinValue(dbSegment, GWEN_DB_FLAGS_DEFAULT, "signature", ptrSignature, lenSignature);

  s=AQFINTS_KeyDescr_GetPin(keyDescr);
  if (s && *s)
    GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_DEFAULT, "pin", s);

  s=AQFINTS_KeyDescr_GetTan(keyDescr);
  if (s && *s)
    GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_DEFAULT, "tan", s);

  rv=AQFINTS_Session_WriteSegment(sess, segment);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    AQFINTS_Segment_free(segment);
  }

  return segment;
}





int _createCtrlRef(char *ptrBuf, size_t lenBuf)
{

  struct tm *lt;
  time_t tt;

  tt=time(0);
  lt=localtime(&tt);

  if (!strftime(ptrBuf, lenBuf, "%Y%m%d%H%M%S", lt)) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "CtrlRef string too long");
    return GWEN_ERROR_INTERNAL;
  }

  return 0;
}



int _prepareSignSeg(AQFINTS_SESSION *sess,
		    const AQFINTS_CRYPTPARAMS *cryptParams,
		    const AQFINTS_KEYDESCR *keyDescr,
                    const char *ctrlRef,
                    GWEN_DB_NODE *cfg)
{
  char sdate[9];
  char stime[7];
  struct tm *lt;
  time_t tt;
  const char *s;
  const char *securityProfileName;
  int securityProfileVersion;

  /* some preparations */
  tt=time(0);
  lt=localtime(&tt);

  /* create date */
  if (!strftime(sdate, sizeof(sdate), "%Y%m%d", lt)) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Date string too long");
    return GWEN_ERROR_INTERNAL;
  }
  /* create time */
  if (!strftime(stime, sizeof(stime), "%H%M%S", lt)) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Date string too long");
    return GWEN_ERROR_INTERNAL;
  }

  /* store info */

  /* security profile */
  securityProfileName=AQFINTS_KeyDescr_GetSecurityProfileName(keyDescr);
  securityProfileVersion=AQFINTS_KeyDescr_GetSecurityProfileVersion(keyDescr);

  /* hack for hibiscus */
  if (securityProfileVersion==0) {
    if (securityProfileName && strcasecmp(securityProfileName, "RDH")==0)
      securityProfileVersion=10;
  }

  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/code", securityProfileName);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "secProfile/version", securityProfileVersion);

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "function", 2); /* sign with digital signature key */
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "ctrlref", ctrlRef);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "area", 1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "role", 1);

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/id",
                      AQFINTS_Session_GetIsServer(sess)?2:1); /* 1 client, 2=server */

  s=AQFINTS_KeyDescr_GetSystemId(keyDescr);
  if (s && *s)
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/secId", s);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signseq", AQFINTS_KeyDescr_GetSignatureCounter(keyDescr));

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/stampCode", 1);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/date", sdate);
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecStamp/time", stime);

  /* hashAlgo */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "hashAlgo/purpose", 1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "hashAlgo/algo", AQFINTS_CryptParams_GetHashAlgoSign(cryptParams));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "hashAlgo/pname", 1);

  /* signAlgo */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signAlgo/purpose", 6);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signAlgo/algo", AQFINTS_CryptParams_GetSignAlgo(cryptParams));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "signAlgo/mode", AQFINTS_CryptParams_GetOpModeSign(cryptParams));

  /* keyname */
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/country", AQFINTS_KeyDescr_GetCountry(keyDescr));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/bankcode", AQFINTS_KeyDescr_GetBankCode(keyDescr));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/userid", AQFINTS_KeyDescr_GetUserId(keyDescr));
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keytype", AQFINTS_KeyDescr_GetKeyType(keyDescr));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keynum", AQFINTS_KeyDescr_GetKeyNumber(keyDescr));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "key/keyversion", AQFINTS_KeyDescr_GetKeyVersion(keyDescr));

  /* TODO: add certificate data */

  return 0;
}



