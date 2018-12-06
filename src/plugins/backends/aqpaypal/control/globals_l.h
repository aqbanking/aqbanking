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
#include <gwenhywfar/db.h>


#include "control_l.h"



int listUsers(AB_PROVIDER *pro,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);


int listAccounts(AB_PROVIDER *pro,
                 GWEN_DB_NODE *dbArgs,
                 int argc,
		 char **argv);

int addUser(AB_PROVIDER *pro,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

int addAccount(AB_PROVIDER *pro,
	       GWEN_DB_NODE *dbArgs,
	       int argc,
	       char **argv);

int setSecrets(AB_PROVIDER *pro,
	       GWEN_DB_NODE *dbArgs,
	       int argc,
	       char **argv);


#endif
