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

#ifndef AO_USER_H
#define AO_USER_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/db.h>
#include <aqofxconnect/aqofxconnect.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AO_USER AO_USER;
GWEN_LIST_FUNCTION_LIB_DEFS(AO_USER, AO_User, AQOFXCONNECT_API)

#ifdef __cplusplus
}
#endif

#include <aqofxconnect/bank.h>

#ifdef __cplusplus
extern "C" {
#endif

AO_USER *AO_User_new(AO_BANK *b, const char *userId);
void AO_User_free(AO_USER *u);

const char *AO_User_GetUserId(const AO_USER *u);
void AO_User_SetUserId(AO_USER *u, const char *s);

const char *AO_User_GetUserName(const AO_USER *u);
void AO_User_SetUserName(AO_USER *u, const char *s);

AO_BANK *AO_User_GetBank(const AO_USER *u);
void AO_User_SetBank(AO_USER *u, AO_BANK *b);


#ifdef __cplusplus
}
#endif

#include <aqofxconnect/bank.h>



#ifdef __cplusplus
extern "C" {
#endif

AO_USER *AO_User_new(AO_BANK *b, const char *userId);
void AO_User_free(AO_USER *u);

AO_BANK *AO_User_GetBank(const AO_USER *u);
const char *AO_User_GetUserId(const AO_USER *u);


const char *AO_User_GetUserName(const AO_USER *u);
void AO_User_SetUserName(AO_USER *u, const char *s);


AO_USER *AO_User_fromDb(AO_BANK *b, GWEN_DB_NODE *db);
int AO_User_toDb(const AO_USER *u, GWEN_DB_NODE *db);

#ifdef __cplusplus
}
#endif



#endif
