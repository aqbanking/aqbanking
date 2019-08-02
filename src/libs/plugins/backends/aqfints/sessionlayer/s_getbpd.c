/***************************************************************************
 begin       : Thu Aug 01 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "./session.h"

#include <gwenhywfar/debug.h>



//AQFINTS_MESSAGE *mkGetAnonBpdMessage(AQFINTS_SESSION *sess, const char *bankCode)





AQFINTS_MESSAGE *createBasicMessage(AQFINTS_SESSION *sess, const char *bankCode)
{
  AQFINTS_PARSER *parser;
  int hbciVersion;
  AQFINTS_MESSAGE *message;
  AQFINTS_SEGMENT *defSegment;
  AQFINTS_SEGMENT *segment;
  GWEN_DB_NODE *dbSegment;

  parser=AQFINTS_Session_GetParser(sess);
  hbciVersion=AQFINTS_Session_GetHbciVersion(sess);

  message=AQFINTS_Message_new();
  AQFINTS_Message_SetMessageNumber(message, AQFINTS_Session_GetLastMessageNumSent(sess)+1);

  /* ident */
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HKIDN", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(0, "No matching definition segment found for HKIDN (proto=%d)", hbciVersion);
    return NULL;
  }
  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  dbSegment=GWEN_DB_Group_new("ident");
  AQFINTS_Segment_SetDbData(segment, dbSegment);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "country", 280);
  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "bankCode", bankCode);
  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "customerId", "9999999999");
  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "systemId", "0");
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "status", 0);
  AQFINTS_Message_AddSegment(message, segment);

  /* prepare */
  defSegment=AQFINTS_Parser_FindSegmentHighestVersionForProto(parser, "HKVVB", hbciVersion);
  if (defSegment==NULL) {
    DBG_ERROR(0, "No matching definition segment found for HKVVB (proto=%d)", hbciVersion);
    return NULL;
  }
  segment=AQFINTS_Segment_new();
  AQFINTS_Segment_copy(segment, defSegment);
  dbSegment=GWEN_DB_Group_new("ident");
  AQFINTS_Segment_SetDbData(segment, dbSegment);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "bpdVersion", 0);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "updVersion", 0);
  GWEN_DB_SetIntValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "lang", 1);
  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "product", "AqBanking");
  GWEN_DB_SetCharValue(dbSegment, GWEN_DB_FLAGS_OVERWRITE_VARS, "version", "5.99");
  AQFINTS_Message_AddSegment(message, segment);

  return message;
}




