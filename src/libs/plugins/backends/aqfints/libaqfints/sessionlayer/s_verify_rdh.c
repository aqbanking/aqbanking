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

#include "sessionlayer/s_verify_rdh.h"
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


int AQFINTS_Session_VerifySegmentsRdh(AQFINTS_SESSION *sess,
                                      AQFINTS_SEGMENT *segSigHead,
                                      AQFINTS_SEGMENT *segSigTail,
                                      AQFINTS_SEGMENT *segFirstSigned,
                                      AQFINTS_SEGMENT *segLastSigned,
                                      int secProfileVersion,
                                      const AQFINTS_KEYDESCR *keyDescr)
{
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



