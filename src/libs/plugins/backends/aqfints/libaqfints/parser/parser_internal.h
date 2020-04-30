/***************************************************************************
 begin       : Sun Jul 07 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_PARSER_INTERNAL_H
#define AQFINTS_PARSER_INTERNAL_H

#include "parser/element.h"
#include "parser/segment.h"


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

