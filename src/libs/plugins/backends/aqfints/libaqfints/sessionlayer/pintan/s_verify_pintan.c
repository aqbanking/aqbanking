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

#include "sessionlayer/pintan/s_verify_pintan.h"
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

  dbSigTail=AQFINTS_Segment_GetDbData(segSigTail);
  assert(dbSigTail);

  sPin=GWEN_DB_GetCharValue(dbSigTail, "pin", 0, NULL);

  rv=AQFINTS_Session_VerifyPin(sess, keyDescr, sPin);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



