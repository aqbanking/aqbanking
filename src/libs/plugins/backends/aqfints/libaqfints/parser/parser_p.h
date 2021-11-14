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


#include "libaqfints/parser/parser.h"

#include <gwenhywfar/stringlist.h>



struct AQFINTS_PARSER {
  AQFINTS_JOBDEF_LIST *jobDefList;
  AQFINTS_SEGMENT_LIST *segmentList;
  GWEN_STRINGLIST *pathList;
};


#endif

