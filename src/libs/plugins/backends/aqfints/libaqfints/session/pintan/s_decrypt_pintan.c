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

#include "libaqfints/session/pintan/s_decrypt_pintan.h"
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


int AQFINTS_Session_DecryptSegmentPinTan(AQFINTS_SESSION *sess,
                                         AQFINTS_SEGMENT *segCryptHead,
                                         AQFINTS_SEGMENT *segCryptData,
                                         const AQFINTS_CRYPTPARAMS *cryptParams,
                                         const AQFINTS_KEYDESCR *keyDescr,
                                         AQFINTS_SEGMENT_LIST *segmentList)
{
  GWEN_DB_NODE *dbCryptData;
  unsigned int len=0;
  const uint8_t *ptr;

  dbCryptData=AQFINTS_Segment_GetDbData(segCryptData);
  assert(dbCryptData);
  ptr=(const uint8_t *) GWEN_DB_GetBinValue(dbCryptData, "CryptData", 0, NULL, 0, &len);
  if (ptr==NULL || len<1) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No CryptData in segment");
    return GWEN_ERROR_BAD_DATA;
  }
  else {
    AQFINTS_PARSER *parser;
    AQFINTS_SEGMENT_LIST *newSegmentList;
    AQFINTS_SEGMENT *segment;
    int rv;

    parser=AQFINTS_Session_GetParser(sess);
    newSegmentList=AQFINTS_Segment_List_new();
    rv=AQFINTS_Parser_ReadIntoSegmentList(parser, newSegmentList, ptr, len);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      AQFINTS_Segment_List_free(newSegmentList);
      return rv;
    }

    rv=AQFINTS_Parser_ReadSegmentListToDb(parser, newSegmentList);
    if (rv<0) {
      DBG_ERROR(0, "here (%d)", rv);
      AQFINTS_Segment_List_free(newSegmentList);
      return rv;
    }

    /* move new segments to given segment list, delete new segment list */
    while ((segment=AQFINTS_Segment_List_First(newSegmentList))) {
      AQFINTS_Segment_AddRuntimeFlags(segment, AQFINTS_SEGMENT_RTFLAGS_ENCRYPTED);
      AQFINTS_Segment_List_Del(segment);
      AQFINTS_Segment_List_Add(segment, segmentList);
    }
    AQFINTS_Segment_List_free(newSegmentList);
    /* done */
    return 0;
  }
}



