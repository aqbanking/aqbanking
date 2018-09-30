/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_ACCOUNT_BE_H
#define AQBANKING_ACCOUNT_BE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aqbanking/account.h>


AQBANKING_API AB_ACCOUNT *AB_Account_new(AB_BANKING *ab, AB_PROVIDER *pro);
AQBANKING_API void AB_Account_Attach(AB_ACCOUNT *acc);


AQBANKING_API int AB_Account_toDb(const AB_ACCOUNT *a, GWEN_DB_NODE *db);
AQBANKING_API int AB_Account_ReadDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);


/**
 * Frees a List2 of accounts and all its members.
 * This MUST NOT be used on account lists returned by AqBanking, but only
 * on account lists created by backends.
 * Therefore this function is only defined here.
 */
AQBANKING_API 
void AB_Account_List2_FreeAll(AB_ACCOUNT_LIST2 *al);


#ifdef __cplusplus
}
#endif


#endif

