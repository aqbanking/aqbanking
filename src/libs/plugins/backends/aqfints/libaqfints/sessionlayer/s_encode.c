/***************************************************************************
 begin       : Sat Aug 03 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "sessionlayer/s_encode.h"
#include "sessionlayer/session.h"
#include "sessionlayer/s_encrypt.h"
#include "sessionlayer/s_sign.h"
#include "sessionlayer/s_message.h"

#include "parser/parser.h"

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



GWEN_BUFFER *AQFINTS_Session_EncodeMessage(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *message)
{
  AQFINTS_SEGMENT_LIST *segmentList;
  GWEN_BUFFER *msgBuffer;
  int lastSegNum;
  uint32_t flags;
  int rv;
  uint32_t messageSize;

  segmentList=AQFINTS_Message_GetSegmentList(message);
  assert(segmentList);

  flags=AQFINTS_Segment_List_SampleFlags(segmentList);

  AQFINTS_Message_Reenumerate(message);
  AQFINTS_Message_WriteSegments(message);

  if (flags & AQFINTS_SEGMENT_FLAGS_SIGN) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Signing requested");
    rv=AQFINTS_Session_SignMessage(sess, message);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      return NULL;
    }
  }
  lastSegNum=AQFINTS_Message_GetLastSegNum(message);

  if (flags & AQFINTS_SEGMENT_FLAGS_CRYPT) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Encryption requested");
    rv=AQFINTS_Session_EncryptMessage(sess, message);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      return NULL;
    }
  }

  rv=AQFINTS_Session_WrapMessageHeadAndTail(sess, segmentList,
                                            AQFINTS_Message_GetMessageNumber(message),
                                            AQFINTS_Message_GetRefMessageNumber(message),
                                            lastSegNum);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return NULL;
  }

  messageSize=AQFINTS_Segment_List_SampleSizes(segmentList);
  msgBuffer=GWEN_Buffer_new(0, messageSize, 0, 1);
  AQFINTS_Segment_List_SampleBuffers(segmentList, msgBuffer);
  return msgBuffer;
}



