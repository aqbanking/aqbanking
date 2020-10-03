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

#include "session/hbci/s_verify_hbci.h"
#include "session/s_decode.h"
#include "session/cryptparams.h"
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



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Session_VerifySegmentHbci(AQFINTS_SESSION *sess,
                                      AQFINTS_MESSAGE *message,
                                      const AQFINTS_KEYDESCR *keyDescr,
                                      AQFINTS_SEGMENT *segSigHead,
                                      AQFINTS_SEGMENT *segSigTail,
                                      AQFINTS_SEGMENT *segFirstSigned,
                                      AQFINTS_SEGMENT *segLastSigned)
{
  int rv;
  GWEN_DB_NODE *dbSigHead;
  GWEN_DB_NODE *dbSigTail;
  const uint8_t *ptr;
  unsigned int len=0;
  int sigCounter;
  const char *securityProfileName;
  int securityProfileVersion;
  const AQFINTS_CRYPTPARAMS *cryptParams;
  GWEN_BUFFER *bufHashData;

  securityProfileName=AQFINTS_KeyDescr_GetSecurityProfileName(keyDescr);
  securityProfileVersion=AQFINTS_KeyDescr_GetSecurityProfileVersion(keyDescr);

  /* hack for hibiscus */
  if (securityProfileVersion==0) {
    if (securityProfileName && strcasecmp(securityProfileName, "RDH")==0)
      securityProfileVersion=10;
  }

  cryptParams=AQFINTS_CryptParams_GetParamsForSecurityProfile(securityProfileName, securityProfileVersion);
  if (cryptParams==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No crypt params for [%s:%d]", securityProfileName?securityProfileName:"<empty>",
              securityProfileVersion);
    return GWEN_ERROR_INVALID;
  }

  dbSigTail=AQFINTS_Segment_GetDbData(segSigTail);
  assert(dbSigTail);
  ptr=(const uint8_t *) GWEN_DB_GetBinValue(dbSigTail, "signature", 0, 0, 0, &len);
  if (ptr==NULL || len<1) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No signature in segment");
    return GWEN_ERROR_BAD_DATA;
  }

  dbSigHead=AQFINTS_Segment_GetDbData(segSigHead);
  assert(dbSigHead);
  sigCounter=GWEN_DB_GetIntValue(dbSigHead, "signseq", 0, 0);

  bufHashData=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AQFINTS_Session_SampleDataToHash(segSigHead, segFirstSigned, segLastSigned, bufHashData);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufHashData);
    return rv;
  }

  rv=AQFINTS_Session_Verify(sess, keyDescr, cryptParams,
                            (const uint8_t *) GWEN_Buffer_GetStart(bufHashData), GWEN_Buffer_GetUsedBytes(bufHashData),
                            ptr, len, sigCounter);
  if (rv<0) {
    if (rv==GWEN_ERROR_TRY_AGAIN) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "Not yet ready to verify signature, retry later");
      GWEN_Buffer_free(bufHashData);
    }
    else {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    }
    return rv;
  }

  return 0;
}




