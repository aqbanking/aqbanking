/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AO_N_STATEMENT_H
#define AO_N_STATEMENT_H


#include <aqofxconnect/aqofxconnect.h>
#include <aqbanking/backendsupport/provider.h>



GWEN_XMLNODE *AO_Provider_V2_MkStatementRqNode(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j);



#endif


