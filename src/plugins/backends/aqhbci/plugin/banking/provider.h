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

#ifndef AH_PROVIDER_H
#define AH_PROVIDER_H


#include <aqhbci/aqhbci.h>
#include <aqhbci/medium.h>

#include <aqbanking/banking.h>
#include <aqbanking/provider_be.h>
#include <aqbanking/user.h>

/** @defgroup G_AB_BE_AQHBCI HBCI Backend (AqHBCI)
 *  @ingroup G_AB_BACKENDS
 *
 * AqHBCI supports the German HBCI (Homebanking Computer Interface) protocol
 * for online banking. It currently supports version 2.01, 2.10 and 2.20 of
 * the HBCI specification.
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif


AQHBCI_API
AB_PROVIDER *AH_Provider_new(AB_BANKING *ab, const char *name);


/** @name Informative Functions
 *
 */
/*@{*/
AQHBCI_API
const char *AH_Provider_GetProductName(const AB_PROVIDER *pro);

AQHBCI_API
const char *AH_Provider_GetProductVersion(const AB_PROVIDER *pro);

/*@}*/



/** @name Server Interactive Functions
 *
 * Functions in this group are used from setup wizards or tools.
 * They send requests to the server (including opening and closing of the
 * progress dialog by calling @ref AB_Banking_ProgressStart etc).
 */
/*@{*/
/**
 * Retrieve a list of accounts. Not all banks support this. If the bank does
 * then the retrieved accounts are automatically added to AqBanking.
 * @param pro pointer to the HBCI provider
 * @param u user for which the list of accounts is to be received
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
AQHBCI_API
int AH_Provider_GetAccounts(AB_PROVIDER *pro, AB_USER *u,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            int nounmount);

/**
 * Retrieve the system id for the given user. This is only needed for
 * PIN/TAN and for RDH mode.
 * @param pro pointer to the HBCI provider
 * @param u user for which the system id is to be received
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
AQHBCI_API
int AH_Provider_GetSysId(AB_PROVIDER *pro, AB_USER *u,
                         AB_IMEXPORTER_CONTEXT *ctx,
                         int nounmount);

/**
 * Retrieve the public server keys for the given user. This is only needed for
 * PIN/TAN and for RDH mode.
 * @param pro pointer to the HBCI provider
 * @param u user for which the public keys are to be received
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
AQHBCI_API
int AH_Provider_GetServerKeys(AB_PROVIDER *pro, AB_USER *u,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              int nounmount);

/**
 * Retrieve the public keys of the given user. This is only needed for
 * PIN/TAN and for RDH mode.
 * @param pro pointer to the HBCI provider
 * @param u user for which the public keys are to be sent
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
AQHBCI_API
int AH_Provider_SendUserKeys(AB_PROVIDER *pro, AB_USER *u,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             int nounmount);

/**
 * Retrieve the SSL certificate for the given user. This is only needed for
 * PIN/TAN mode.
 * @param pro pointer to the HBCI provider
 * @param u user for which the SSL certificate is to be received
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
AQHBCI_API
int AH_Provider_GetCert(AB_PROVIDER *pro, AB_USER *u, int nounmount);
/*@}*/



/** @name Generating Ini-Letters
 *
 * INI letters are used in RDH mode only. They are used to verify the public
 * server keys and to create a letter to be sent to the bank for verification
 * of the public user keys.
 */
/*@{*/
/**
 * Creates a text version of the INI letter. This function needs to mount
 * the medium so the user will be ask for the pin.
 */
AQHBCI_API
int AH_Provider_GetIniLetterTxt(AB_PROVIDER *pro,
                                AB_USER *u,
                                int useBankKey,
                                GWEN_BUFFER *lbuf,
                                int nounmount);

/**
 * Creates a HTML version of the INI letter. This function needs to mount
 * the medium so the user will be ask for the pin.
 */
AQHBCI_API
int AH_Provider_GetIniLetterHtml(AB_PROVIDER *pro,
                                 AB_USER *u,
                                 int useBankKey,
                                 GWEN_BUFFER *lbuf,
                                 int nounmount);
/*@}*/


/** @name Security Media Management
 *
 */
/*@{*/
AQHBCI_API
const AH_MEDIUM_LIST *AH_Provider_GetMediaList(AB_PROVIDER *pro);

AQHBCI_API
AH_MEDIUM *AH_Provider_MediumFactory(AB_PROVIDER *pro,
                                     const char *typeName,
                                     const char *subTypeName,
                                     const char *mediumName);

AQHBCI_API
AH_MEDIUM *AH_Provider_FindMedium(const AB_PROVIDER *pro,
                                  const char *typeName,
                                  const char *mediumName);

AQHBCI_API
AH_MEDIUM *AH_Provider_FindMediumById(const AB_PROVIDER *pro,
                                      GWEN_TYPE_UINT32 id);

AQHBCI_API
int AH_Provider_AddMedium(AB_PROVIDER *pro, AH_MEDIUM *m);

AQHBCI_API
int AH_Provider_RemoveMedium(AB_PROVIDER *pro, AH_MEDIUM *m);

AQHBCI_API
int AH_Provider_CheckMedium(AB_PROVIDER *pro,
                            GWEN_CRYPTTOKEN_DEVICE dev,
                            GWEN_BUFFER *mtypeName,
                            GWEN_BUFFER *msubTypeName,
                            GWEN_BUFFER *mediumName);

/*@}*/


#ifdef __cplusplus
}
#endif



/*@}*/ /* defgroup */



#endif /* AH_PROVIDER_H */




