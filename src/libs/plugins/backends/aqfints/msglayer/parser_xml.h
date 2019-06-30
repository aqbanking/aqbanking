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


#include "msglayer/element.h"
#include "msglayer/segment.h"

#include <gwenhywfar/xml.h>


int AQFINTS_Parser_Xml_ReadFile(AQFINTS_SEGMENT_LIST *segmentList,
                                AQFINTS_ELEMENT_TREE *groupTree,
                                const char *filename);


#endif

