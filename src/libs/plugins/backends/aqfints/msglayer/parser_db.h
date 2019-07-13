/***************************************************************************
 begin       : Sun Jul 07 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_PARSER_DB_H
#define AQFINTS_PARSER_DB_H


#include "msglayer/element.h"
#include "msglayer/segment.h"


int AQFINTS_Parser_Db_ReadSegment(AQFINTS_SEGMENT *definitionSegment, AQFINTS_SEGMENT *dataSegment, GWEN_DB_NODE *db);


#endif

