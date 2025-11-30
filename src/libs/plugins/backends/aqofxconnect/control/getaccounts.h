/***************************************************************************
 begin       : Thu Jan 16 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AO_CONTROL_GETACCOUNTS_H
#define AO_CONTROL_GETACCOUNTS_H


#include <aqofxconnect/aqofxconnect.h>

#include <cli/helper.h>
#include <aqbanking/backendsupport/provider.h>



int AO_Control_GetAccounts(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv);


#endif

