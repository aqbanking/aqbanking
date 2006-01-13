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

#include <aqofxconnect/aqofxconnect.h>
#include <aqbanking/provider_be.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  AO_User_ServerTypeUnknown=0,
  AO_User_ServerTypeHTTP,
  AO_User_ServerTypeHTTPS
} AO_USER_SERVERTYPE;


AQOFXCONNECT_API
AO_USER_SERVERTYPE AO_User_ServerType_fromString(const char *s);

AQOFXCONNECT_API
const char *AO_User_ServerType_toString(AO_USER_SERVERTYPE t);

AQOFXCONNECT_API
void AO_User_Extend(AB_USER *u, AB_PROVIDER *pro);

AQOFXCONNECT_API
const char *AO_User_GetBrokerId(const AB_USER *u);

AQOFXCONNECT_API
void AO_User_SetBrokerId(AB_USER *u, const char *s);

AQOFXCONNECT_API
const char *AO_User_GetOrg(const AB_USER *u);

AQOFXCONNECT_API
void AO_User_SetOrg(AB_USER *u, const char *s);

AQOFXCONNECT_API
const char *AO_User_GetFid(const AB_USER *u);

AQOFXCONNECT_API
void AO_User_SetFid(AB_USER *u, const char *s);

AQOFXCONNECT_API
AO_USER_SERVERTYPE AO_User_GetServerType(const AB_USER *u);

AQOFXCONNECT_API
void AO_User_SetServerType(AB_USER *u, AO_USER_SERVERTYPE t);

AQOFXCONNECT_API
const char *AO_User_GetServerAddr(const AB_USER *u);

AQOFXCONNECT_API
void AO_User_SetServerAddr(AB_USER *u, const char *s);

AQOFXCONNECT_API
int AO_User_GetServerPort(const AB_USER *u);

AQOFXCONNECT_API
void AO_User_SetServerPort(AB_USER *u, int i);

AQOFXCONNECT_API
int AO_User_GetHttpVMajor(const AB_USER *u);

AQOFXCONNECT_API
void AO_User_SetHttpVMajor(AB_USER *u, int i);

AQOFXCONNECT_API
int AO_User_GetHttpVMinor(const AB_USER *u);

AQOFXCONNECT_API
void AO_User_SetHttpVMinor(AB_USER *u, int i);

#ifdef __cplusplus
}
#endif


#endif
