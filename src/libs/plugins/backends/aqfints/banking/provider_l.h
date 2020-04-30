/***************************************************************************
 begin       : Sat Oct 26 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQFINTS_PROVIDER_L_H
#define AQFINTS_PROVIDER_L_H


#include "banking/provider.h"
#include "parser/parser.h"


AQFINTS_PARSER *AF_Provider_GetParser(AB_PROVIDER *pro);

AQFINTS_PARSER *AF_Provider_CreateParser(AB_PROVIDER *pro);




#endif

