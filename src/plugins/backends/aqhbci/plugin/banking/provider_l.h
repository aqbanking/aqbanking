/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_L_H
#define AH_PROVIDER_L_H

#include "provider.h"
#include "hbci_l.h"


AH_HBCI *AH_Provider_GetHbci(const AB_PROVIDER *pro);


AQHBCI_API
int AH_Provider_SendDtazv(AB_PROVIDER *pro,
                          AB_USER *u,
			  AB_ACCOUNT *a,
			  AB_IMEXPORTER_CONTEXT *ctx,
			  const uint8_t *dataPtr,
			  uint32_t dataLen,
			  int withProgress, int nounmount, int doLock);

AQHBCI_API
int AH_Provider_WriteValueToDb(const AB_VALUE *v, GWEN_DB_NODE *dbV);


/**
 * Create an AB_ACCOUNT_SPEC structure for the given account and write it to the database.
 *
 * If there already is an account spec it will get overwritten.
 *
 * @return 0 on sucess, error code otherwise
 * @param pro pointer to provider object
 * @param u user to which the corresponding account belongs
 * @param acc account for which an AB_ACCOUNT_SPEC object is to be writtem
 *
 */
AQHBCI_API int AH_Provider_WriteAccountSpecForAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *acc);


#endif

