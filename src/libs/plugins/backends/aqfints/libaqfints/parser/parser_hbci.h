/***************************************************************************
 begin       : Fri Jun 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_PARSER_HBCI_H
#define AQFINTS_PARSER_HBCI_H


#include "libaqfints/parser/element.h"
#include "libaqfints/parser/segment.h"



int AQFINTS_Parser_Hbci_ReadBuffer(AQFINTS_SEGMENT_LIST *targetSegmentList,
                                   const uint8_t *ptrBuf,
                                   uint32_t lenBuf);
void AQFINTS_Parser_Hbci_WriteBuffer(AQFINTS_SEGMENT_LIST *segmentList);

void AQFINTS_Parser_Hbci_SampleSegmentBuffers(AQFINTS_SEGMENT_LIST *segmentList, GWEN_BUFFER *destBuf);


void AQFINTS_Parser_Hbci_WriteSegment(AQFINTS_SEGMENT *segment);


#endif

