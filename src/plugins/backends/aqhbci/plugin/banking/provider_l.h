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
 * Read an account from the configuration database.
 * When reading the account object it will be locked and/or unlocked as requestd.
 * If both the parameters doLock and doUnlock are !=0 you can later call @ref AH_Provider_EndExclUseAccount on the
 * account object returned (if any).
 *
 * @param pro pointer to provider object
 * @param uid unique id of the account to read
 * @param doLock if !0 0 the config group for the given object will be locked before reading
 * @param doUnlock if !0 0 the config group for the given object will be unlocked after reading
 * @param account pointer to a variable to receive the data read
 */
AQHBCI_API int AH_Provider_ReadAccount(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, AB_ACCOUNT *account);


/*
 * Write an account to the configuration database.
 * When writing the account object it will be locked and/or unlocked as requested.
 * If both the parameters doLock and doUnlock are !=0 you can later call @ref AH_Provider_EndExclUseAccount on the
 * account object returned (if any).
 *
 * @param pro pointer to provider object
 * @param uid unique id of the account to read
 * @param doLock if !0 0 the config group for the given object will be locked before reading
 * @param doUnlock if !0 0 the config group for the given object will be unlocked after reading
 * @param account pointer to the account whose data is to be written
 */
AQHBCI_API int AH_Provider_WriteAccount(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, const AB_ACCOUNT *account);


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

