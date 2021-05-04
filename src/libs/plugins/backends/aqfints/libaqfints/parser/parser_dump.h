/***************************************************************************
 begin       : Fri Jun 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_PARSER_DUMP_H
#define AQFINTS_PARSER_DUMP_H


#include "libaqfints/parser/segment.h"


void AQFINTS_Parser_DumpElementTree(AQFINTS_ELEMENT *element, int indent);

void AQFINTS_Parser_DumpSegment(AQFINTS_SEGMENT *segment, int indent);

void AQFINTS_Parser_DumpSegmentList(AQFINTS_SEGMENT_LIST *segmentList, int indent);


void AQFINTS_Parser_DumpContext(AQFINTS_ELEMENT *elementDef,
                                AQFINTS_ELEMENT *elementData,
                                GWEN_DB_NODE *dbData,
                                int indent);


#endif

