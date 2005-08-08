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

#ifndef AD_ACCOUNT_H
#define AD_ACCOUNT_H

#include <aqbanking/account_be.h>
#include <aqdtaus/provider.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct AD_ACCOUNT AD_ACCOUNT;


AB_ACCOUNT *AD_Account_new(AB_BANKING *ab,
                           AB_PROVIDER *pro,
                           const char *idForProvider);


AB_ACCOUNT *AD_Account_fromDb(AB_BANKING *ab,
                              GWEN_DB_NODE *db);

int AD_Account_toDb(const AB_ACCOUNT *acc, GWEN_DB_NODE *db);


int AD_Account_GetMaxTransfersPerJob(const AB_ACCOUNT *acc);
void AD_Account_SetMaxTransfersPerJob(AB_ACCOUNT *acc, int i);

int AD_Account_GetMaxPurposeLines(const AB_ACCOUNT *acc);
void AD_Account_SetMaxPurposeLines(AB_ACCOUNT *acc, int i);

int AD_Account_GetDebitAllowed(const AB_ACCOUNT *acc);
void AD_Account_SetDebitAllowed(AB_ACCOUNT *acc, int i);

int AD_Account_GetMountAllowed(const AB_ACCOUNT *acc);
void AD_Account_SetMountAllowed(AB_ACCOUNT *acc, int i);

const char *AD_Account_GetMountCommand(const AB_ACCOUNT *acc);
void AD_Account_SetMountCommand(AB_ACCOUNT *acc, const char *s);

const char *AD_Account_GetUnmountCommand(const AB_ACCOUNT *acc);
void AD_Account_SetUnmountCommand(AB_ACCOUNT *acc, const char *s);


const char *AD_Account_GetFolder(const AB_ACCOUNT *acc);
void AD_Account_SetFolder(AB_ACCOUNT *acc, const char *s);

int AD_Account_GetUseDisc(const AB_ACCOUNT *acc);
void AD_Account_SetUseDisc(AB_ACCOUNT *acc, int i);

int AD_Account_GetPrintAllTransactions(const AB_ACCOUNT *acc);
void AD_Account_SetPrintAllTransactions(AB_ACCOUNT *acc, int b);







#ifdef __cplusplus
}
#endif


#endif
