/***************************************************************************
 begin       : Fri Jun 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_PARSER_XML_H
#define AQFINTS_PARSER_XML_H


#include "msglayer/parser/element.h"
#include "msglayer/parser/segment.h"
#include "msglayer/parser/jobdef.h"

#include <gwenhywfar/xml.h>


int AQFINTS_Parser_Xml_ReadFile(AQFINTS_JOBDEF_LIST *jobDefList,
                                AQFINTS_SEGMENT_LIST *segmentList,
                                AQFINTS_ELEMENT *groupTree,
                                const char *filename);

int AQFINTS_Parser_Xml_ReadBuffer(AQFINTS_JOBDEF_LIST *jobDefList,
                                  AQFINTS_SEGMENT_LIST *segmentList,
                                  AQFINTS_ELEMENT *groupTree,
                                  const char *dataString);

int AQFINTS_Parser_Xml_WriteSegmentDefinitionFile(const AQFINTS_SEGMENT_LIST *segmentList, const char *filename);


#endif

