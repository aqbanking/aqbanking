/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AN_PROVIDER_P_H
#define AN_PROVIDER_P_H

#include "provider_l.h"

int AN_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AN_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);

#endif

