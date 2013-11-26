/***************************************************************************
    begin       : Wed May 07 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef EBC_CLIENT_ACCOUNT_L_H
#define EBC_CLIENT_ACCOUNT_L_H


#include <aqebics/account.h>
#include <aqbanking/provider_be.h>


void EBC_Account_Extend(AB_ACCOUNT *a, AB_PROVIDER *pro,
			AB_PROVIDER_EXTEND_MODE em,
			GWEN_DB_NODE *db);


#endif /* EBC_CLIENT_ACCOUNT_L_H */






