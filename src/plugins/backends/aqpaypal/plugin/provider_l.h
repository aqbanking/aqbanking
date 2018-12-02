/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_PROVIDER_L_H
#define AQPAYPAL_PROVIDER_L_H


#include "provider.h"


AQPAYPAL_API AB_PROVIDER *APY_Provider_new(AB_BANKING *ab);

int APY_Provider_ReadUserApiSecrets(AB_PROVIDER *pro, const AB_USER *u, GWEN_BUFFER *secbuf);
int APY_Provider_WriteUserApiSecrets(AB_PROVIDER *pro, const AB_USER *u, const char *sec);


int APY_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);
int APY_Provider_DeleteAccount(AB_PROVIDER *pro, uint32_t uid);




#endif



