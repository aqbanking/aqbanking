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

#include "libaqfints/session/pintan/s_verify_pintan.h"
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


int AQFINTS_Session_VerifySegmentsPinTan(AQFINTS_SESSION *sess,
                                         AQFINTS_SEGMENT *segSigHead,
                                         AQFINTS_SEGMENT *segSigTail,
                                         AQFINTS_SEGMENT *segFirstSigned,
                                         AQFINTS_SEGMENT *segLastSigned,
                                         int secProfileVersion,
                                         const AQFINTS_KEYDESCR *keyDescr)
{
  GWEN_DB_NODE *dbSigTail;
  const char *sPin;
  int rv;
  AQFINTS_SEGMENT *segment;

  dbSigTail=AQFINTS_Segment_GetDbData(segSigTail);
  assert(dbSigTail);

  sPin=GWEN_DB_GetCharValue(dbSigTail, "pin", 0, NULL);

  if (AQFINTS_Session_GetIsServer(sess)) {
    rv=AQFINTS_Session_VerifyPin(sess, keyDescr, sPin);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  segment=segFirstSigned;
  while (segment) {
    AQFINTS_Segment_AddRuntimeFlags(segment, AQFINTS_SEGMENT_RTFLAGS_SIGNED);
    if (segment==segLastSigned)
      break;
    segment=AQFINTS_Segment_List_Next(segment);
  }

  return 0;
}



