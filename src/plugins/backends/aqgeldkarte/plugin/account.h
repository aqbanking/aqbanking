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

#ifndef AG_ACCOUNT_H
#define AG_ACCOUNT_H

#include <aqbanking/account_be.h>
#include <aqgeldkarte/provider.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct AG_ACCOUNT AG_ACCOUNT;


AB_ACCOUNT *AG_Account_new(AB_BANKING *ab,
                           AB_PROVIDER *pro,
                           const char *idForProvider);


AB_ACCOUNT *AG_Account_fromDb(AB_BANKING *ab,
                              GWEN_DB_NODE *db);

int AG_Account_toDb(const AB_ACCOUNT *acc, GWEN_DB_NODE *db);


const char *AG_Account_GetCardId(const AB_ACCOUNT *acc);
void AG_Account_SetCardId(AB_ACCOUNT *acc, const char *s);







#ifdef __cplusplus
}
#endif


#endif
