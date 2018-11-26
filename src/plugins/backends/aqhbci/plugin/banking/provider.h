/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_H
#define AH_PROVIDER_H


#include <aqhbci/aqhbci.h>

#include <aqbanking/banking.h>
#include <aqbanking/provider_be.h>
#include <aqbanking/user.h>

#include <gwenhywfar/ct.h>


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


  /**
   * This enum can be used as argument to @ref AB_Provider_GetNewUserDialog
   */
  enum AQHBCI_NEWUSER_DIALOG_CODE {
    AqHBCI_NewUserDialog_CodeGeneric=0,
    AqHBCI_NewUserDialog_CodeExistingPinTan,
    AqHBCI_NewUserDialog_CodeCreateKeyFile,
    AqHBCI_NewUserDialog_CodeExistingKeyFile,
    AqHBCI_NewUserDialog_CodeCreateChipcard,
    AqHBCI_NewUserDialog_CodeExistingChipcard
  };


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
			    int withProgress, int nounmount, int doLock);

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
			 int withProgress, int nounmount, int doLock);

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
			      int withProgress, int nounmount, int doLock);

/**
 * Send the public keys of the given user. This is only needed for
 * PIN/TAN and for RDH mode.
 * @param pro pointer to the HBCI provider
 * @param u user for which the public keys are to be sent
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
AQHBCI_API
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
AQHBCI_API
int AH_Provider_SendUserKeys2(AB_PROVIDER *pro, AB_USER *u,
			      AB_IMEXPORTER_CONTEXT *ctx,
			      int withAuthKey,
			      int withProgress, int nounmount, int doLock);

/**
 * Retrieve the SSL certificate for the given user. This is only needed for
 * PIN/TAN mode.
 * @param pro pointer to the HBCI provider
 * @param u user for which the SSL certificate is to be received
 * @param nounmount if !=0 then the user's medium is not unmounted in the end.
 *  This is used by setup wizards to avoid having to enter a pin too often.
 */
AQHBCI_API
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
AQHBCI_API
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
AQHBCI_API
int AH_Provider_ChangePin(AB_PROVIDER *pro, AB_USER *u,
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
AQHBCI_API
int AH_Provider_GetAccountSepaInfo(AB_PROVIDER *pro,
                                   AB_ACCOUNT *a,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   int withProgress, int nounmount, int doLock);


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
 * @param pro provider
 * @param u user for which the iniletter is to be created
 * @param useBankKey if !=0 create an iniletter for the bank key
 * @param variant use 1 for RDH1, 2 for RDH2-10 (or 0 for autoselection)
 * @param lbuf buffer to write the iniletter to
 * @param nounmount if !=0 the CryptToken will not be unmounted after use
 */
AQHBCI_API
int AH_Provider_GetIniLetterTxt(AB_PROVIDER *pro,
                                AB_USER *u,
                                int useBankKey,
                                int variant,
                                GWEN_BUFFER *lbuf,
				int nounmount);

/**
 * Creates a HTML version of the INI letter. This function needs to mount
 * the medium so the user will be ask for the pin.
 * @param pro provider
 * @param u user for which the iniletter is to be created
 * @param useBankKey if !=0 create an iniletter for the bank key
 * @param variant use 1 for RDH1, 2 for RDH2-10 (or 0 for autoselection)
 * @param lbuf buffer to write the iniletter to
 * @param nounmount if !=0 the CryptToken will not be unmounted after use
 */
AQHBCI_API
int AH_Provider_GetIniLetterHtml(AB_PROVIDER *pro,
                                 AB_USER *u,
                                 int useBankKey,
                                 int variant,
                                 GWEN_BUFFER *lbuf,
				 int nounmount);
/*@}*/


/**
 * Creates user keys for RDH type users.
 *
 * @param pro provider
 * @param u user for which the keys are to be created
 * @param nounmount if !=0 the CryptToken will not be unmounted after use
 */
AQHBCI_API
int AH_Provider_CreateKeys(AB_PROVIDER *pro, AB_USER *u, int nounmount);


AQHBCI_API
int AH_Provider_Test(AB_PROVIDER *pro);


/*@}*/




/** @name Account Management Functions
 *
 */
/*@{*/

/**
 * Add the given account. It gets assigned a unique id.
 *
 * @return 0 on success, error code otherwise
 * @param pro pointer to provider object
 * @param u user to which the account belongs
 * @param a pointer to account
 * @param withAccountSpec if !=0 then an account spec will also be created and written
 */
AQHBCI_API int AH_Provider_AddAccount(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, int withAccountSpec);

/**
 * Delete the given account.
 *
 * @param pro pointer to provider object
 * @param a pointer to account
 */
AQHBCI_API int AH_Provider_DeleteAccount(AB_PROVIDER *pro, uint32_t uid);


/*@}*/



/** @name User Management Functions
 *
 */
/*@{*/

/**
 * This functions reads a user from the configuration database.
 * When reading the user object it will be locked and/or unlocked as requestd.
 * If both the parameters doLock and doUnlock are !=0 you can later call @ref AH_Provider_EndExclUseUser on the
 * user object returned (if any).
 *
 * @param pro pointer to provider object
 * @param uid unique id of the user to read
 * @param doLock if !0 0 the config group for the given object will be locked before reading
 * @param doUnlock if !0 0 the config group for the given object will be unlocked after reading
 * @param pUser pointer to a variable to receive the user read
 */
AQHBCI_API int AH_Provider_GetUser(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, AB_USER **pUser);


/**
 * Read all users of this backend.
 *
 * The caller is responsible for releasing the list and the contained users (if any),
 * e.g. by calling @ref AB_User_List_free().
 *
 * @return 0 on success, error code otherwise
 * @param pro pointer to provider object
 * @param userList pointer to a list to receive the users.
 */
AQHBCI_API int AH_Provider_ReadUsers(AB_PROVIDER *pro, AB_USER_LIST *userList);

/**
 * Begin exclusively using the given user.
 * This function locks the configuration for the given user, reads the configuration and
 * leaves the configuration locked upon return.
 * Therefore you MUST call @ref AH_Provider_EndExclUseUser() to unlock it later.
 */
AQHBCI_API int AH_Provider_BeginExclUseUser(AB_PROVIDER *pro, AB_USER *u);


/**
 * End exclusive use of the given user.
 * This function writes the still locked configuration of the user and unlocks it afterwards.
 *
 * @param pro pointer to provider object
 * @param u pointer to user
 * @param abandon if !=0 the configuration is just unlocked, not written
 */
AQHBCI_API int AH_Provider_EndExclUseUser(AB_PROVIDER *pro, AB_USER *u, int abandon);


/**
 * Add the given user. It gets assigned a unique id.
 *
 * @return 0 on success, error code otherwise
 * @param pro pointer to provider object
 * @param u pointer to user
 */
AQHBCI_API int AH_Provider_AddUser(AB_PROVIDER *pro, AB_USER *u);


/**
 * Delete the given user.
 *
 * The user will not be deleted if any account refers to it. In that case the
 * accounts need to be removed first.
 *
 * @return 0 on success, error code otherwise
 * @param pro pointer to provider object
 * @param uid unique id of the user to remove
 */
AQHBCI_API int AH_Provider_DeleteUser(AB_PROVIDER *pro, uint32_t uid);


/*@}*/





#ifdef __cplusplus
}
#endif



/*@}*/ /* defgroup */



#endif /* AH_PROVIDER_H */




