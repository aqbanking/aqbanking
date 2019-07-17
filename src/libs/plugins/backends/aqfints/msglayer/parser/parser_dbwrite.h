/***************************************************************************
 begin       : Wed Jul 17 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_PARSER_DBWRITE_H
#define AQFINTS_PARSER_DBWRITE_H

#include "msglayer/parser/element.h"
#include "msglayer/parser/segment.h"

#include <gwenhywfar/db.h>


int AQFINTS_Parser_Db_WriteSegment(AQFINTS_SEGMENT *segmentDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db);


#endif

