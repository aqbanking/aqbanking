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
			  AB_ACCOUNT *a,
			  AB_IMEXPORTER_CONTEXT *ctx,
			  const uint8_t *dataPtr,
			  uint32_t dataLen,
			  int withProgress, int nounmount, int doLock);

AQHBCI_API
int AH_Provider_WriteValueToDb(const AB_VALUE *v, GWEN_DB_NODE *dbV);


/**
 * Begin exclusively using the given account.
 * This function locks the configuration for the given account, reads the configuration and
 * leaves the configuration locked upon return.
 * Therefore you MUST call @ref AH_Provider_EndExclUseAccount() to unlock it later.
 */
int AH_Provider_BeginExclUseAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);


/**
 * End exclusive use of the given account.
 * This function writes the still locked configuration of the account and unlocks it afterwards.
 *
 * @param pro pointer to provider object
 * @param a pointer to account
 * @param abandon if !=0 the configuration is just unlocked, not written
 */
int AH_Provider_EndExclUseAccount(AB_PROVIDER *pro, AB_ACCOUNT *a, int abandon);


/**
 * Begin exclusively using the given user.
 * This function locks the configuration for the given user, reads the configuration and
 * leaves the configuration locked upon return.
 * Therefore you MUST call @ref AH_Provider_EndExclUseUser() to unlock it later.
 */
int AH_Provider_BeginExclUseUser(AB_PROVIDER *pro, AB_USER *u);


/**
 * End exclusive use of the given user.
 * This function writes the still locked configuration of the user and unlocks it afterwards.
 *
 * @param pro pointer to provider object
 * @param u pointer to user
 * @param abandon if !=0 the configuration is just unlocked, not written
 */
int AH_Provider_EndExclUseUser(AB_PROVIDER *pro, AB_USER *u, int abandon);


#endif

