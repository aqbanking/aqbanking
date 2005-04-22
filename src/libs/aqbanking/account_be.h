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


#ifndef AQBANKING_ACCOUNT_BE_H
#define AQBANKING_ACCOUNT_BE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <aqbanking/account.h>

GWEN_LIST_FUNCTION_DEFS(AB_ACCOUNT, AB_Account)


/**
 *  This constructor MUST NOT be used by applications. Only backends
 * (see @ref AB_PROVIDER) and AB_BANKING need to create accounts.
 */
AQBANKING_API 
AB_ACCOUNT *AB_Account_new(AB_BANKING *ab,
                           AB_PROVIDER *pro,
                           const char *idForProvider);

/**
 * Frees a List2 of accounts and all its members.
 * This MUST NOT be used on account lists returned by AqBanking, but only
 * on account lists created by backends.
 * Therefore this function is only defined here.
 */
AQBANKING_API 
void AB_Account_List2_FreeAll(AB_ACCOUNT_LIST2 *al);

AQBANKING_API 
AB_ACCOUNT *AB_Account_dup(AB_ACCOUNT *acc);

AQBANKING_API 
AB_ACCOUNT *AB_Account_fromDb(AB_BANKING *ab,
                              GWEN_DB_NODE *db);

AQBANKING_API
int AB_Account_toDb(const AB_ACCOUNT *acc, GWEN_DB_NODE *db);


#ifdef __cplusplus
}
#endif


#endif

