/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AO_N_STATEMENT_H
#define AO_N_STATEMENT_H


/* plugin headers */
#include <aqofxconnect/aqofxconnect.h>

/* aqbanking headers */
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/backendsupport/account.h>
#include <aqbanking/types/transaction.h>

/* gwenhywfar headers */
#include <gwenhywfar/xml.h>



GWEN_XMLNODE *AO_V2_MkStatementRqNode(AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *j);



#endif


