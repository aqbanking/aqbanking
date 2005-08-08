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

#ifndef AO_BANK_H
#define AO_BANK_H


#include <gwenhywfar/misc.h>
#include <gwenhywfar/db.h>

#include <aqofxconnect/aqofxconnect.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AO_BANK AO_BANK;
GWEN_LIST_FUNCTION_LIB_DEFS(AO_BANK, AO_Bank, AQOFXCONNECT_API)

#ifdef __cplusplus
}
#endif

#include <aqofxconnect/account.h>
#include <aqofxconnect/user.h>
#include <aqbanking/banking.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  AO_Bank_ServerTypeUnknown=0,
  AO_Bank_ServerTypeHTTP,
  AO_Bank_ServerTypeHTTPS
} AO_BANK_SERVERTYPE;


AO_BANK *AO_Bank_new(AB_PROVIDER *pro,
                     const char *country, const char *bankId);
void AO_Bank_free(AO_BANK *b);

AB_PROVIDER *AO_Bank_GetProvider(const AO_BANK *b);

const char *AO_Bank_GetCountry(const AO_BANK *b);
const char *AO_Bank_GetBankId(const AO_BANK *b);

const char *AO_Bank_GetBankName(const AO_BANK *b);
void AO_Bank_SetBankName(AO_BANK *b, const char *s);

const char *AO_Bank_GetBrokerId(const AO_BANK *b);
void AO_Bank_SetBrokerId(AO_BANK *b, const char *s);

const char *AO_Bank_GetOrg(const AO_BANK *b);
void AO_Bank_SetOrg(AO_BANK *b, const char *s);

const char *AO_Bank_GetFid(const AO_BANK *b);
void AO_Bank_SetFid(AO_BANK *b, const char *s);

AO_BANK_SERVERTYPE AO_Bank_GetServerType(const AO_BANK *b);
void AO_Bank_SetServerType(AO_BANK *b, AO_BANK_SERVERTYPE t);

const char *AO_Bank_GetServerAddr(const AO_BANK *b);
void AO_Bank_SetServerAddr(AO_BANK *b, const char *s);

int AO_Bank_GetServerPort(const AO_BANK *b);
void AO_Bank_SetServerPort(AO_BANK *b, int i);

const char *AO_Bank_GetHttpHost(const AO_BANK *b);
void AO_Bank_SetHttpHost(AO_BANK *b, const char *s);

int AO_Bank_GetHttpVMajor(const AO_BANK *b);
void AO_Bank_SetHttpVMajor(AO_BANK *b, int i);

int AO_Bank_GetHttpVMinor(const AO_BANK *b);
void AO_Bank_SetHttpVMinor(AO_BANK *b, int i);

AO_BANK *AO_Bank_fromDb(AB_PROVIDER *pro, GWEN_DB_NODE *db);
int AO_Bank_toDb(const AO_BANK *b, GWEN_DB_NODE *db);

AB_ACCOUNT_LIST *AO_Bank_GetAccounts(const AO_BANK *b);
AB_ACCOUNT *AO_Bank_FindAccount(AO_BANK *b, const char *id);
int AO_Bank_AddAccount(AO_BANK *b, AB_ACCOUNT *a);

AO_USER_LIST *AO_Bank_GetUsers(const AO_BANK *b);
AO_USER *AO_Bank_FindUser(AO_BANK *b, const char *id);
int AO_Bank_AddUser(AO_BANK *b, AO_USER *u);

#ifdef __cplusplus
}
#endif


#endif
