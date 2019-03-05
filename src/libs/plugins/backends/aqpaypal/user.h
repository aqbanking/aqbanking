/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQPAYPAL_USER_H
#define AQPAYPAL_USER_H


#include <aqbanking/backendsupport/user.h>
#include <aqpaypal/aqpaypal.h>



const char *APY_User_GetServerUrl(const AB_USER *u);
void APY_User_SetServerUrl(AB_USER *u, const char *s);

/**
 * This function sets the given secrets and writes a new secrets file.
 */
int APY_User_SetApiSecrets(AB_USER *u, const char *password, const char *signature, const char *userId);


int APY_User_GetHttpVMajor(const AB_USER *u);
void APY_User_SetHttpVMajor(const AB_USER *u, int i);

int APY_User_GetHttpVMinor(const AB_USER *u);
void APY_User_SetHttpVMinor(const AB_USER *u, int i);




#endif


