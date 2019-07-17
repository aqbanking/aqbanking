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


#include "msglayer/parser/element.h"
#include "msglayer/parser/segment.h"

#include <gwenhywfar/xml.h>


typedef struct AQFINTS_PARSER AQFINTS_PARSER;



AQFINTS_PARSER *AQFINTS_Parser_new();

void AQFINTS_Parser_free(AQFINTS_PARSER *parser);


AQFINTS_SEGMENT *AQFINTS_Parser_FindSegment(const AQFINTS_PARSER *parser, const char *id, int segmentVersion, int protocolVersion);


int AQFINTS_Parser_ReadIntoDb(AQFINTS_PARSER *parser,
                              const uint8_t *ptrBuf,
                              uint32_t lenBuf,
                              GWEN_DB_NODE *db);


int AQFINTS_Parser_ReadIntoSegmentList(AQFINTS_PARSER *parser,
                                       AQFINTS_SEGMENT_LIST *targetSegmentList,
                                       const uint8_t *ptrBuf,
                                       uint32_t lenBuf);

int AQFINTS_Parser_ReadSegmentListToDb(AQFINTS_PARSER *parser,
                                       AQFINTS_SEGMENT_LIST *segmentList,
                                       GWEN_DB_NODE *db);



void AQFINTS_Parser_AddPath(AQFINTS_PARSER *parser, const char *path);
int AQFINTS_Parser_ReadFiles(AQFINTS_PARSER *parser);



AQFINTS_SEGMENT_LIST *AQFINTS_Parser_GetSegmentList(const AQFINTS_PARSER *parser);
AQFINTS_ELEMENT *AQFINTS_Parser_GetGroupTree(const AQFINTS_PARSER *parser);


AQFINTS_ELEMENT *AQFINTS_Parser_FindGroupInTree(AQFINTS_ELEMENT *groupTree, const char *id, int version);


/**
 * @return 1 if sType referes to a char type, 0 otherwise
 */
int AQFINTS_Parser_IsCharType(const char *sType);

/**
 * @return 1 if sType referes to an int type, 0 otherwise
 */
int AQFINTS_Parser_IsIntType(const char *sType);

/**
 * @return 1 if sType referes to a binary type, 0 otherwise
 */
int AQFINTS_Parser_IsBinType(const char *sType);


#endif

