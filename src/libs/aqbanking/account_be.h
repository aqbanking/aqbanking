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


/**
 *  This constructor MUST NOT be used by applications. Only backends
 * (see @ref AB_PROVIDER) and AB_BANKING need to create accounts.
 */
AQBANKING_API 
AB_ACCOUNT *AB_Account_new(AB_BANKING *ab,
                           AB_PROVIDER *pro,
                           const char *idForProvider);

AQBANKING_API 
AB_ACCOUNT *AB_Account_fromDb(AB_BANKING *ab,
                              GWEN_DB_NODE *db);
AQBANKING_API 
int AB_Account_toDb(const AB_ACCOUNT *acc, GWEN_DB_NODE *db);

#ifdef __cplusplus
}
#endif


#endif

