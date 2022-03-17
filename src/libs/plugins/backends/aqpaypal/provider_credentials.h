/***************************************************************************
    begin       : Sat Dec 01 2018
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_PROVIDER_CREDENTIALS_H
#define AQPAYPAL_PROVIDER_CREDENTIALS_H


#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>



int APY_Provider_ReadUserApiSecrets(AB_PROVIDER *pro, const AB_USER *u, GWEN_BUFFER *secbuf);
int APY_Provider_WriteUserApiSecrets(AB_PROVIDER *pro, const AB_USER *u, const char *sec);



#endif


