/***************************************************************************
 begin       : Fri Jun 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_PARSER_NORMALIZE_H
#define AQFINTS_PARSER_NORMALIZE_H


#include "msglayer/parser/segment.h"

#include <gwenhywfar/xml.h>


void AQFINTS_Parser_SegmentList_ResolveGroups(AQFINTS_SEGMENT_LIST *segmentList, AQFINTS_ELEMENT *groupTree);

void AQFINTS_Parser_SegmentList_Normalize(AQFINTS_SEGMENT_LIST *segmentList);

void AQFINTS_Parser_Segment_RemoveTrailingEmptyElements(AQFINTS_SEGMENT *segment);

#endif


