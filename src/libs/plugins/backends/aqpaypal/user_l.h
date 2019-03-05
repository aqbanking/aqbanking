/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_USER_L_H
#define AQPAYPAL_USER_L_H


#include <aqpaypal/user.h>
#include <aqbanking/backendsupport/provider_be.h>


AB_USER *APY_User_new(AB_PROVIDER *pro);

/* internal function, called from APY_Provider */
void APY_User_SetApiSecrets_l(AB_USER *u, const char *password, const char *signature, const char *userid);

const char *APY_User_GetApiUserId(const AB_USER *u);
const char *APY_User_GetApiPassword(const AB_USER *u);
const char *APY_User_GetApiSignature(const AB_USER *u);



#endif


