/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_ONLINE_H
#define AH_PROVIDER_ONLINE_H

#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/backendsupport/account.h>

#include "aqhbci/joblayer/job_l.h"
#include "aqhbci/applayer/outbox_l.h"



/** @name Server Interactive Functions
 *
 * Functions in this group are used from setup wizards or tools.
 * They send requests to the server (including opening and closing of the
 * progress dialog by calling @ref AB_Banking_ProgressStart etc).
 */
/*@{*/


/**
 * Anonymously retrieve BPD ("Bankparameterdaten") for the given user.
 * @param pro pointer to the HBCI provider
 * @param u user for which the BPD is to be received
 * @param withTanSeg include "HKTAN:6"-Segment (needed for Strong Customer Authentication)
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
int AH_Provider_GetBankInfo(AB_PROVIDER *pro, AB_USER *u,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            int withTanSeg,
                            int withProgress, int nounmount, int doLock);


/**
 * Retrieve a list of accounts. Not all banks support this. If the bank does
 * then the retrieved accounts are automatically added to AqBanking.
 * @param pro pointer to the HBCI provider
 * @param u user for which the list of accounts is to be received
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
int AH_Provider_GetAccounts(AB_PROVIDER *pro, AB_USER *u,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            int withProgress, int nounmount, int doLock);

/**
 * Retrieve the system id for the given user. This is only needed for
 * PIN/TAN and for RDH mode.
 * @param pro pointer to the HBCI provider
 * @param u user for which the system id is to be received
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
int AH_Provider_GetSysId(AB_PROVIDER *pro, AB_USER *u,
                         AB_IMEXPORTER_CONTEXT *ctx,
                         int withProgress, int nounmount, int doLock);

/**
 * Retrieve the public server keys for the given user. This is only needed for
 * PIN/TAN and for RDH mode.
 * @param pro pointer to the HBCI provider
 * @param u user for which the public keys are to be received
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
int AH_Provider_GetServerKeys(AB_PROVIDER *pro, AB_USER *u,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              int withProgress, int nounmount, int doLock);

/**
 * Send the public keys of the given user. This is only needed for
 * PIN/TAN and for RDH mode.
 * @param pro pointer to the HBCI provider
 * @param u user for which the public keys are to be sent
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
int AH_Provider_SendUserKeys(AB_PROVIDER *pro, AB_USER *u,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             int withProgress, int nounmount, int doLock);

/**
 * Send the public keys of the given user. This is only needed for
 * PIN/TAN and for RDH mode.
 * In addition to @ref AH_Provider_SendUserKeys this function also sends the
 * authentication key if requested.
 * @param pro pointer to the HBCI provider
 * @param u user for which the public keys are to be sent
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
int AH_Provider_SendUserKeys2(AB_PROVIDER *pro, AB_USER *u,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              int withAuthKey,
                              int withProgress, int nounmount, int doLock);

/**
 */
int AH_Provider_ChangeUserKeys(AB_PROVIDER *pro, AB_USER *u, GWEN_DB_NODE *args, int withProgress, int nounmount,
                               int doLock);

/**
 * Retrieve the SSL certificate for the given user. This is only needed for
 * PIN/TAN mode.
 * @param pro pointer to the HBCI provider
 * @param u user for which the SSL certificate is to be received
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
int AH_Provider_GetCert(AB_PROVIDER *pro,
                        AB_USER *u,
                        int withProgress, int nounmount, int doLock);

/**
 * Ask the server for the list of supported iTAN modes. Not all servers
 * support iTAN so it is ok for the server to not report any modes.
 * @param pro pointer to the HBCI provider
 * @param u user for which the list of iTAN modes is to be received
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
int AH_Provider_GetItanModes(AB_PROVIDER *pro, AB_USER *u,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             int withProgress, int nounmount, int doLock);


/**
 * Ask the user for a new PIN and make the server change the PIN to the
 * newly entered one.
 * @param pro pointer to the HBCI provider
 * @param u user for which the pin is to be changed
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
int AH_Provider_ChangePin(AB_PROVIDER *pro, AB_USER *u,
                          AB_IMEXPORTER_CONTEXT *ctx,
                          int withProgress, int nounmount, int doLock);


/**
 * Unblock the PIN in Pin-TAN mode.
 * @param pro pointer to the HBCI provider
 * @param u user for which the pin is to be changed
 * @param ctx im-/exporter context to receive info from the bank
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
int AH_Provider_UnblockPin(AB_PROVIDER *pro,
                           AB_USER *u,
                           AB_IMEXPORTER_CONTEXT *ctx,
                           int withProgress, int nounmount, int doLock);


/**
 * Request SEPA information for a given account. This info is then stored
 * within the AqBanking account information (IBAN, BIC).
 * @param pro pointer to the HBCI provider
 * @param a account for which the info is to be requested
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
int AH_Provider_GetAccountSepaInfo(AB_PROVIDER *pro,
                                   AB_ACCOUNT *a,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   int withProgress, int nounmount, int doLock);

/**
 * Request target accounts for a SEPA internal transfer for a given account. This info is then stored
 * within the AqBanking account information (IBAN, BIC).
 * @param pro pointer to the HBCI provider
 * @param a account for which the info is to be requested
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
int AH_Provider_GetTargetAccount(AB_PROVIDER *pro,
                                   AB_ACCOUNT *a,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   int withProgress, int nounmount, int doLock);

/*@}*/














#endif

