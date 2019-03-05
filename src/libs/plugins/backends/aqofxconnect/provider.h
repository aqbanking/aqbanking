/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AO_PROVIDER_H
#define AO_PROVIDER_H

#include <aqbanking/banking_be.h>
#include <aqbanking/backendsupport/provider_be.h>

#define AQOFXCONNECT_BACKENDNAME "aqofxconnect"

#define AQOFXCONNECT_LOGDOMAIN "aqofxconnect"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AO_PROVIDER AO_PROVIDER;


typedef struct {
  const char *appName;
  const char *appId;
  const char *appVer;
} AO_APPINFO;


AB_PROVIDER *AO_Provider_new(AB_BANKING *ab);

const AO_APPINFO *AO_Provider_GetAppInfos(AB_PROVIDER *pro);

int AO_Provider_GetCert(AB_PROVIDER *pro, AB_USER *u);

int AO_Provider_RequestAccounts(AB_PROVIDER *pro, AB_USER *u, int keepOpen);


#ifdef __cplusplus
}
#endif


#endif

