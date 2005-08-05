/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_CUSTOMER_L_H
#define AH_CUSTOMER_L_H

#include <aqhbci/customer.h>

GWEN_LIST_FUNCTION_DEFS(AH_CUSTOMER, AH_Customer);


AH_CUSTOMER *AH_Customer_fromDb(AH_USER *u, GWEN_DB_NODE *db);
int AH_Customer_toDb(const AH_CUSTOMER *cu, GWEN_DB_NODE *db);


#endif /* AH_CUSTOMER_L_H */


