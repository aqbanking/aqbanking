/***************************************************************************
 begin       : Sat Oct 26 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQFINTS_PROVIDER_P_H
#define AQFINTS_PROVIDER_P_H

#include "provider_l.h"


#define AF_PM_LIBNAME       "aqfints"
#define AF_PM_FINTSDATADIR  "fintsdatadir"



typedef struct AF_PROVIDER AF_PROVIDER;
struct AF_PROVIDER {
  GWEN_DB_NODE *dbConfig;
  uint32_t lastVersion;

  AQFINTS_PARSER *parser;

};




#endif

