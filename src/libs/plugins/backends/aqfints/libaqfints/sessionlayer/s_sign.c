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

#include "sessionlayer/s_sign.h"
#include "sessionlayer/pintan/s_sign_pintan.h"
#include "sessionlayer/hbci/s_sign_hbci.h"
#include "sessionlayer/s_decode.h"
#include "parser/parser.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _signSegment(AQFINTS_SESSION *sess,
                        AQFINTS_MESSAGE *message,
                        AQFINTS_KEYDESCR *keyDescr,
                        AQFINTS_SEGMENT *segFirstToSign,
                        AQFINTS_SEGMENT *segLastToSign,
                        int sigHeadNum,
                        int sigTailNum);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Session_SignMessage(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *message)
{
  AQFINTS_KEYDESCR_LIST *signerKeyDescrList;

  signerKeyDescrList=AQFINTS_Message_GetSignerList(message);
  if (signerKeyDescrList) {
    AQFINTS_SEGMENT_LIST *segmentList;
    AQFINTS_SEGMENT *segFirstToSign;
    AQFINTS_SEGMENT *segLastToSign;
    AQFINTS_SEGMENT *segment;
    AQFINTS_KEYDESCR *keyDescr;
    int sigHeadNum;
    int sigTailNum;

    segmentList=AQFINTS_Message_GetSegmentList(message);
    segFirstToSign=AQFINTS_Segment_List_First(segmentList);
    segLastToSign=AQFINTS_Segment_List_Last(segmentList);

    sigHeadNum=AQFINTS_Segment_GetSegmentNumber(segFirstToSign)-1;
    sigTailNum=AQFINTS_Segment_GetSegmentNumber(segLastToSign)+1;

    keyDescr=AQFINTS_KeyDescr_List_First(signerKeyDescrList);
    while (keyDescr) {
      const char *sUserId;
      int rv;

      sUserId=AQFINTS_KeyDescr_GetUserId(keyDescr);

      DBG_INFO(AQFINTS_LOGDOMAIN, "User [%s]: Filling out keyname", sUserId?sUserId:"<empty>");
      rv=AQFINTS_Session_FilloutKeyname(sess, keyDescr, AQFINTS_SESSION_CRYPTOP_SIGN);
      if (rv<0) {
        DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }

      rv=_signSegment(sess, message, keyDescr, segFirstToSign, segLastToSign, sigHeadNum, sigTailNum);
      if (rv<0) {
        DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }

      sigHeadNum--;
      sigTailNum++;
      keyDescr=AQFINTS_KeyDescr_List_Next(keyDescr);
    }

    /* mark segments as signed */
    segment=segFirstToSign;
    while(segment) {
      AQFINTS_Segment_AddRuntimeFlags(segment, AQFINTS_SEGMENT_RTFLAGS_SIGNED);
      if (segment==segLastToSign)
        break;
      segment=AQFINTS_Segment_List_Next(segment);
    }
  }

  /* done */
  return 0;
}



int _signSegment(AQFINTS_SESSION *sess,
                 AQFINTS_MESSAGE *message,
                 AQFINTS_KEYDESCR *keyDescr,
                 AQFINTS_SEGMENT *segFirstToSign,
                 AQFINTS_SEGMENT *segLastToSign,
                 int sigHeadNum,
                 int sigTailNum)
{
  const char *sSecProfileCode;

  sSecProfileCode=AQFINTS_KeyDescr_GetSecurityProfileName(keyDescr);
  if (sSecProfileCode && *sSecProfileCode) {
    int rv;

    if (strcasecmp(sSecProfileCode, "PIN")==0)
      rv=AQFINTS_Session_SignSegmentPinTan(sess, message, keyDescr, segFirstToSign, segLastToSign, sigHeadNum, sigTailNum);
    else if (strcasecmp(sSecProfileCode, "RDH")==0)
      rv=AQFINTS_Session_SignSegmentHbci(sess, message, keyDescr, segFirstToSign, segLastToSign, sigHeadNum, sigTailNum);
    else if (strcasecmp(sSecProfileCode, "RAH")==0)
      rv=AQFINTS_Session_SignSegmentHbci(sess, message, keyDescr, segFirstToSign, segLastToSign, sigHeadNum, sigTailNum);
    else {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "Unhandled security profile \"%s\"", sSecProfileCode);
      return GWEN_ERROR_GENERIC;
    }

    if (rv<0) {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    return 0;
  }
  else {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No security profile code set in session");
    return GWEN_ERROR_INVALID;
  }
}





