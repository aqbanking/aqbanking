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


#include "libaqfints/session/s_encode.h"
#include "libaqfints/session/session.h"
#include "libaqfints/session/s_encrypt.h"
#include "libaqfints/session/s_sign.h"
#include "libaqfints/session/s_message.h"
#include "libaqfints/parser/parser_dump.h"

#include "libaqfints/parser/parser.h"

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

  AQFINTS_Message_MoveResultSegsToFront(message);
  AQFINTS_Message_Reenumerate(message);

  rv=AQFINTS_Session_WriteSegmentList(sess, segmentList);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return NULL;
  }

#if 0
  if (1) {
    AQFINTS_SEGMENT_LIST *segmentList;

    DBG_ERROR(AQFINTS_LOGDOMAIN, "Segment list before encoding:");
    segmentList=AQFINTS_Message_GetSegmentList(message);
    if (segmentList)
      AQFINTS_Parser_DumpSegmentList(segmentList, 2);
  }
#endif


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
                                            AQFINTS_Message_GetDialogId(message),
                                            AQFINTS_Message_GetMessageNumber(message),
                                            AQFINTS_Message_GetRefMessageNumber(message),
                                            lastSegNum);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return NULL;
  }

#if 0
  if (1) {
    AQFINTS_SEGMENT_LIST *segmentList;

    DBG_ERROR(AQFINTS_LOGDOMAIN, "Segment list after encoding:");
    segmentList=AQFINTS_Message_GetSegmentList(message);
    if (segmentList)
      AQFINTS_Parser_DumpSegmentList(segmentList, 2);
  }
#endif


  messageSize=AQFINTS_Segment_List_SampleSizes(segmentList);
  msgBuffer=GWEN_Buffer_new(0, messageSize, 0, 1);
  AQFINTS_Segment_List_SampleBuffers(segmentList, msgBuffer);

  AQFINTS_Session_LogMessage(sess,
                             (const uint8_t *) GWEN_Buffer_GetStart(msgBuffer),
                             GWEN_Buffer_GetUsedBytes(msgBuffer),
                             0, 1); /* !rec, crypt */


  return msgBuffer;
}



