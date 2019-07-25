/***************************************************************************
 begin       : Fri Jul 19 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "message_p.h"

#include "parser/parser_xml.h"
#include "parser/parser_hbci.h"
#include "parser/parser_normalize.h"
#include "parser/parser_dbwrite.h"

#include <gwenhywfar/debug.h>




AQFINTS_MESSAGE *AQFINTS_Message_new(AQFINTS_PARSER *parser)
{
  AQFINTS_MESSAGE *msg;

  GWEN_NEW_OBJECT(AQFINTS_MESSAGE, msg);
  msg->parser=parser;
  msg->buffer=GWEN_Buffer_new(0, 256, 0, 1);
  return msg;
}



void AQFINTS_Message_free(AQFINTS_MESSAGE *msg)
{
  if (msg) {
    if (msg->buffer) {
      GWEN_Buffer_free(msg->buffer);
      msg->buffer=NULL;
    }
    GWEN_FREE_OBJECT(msg);
  }
}



GWEN_BUFFER *AQFINTS_Message_GetBuffer(AQFINTS_MESSAGE *msg)
{
  assert(msg);
  return msg->buffer;
}



GWEN_BUFFER *AQFINTS_Message_TakeBuffer(AQFINTS_MESSAGE *msg)
{
  assert(msg);
  if (msg->buffer) {
    GWEN_BUFFER *buffer;

    buffer=msg->buffer;
    msg->buffer=GWEN_Buffer_new(0, 256, 0, 1);
    return buffer;
  }
  return NULL;
}



int AQFINTS_Message_AddSegment(AQFINTS_MESSAGE *msg,
                               AQFINTS_SEGMENT *defSegment,
                               GWEN_DB_NODE *dbData)
{
  AQFINTS_SEGMENT *segmentOut;
  int rv;

  segmentOut=AQFINTS_Segment_new();
  rv=AQFINTS_Parser_Db_WriteSegment(defSegment, segmentOut, dbData);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    AQFINTS_Segment_free(segmentOut);
    return rv;
  }
  AQFINTS_Parser_Segment_RemoveTrailingEmptyElements(segmentOut);

  AQFINTS_Parser_Hbci_WriteSegment(segmentOut, msg->buffer);

  AQFINTS_Segment_free(segmentOut);
  return 0;
}





