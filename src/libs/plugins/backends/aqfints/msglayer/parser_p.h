/***************************************************************************
 begin       : Fri Jun 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQFINTS_PARSER_P_H
#define AQFINTS_PARSER_P_H


#include "msglayer/parser.h"



struct AQFINTS_PARSER {
  AQFINTS_SEGMENT_LIST *segmentList;
  AQFINTS_ELEMENT_TREE *groupTree;
};


#endif

