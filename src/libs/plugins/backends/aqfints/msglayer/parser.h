/***************************************************************************
 begin       : Fri Jun 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQFINTS_PARSER_H
#define AQFINTS_PARSER_H


#include "msglayer/element.h"
#include "msglayer/segment.h"

#include <gwenhywfar/xml.h>


typedef struct AQFINTS_PARSER AQFINTS_PARSER;


AQFINTS_PARSER *AQFINTS_Parser_new();

void AQFINTS_Parser_free(AQFINTS_PARSER *parser);


AQFINTS_SEGMENT_LIST *AQFINTS_Parser_GetSegmentList(const AQFINTS_PARSER *parser);
AQFINTS_ELEMENT *AQFINTS_Parser_GetGroupTree(const AQFINTS_PARSER *parser);


AQFINTS_SEGMENT *AQFINTS_Parser_FindSegment(const AQFINTS_PARSER *parser, const char *id, int segmentVersion, int protocolVersion);


AQFINTS_ELEMENT *AQFINTS_Parser_FindGroupInTree(AQFINTS_ELEMENT *groupTree, const char *id, int version);


#endif

