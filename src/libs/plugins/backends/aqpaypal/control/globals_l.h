/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_TOOL_GLOBALS_H
#define AQPAYPAL_TOOL_GLOBALS_H


#include <aqpaypal/aqpaypal.h>

#include <aqbanking/banking.h>
#include <cli/helper.h>
#include <gwenhywfar/db.h>


#include "control_l.h"



int APY_Control_ListUsers(AB_PROVIDER *pro,
                          GWEN_DB_NODE *dbArgs,
                          int argc,
                          char **argv);


int APY_Control_ListAccounts(AB_PROVIDER *pro,
                             GWEN_DB_NODE *dbArgs,
                             int argc,
                             char **argv);

int APY_Control_AddUser(AB_PROVIDER *pro,
                        GWEN_DB_NODE *dbArgs,
                        int argc,
                        char **argv);

int APY_Control_AddAccount(AB_PROVIDER *pro,
                           GWEN_DB_NODE *dbArgs,
                           int argc,
                           char **argv);

int APY_Control_SetSecrets(AB_PROVIDER *pro,
                           GWEN_DB_NODE *dbArgs,
                           int argc,
                           char **argv);


#endif
