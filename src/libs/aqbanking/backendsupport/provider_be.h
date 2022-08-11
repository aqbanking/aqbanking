/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/** @file provider_be.h
 * @short This file is used by provider backends.
 */


#ifndef AQBANKING_PROVIDER_BE_H
#define AQBANKING_PROVIDER_BE_H

#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/providerqueue.h>
#include <aqbanking/backendsupport/userqueue.h>
#include <aqbanking/backendsupport/user.h>

#include <gwenhywfar/plugin.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/dialog.h>


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup G_AB_PROVIDER
 *
 */
/*@{*/


#define AB_PROVIDER_FLAGS_HAS_NEWUSER_DIALOG     0x00000001
#define AB_PROVIDER_FLAGS_HAS_EDITUSER_DIALOG    0x00000002
#define AB_PROVIDER_FLAGS_HAS_NEWACCOUNT_DIALOG  0x00000004
#define AB_PROVIDER_FLAGS_HAS_EDITACCOUNT_DIALOG 0x00000008
#define AB_PROVIDER_FLAGS_HAS_USERTYPE_DIALOG    0x00000010



/** See @ref AB_Provider_Init. */
typedef int (*AB_PROVIDER_INIT_FN)(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);

/** See @ref AB_Provider_Fini. */
typedef int (*AB_PROVIDER_FINI_FN)(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);


typedef GWEN_DIALOG *(*AB_PROVIDER_GET_NEWUSER_DIALOG_FN)(AB_PROVIDER *pro, int i);

typedef GWEN_DIALOG *(*AB_PROVIDER_GET_EDITUSER_DIALOG_FN)(AB_PROVIDER *pro, AB_USER *u);

typedef GWEN_DIALOG *(*AB_PROVIDER_GET_NEWACCOUNT_DIALOG_FN)(AB_PROVIDER *pro);

typedef GWEN_DIALOG *(*AB_PROVIDER_GET_EDITACCOUNT_DIALOG_FN)(AB_PROVIDER *pro, AB_ACCOUNT *a);

typedef GWEN_DIALOG *(*AB_PROVIDER_GET_USERTYPE_DIALOG_FN)(AB_PROVIDER *pro);



typedef int (*AB_PROVIDER_SENDCOMMANDS_FN)(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx);


typedef AB_ACCOUNT *(*AB_PROVIDER_CREATEACCOUNTOBJECT_FN)(AB_PROVIDER *pro);

typedef AB_USER *(*AB_PROVIDER_CREATEUSEROBJECT_FN)(AB_PROVIDER *pro);

typedef int (*AB_PROVIDER_UPDATEACCOUNTSPEC_FN)(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock);

typedef int (*AB_PROVIDER_CONTROL_FN)(AB_PROVIDER *pro, int argc, char **argv);

/*@}*/





AB_PROVIDER *AB_Provider_new(AB_BANKING *ab, const char *name);



/**
 * @return 0 if the backend is not initialized, !=0 if it is
 */
int AB_Provider_IsInit(const AB_PROVIDER *pro);



void AB_Provider_AddFlags(AB_PROVIDER *pro, uint32_t fl);


/** @name Virtual Functions - Minimally Required Functions
 *
 */
/*@{*/


/**
 * Allow the backend to initialize itself.
 * @param pro backend object
 * @param db db of the config group for this backend
 */
int AB_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *db);



/**
 * Allow the backend to deinitialize itself.
 * @param pro backend object
 * @param db db of the config group for this backend
 */
int AB_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *db);



/**
 * Send commands to backends.
 * The given queue can be modified by the provider in this function as it will be deleted upon return from this function.
 * @return 0 if okay, error code otherwise
 * @param pro pointer to the provider
 * @param pq provider queue which contains the commands to send, sorted by account (may be modified by provider)
 * @param ctx context to receive results
 */
int AB_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx);

/*@}*/



/** @name Virtual Functions - Functions Required When Working with AB_ACCOUNT/AB_USER
 *
 */
/*@{*/

/**
 * Create an empty AB_ACCOUNT object.
 * This function only needs to be implemented by backends which use AB_ACCOUNT objects internally and which
 * want to use AqBankings convenience functions for loading and saving AB_ACCOUNT objects.
 *
 * @return AB_ACCOUNT object created, NULL on error
 * @param pro provider which is to create the object
 */
AB_ACCOUNT *AB_Provider_CreateAccountObject(AB_PROVIDER *pro);


/**
 * Create an empty AB_USER object.
 * This function only needs to be implemented by backends which use AB_USER objects internally and which
 * want to use AqBankings convenience functions for loading and saving AB_USER objects.
 *
 * @return AB_USER object created, NULL on error
 * @param pro provider which is to create the object
 */
AB_USER *AB_Provider_CreateUserObject(AB_PROVIDER *pro);


/**
 * Update the given account spec.
 *
 * This callback gives a provider the opportunity to set the transaction limits and other stuff.
 * This function only needs to be implemented in the backend if it has some specials things to
 * setup (like transaction limits).
 *
 * @return 0 if okay, error code otherwise
 * @param pro pointer to provider object
 * @param as account spec object to update
 */
int AB_Provider_UpdateAccountSpec(AB_PROVIDER *pro, AB_ACCOUNT_SPEC *as, int doLock);

/*@}*/



/** @name Virtual Functions - Special Functions
 *
 */
/*@{*/

/**
 * Allow the backend to perform some tasks specific to the given provider.
 *
 * Most backends use this function to provide command line functions to retrieve account list,
 * create accounts and user etc.
 *
 * @return depending on the task performed, generally negative on error
 * @param pro pointer to provider object
 * @param argc number of arguments in the argument list
 * @param argv pointer to the argument list
 */
int AB_Provider_Control(AB_PROVIDER *pro, int argc, char **argv);


/*@}*/




/** @name Virtual Functions - Functions Providing Graphical Dialogs
 *
 */
/*@{*/

/**
 * Create a dialog which allows to create a new user.
 * The dialog returned (if any) must be derived via @ref AB_NewUserDialog_new().
 * @param pro pointer to the backend for which a new user is to be created
 * @param i additional parameter depending on the backend. it can be used
 *   to specify the user type to be created (e.g. for HBCI those values
 *   specify whether PIN/TAN, keyfile or chipcard users are to be created).
 *   Use value 0 for the generic dialog.
 */
GWEN_DIALOG *AB_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i);



GWEN_DIALOG *AB_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u);



/**
 * Create a dialog which allows to create a new account.
 * The dialog returned (if any) must be derived via @ref AB_NewAccountDialog_new().
 */
GWEN_DIALOG *AB_Provider_GetNewAccountDialog(AB_PROVIDER *pro);



GWEN_DIALOG *AB_Provider_GetEditAccountDialog(AB_PROVIDER *pro, AB_ACCOUNT *a);



GWEN_DIALOG *AB_Provider_GetUserTypeDialog(AB_PROVIDER *pro);



/*@}*/



/** @name Setters For Virtual Functions
 *
 */
/*@{*/
void AB_Provider_SetInitFn(AB_PROVIDER *pro, AB_PROVIDER_INIT_FN f);

void AB_Provider_SetFiniFn(AB_PROVIDER *pro, AB_PROVIDER_FINI_FN f);

void AB_Provider_SetSendCommandsFn(AB_PROVIDER *pro, AB_PROVIDER_SENDCOMMANDS_FN f);

void AB_Provider_SetCreateAccountObjectsFn(AB_PROVIDER *pro, AB_PROVIDER_CREATEACCOUNTOBJECT_FN f);

void AB_Provider_SetCreateUserObjectsFn(AB_PROVIDER *pro, AB_PROVIDER_CREATEUSEROBJECT_FN f);

void AB_Provider_SetUpdateAccountSpecFn(AB_PROVIDER *pro, AB_PROVIDER_UPDATEACCOUNTSPEC_FN f);

void AB_Provider_SetControlFn(AB_PROVIDER *pro, AB_PROVIDER_CONTROL_FN f);


void AB_Provider_SetGetNewUserDialogFn(AB_PROVIDER *pro, AB_PROVIDER_GET_NEWUSER_DIALOG_FN f);

void AB_Provider_SetGetEditUserDialogFn(AB_PROVIDER *pro, AB_PROVIDER_GET_EDITUSER_DIALOG_FN f);

void AB_Provider_SetGetNewAccountDialogFn(AB_PROVIDER *pro, AB_PROVIDER_GET_NEWACCOUNT_DIALOG_FN f);

void AB_Provider_SetGetEditAccountDialogFn(AB_PROVIDER *pro, AB_PROVIDER_GET_EDITACCOUNT_DIALOG_FN f);

void AB_Provider_SetGetUserTypeDialogFn(AB_PROVIDER *pro, AB_PROVIDER_GET_USERTYPE_DIALOG_FN f);

/*@}*/





/** @name Account Management Functions
 *
 * Functions in this group can only be used if the callback for @ref AB_Provider_CreateAccountObject is set.
 */
/*@{*/

/**
 * Read account given by its unique id.
 *
 * When reading the object it will be locked and/or unlocked as requestd.
 * If both the parameters doLock and doUnlock are !=0 you can later call @ref AB_Provider_EndExclUseAccount on the
 * account object returned (if any).
 *
 * @return 0 if okay, <0 on error
 * @param pro provider (THIS in C++ speak)
 * @param uid unique id of the object to read
 * @param doLock do lock the objects configuration before reading
 * @param doUnlock do unlock the objects configuration after reading
 * @param account pointer to the object to read the configuration into
 */
int AB_Provider_ReadAccount(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, AB_ACCOUNT *account);


/**
 * Get account given by its unique id.
 *
 * When reading the object it will be locked and/or unlocked as requestd.
 * If both the parameters doLock and doUnlock are !=0 you can later call @ref AB_Provider_EndExclUseAccount on the
 * object returned (if any).
 *
 * Internally calls callback function @ref AB_Provider_CreateAccountObject.
 *
 * @return 0 if okay, <0 on error
 * @param pro provider (THIS in C++ speak)
 * @param uid unique id of the object to read
 * @param doLock do lock the objects configuration before reading
 * @param doUnlock do unlock the objects configuration after reading
 * @param pAccount pointer to a pointer to receive the object created and read
 */
int AB_Provider_GetAccount(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, AB_ACCOUNT **pAccount);


/**
 * Read all account objects belonging to this provider.
 *
 * Internally calls callback function @ref AB_Provider_CreateAccountObject.
 *
 * @return 0 if okay, <0 on error
 * @param pro provider (THIS in C++ speak)
 * @param accountList list to receive all objects read
 */
int AB_Provider_ReadAccounts(AB_PROVIDER *pro, AB_ACCOUNT_LIST *accountList);


/**
 * Check whether an account with the given id exists.
 * @return 0 if a user with the given id exists, error code otherwise
 * @param pro pointer to provider object
 * @param uid unique id of the object in question
 */
int AB_Provider_HasAccount(AB_PROVIDER *pro, uint32_t uid);


/**
 * Write account given by its unique id.
 *
 * When writing the object it will be locked and/or unlocked as requested.
 * If both the parameters doLock and doUnlock are !=0 you can later call @ref AH_Provider_EndExclUseAccount on the
 * object returned (if any).
 *
 * @return 0 if okay, <0 on error
 * @param pro provider (THIS in C++ speak)
 * @param uid unique id of the object to read
 * @param doLock do lock the objects configuration before reading
 * @param doUnlock do unlock the objects configuration after reading
 * @param account pointer to the object to be written to the configuration
 */
int AB_Provider_WriteAccount(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, const AB_ACCOUNT *account);


/**
 * Add an account to the configuration. Assigns a unique id to the new account.
 *
 * Also creates and adds a AB_ACCOUNT_SPEC object for applications.
 * Internally calls the callback function @ref AB_Provider_UpdateAccountSpec().
 *
 * @return 0 on success, error code otherwise
 * @param pro provider (THIS in C++ speak)
 * @param a account to add
 * @param lockCorrespondingUser lock user which this account belongs to while using user data.
 *        Some backends (e.g. AqHBCI) access the corresponding AB_USER object when creating AB_ACCOUNT_SPEC
 *        objects for the given account. If the user is already locked for any reason, the function
 *        should not try to lock the AB_USER object because that must fail.
 *
 */
int AB_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a, int lockCorrespondingUser);


/**
 * Remove an account from the configuration.
 *
 * Also removes the corresponding AB_ACCOUNT_SPEC object for applications.
 *
 * @return 0 on success, error code otherwise
 * @param pro provider (THIS in C++ speak)
 * @param uid unique id of the account to remove
 */
int AB_Provider_DeleteAccount(AB_PROVIDER *pro, uint32_t uid);


/**
 * Begin exclusively using the given account.
 * This function locks the configuration for the given account, reads the configuration and
 * leaves the configuration locked upon return.
 * Therefore you MUST call @ref AH_Provider_EndExclUseAccount() to unlock it later.
 */
int AB_Provider_BeginExclUseAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);


/**
 * End exclusive use of the given account.
 * This function writes the still locked configuration of the account and unlocks it afterwards.
 *
 * @param pro pointer to provider object
 * @param a pointer to account
 * @param abandon if !=0 the configuration is just unlocked, not written
 */
int AB_Provider_EndExclUseAccount(AB_PROVIDER *pro, AB_ACCOUNT *a, int abandon);


/**
 * Find an account spec from a list which matches the given account.
 * Only checks against account specs from the same provider as the called one (i.e. if this is the AqHBCI
 * provider only AqHBCI account specs are checked against).
 *
 * @return pointer to matching account spec from the given list, NULL if none found
 * @param acc account to look for
 * @param asl account spec list to check against
 */
AB_ACCOUNT_SPEC *AB_Provider_FindMatchingAccountSpec(AB_PROVIDER *pro, const AB_ACCOUNT *acc,
                                                     AB_ACCOUNT_SPEC_LIST *asl);


/*@}*/



/** @name User Management Functions
 *
 * Functions in this group can only be used if the callback for @ref AB_Provider_CreateUserObject is set.
 */
/*@{*/


/**
 * This functions reads a user from the configuration database.
 * When reading the user object it will be locked and/or unlocked as requestd.
 * If both the parameters doLock and doUnlock are !=0 you can later call @ref AB_Provider_EndExclUseUser on the
 * user object returned (if any).
 *
 * Internally calls callback function @ref AB_Provider_CreateUserObject.
 *
 * @param pro pointer to provider object
 * @param uid unique id of the user to read
 * @param doLock if !0 0 the config group for the given object will be locked before reading
 * @param doUnlock if !0 0 the config group for the given object will be unlocked after reading
 * @param pUser pointer to a variable to receive the user read
 */
int AB_Provider_GetUser(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, AB_USER **pUser);


/**
 * Read all users of this backend.
 *
 * The caller is responsible for releasing the list and the contained users (if any),
 * e.g. by calling @ref AB_User_List_free().
 *
 * Internally calls callback function @ref AB_Provider_CreateUserObject.
 *
 * @return 0 on success, error code otherwise
 * @param pro pointer to provider object
 * @param userList pointer to a list to receive the users.
 */
int AB_Provider_ReadUsers(AB_PROVIDER *pro, AB_USER_LIST *userList);


/**
 * Check whether a user with the given id exists.
 * @return 0 if a user with the given id exists, error code otherwise
 * @param pro pointer to provider object
 * @param uid unique id of the object in question
 */
int AB_Provider_HasUser(AB_PROVIDER *pro, uint32_t uid);


/**
 * Write user given by its unique id.
 *
 * When writing the object it will be locked and/or unlocked as requested.
 * If both the parameters doLock and doUnlock are !=0 you can later call @ref AB_Provider_EndExclUseUser on the
 * object returned (if any).
 *
 * @return 0 if okay, <0 on error
 * @param pro provider (THIS in C++ speak)
 * @param uid unique id of the object to write
 * @param doLock do lock the objects configuration before reading
 * @param doUnlock do unlock the objects configuration after reading
 * @param user pointer to the object to be written to the configuration
 */
int AB_Provider_WriteUser(AB_PROVIDER *pro, uint32_t uid, int doLock, int doUnlock, const AB_USER *user);


/**
 * Add a user to the configuration. Assigns a unique id to the new user.
 *
 * @return 0 on success, error code otherwise
 * @param pro provider (THIS in C++ speak)
 * @param u user to add
 *
 */
int AB_Provider_AddUser(AB_PROVIDER *pro, AB_USER *u);


/**
 * Remove a user from the configuration.
 *
 * @return 0 on success, error code otherwise
 * @param pro provider (THIS in C++ speak)
 * @param uid unique id of the account to remove
 */
int AB_Provider_DeleteUser(AB_PROVIDER *pro, uint32_t uid);



/**
 * Begin exclusively using the given user.
 * This function locks the configuration for the given user, reads the configuration and
 * leaves the configuration locked upon return.
 * Therefore you MUST call @ref AH_Provider_EndExclUseUser() to unlock it later.
 */
int AB_Provider_BeginExclUseUser(AB_PROVIDER *pro, AB_USER *u);


/**
 * End exclusive use of the given user.
 * This function writes the still locked configuration of the user and unlocks it afterwards.
 *
 * @param pro pointer to provider object
 * @param u pointer to user
 * @param abandon if !=0 the configuration is just unlocked, not written
 */
int AB_Provider_EndExclUseUser(AB_PROVIDER *pro, AB_USER *u, int abandon);


/*@}*/



/** @name Account Spec Management Functions
 *
 */
/*@{*/


int AB_Provider_AccountToAccountSpec(AB_PROVIDER *pro, const AB_ACCOUNT *acc, AB_ACCOUNT_SPEC *as, int doLock);
int AB_Provider_WriteAccountSpecForAccount(AB_PROVIDER *pro, const AB_ACCOUNT *acc, int doLock);

/**
 *
 * Internally calls callback function @ref AB_Provider_CreateAccountObject(.
 */
int AB_Provider_CreateInitialAccountSpecs(AB_PROVIDER *pro);


/*@}*/






/** @name Queue Management Functions
 *
 */
/*@{*/


/**
 * Sort jobs in provider queues (AB_PROVIDERQUEUE) into a list of AB_USERQUEUEs.
 * This function makes use of the field @ref AB_ACCOUNT_userId.
 */
int AB_Provider_SortProviderQueueIntoUserQueueList(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_USERQUEUE_LIST *uql);


/**
 * Frees all users and accounts mentioned in the given AB_USERQUEUE list.
 */
void AB_Provider_FreeUsersAndAccountsFromUserQueueList(AB_PROVIDER *pro, AB_USERQUEUE_LIST *uql);



/*@}*/




/** @name General Utility Functions
 *
 */
/*@{*/

void AB_Provider_DumpTransactionsIfDebug(const AB_IMEXPORTER_ACCOUNTINFO *ai, const char *logdomain);

/**
 * This function takes all data from the given srcContext and writes it into the given destAccountInfo.
 * Transactions will get the given type ty. Destroys srcContext.
 */
void AB_Provider_MergeContextsSetTypeAndFreeSrc(AB_IMEXPORTER_ACCOUNTINFO *destAccountInfo, AB_IMEXPORTER_CONTEXT *srcContext, int ty);


/**
 * Get or create and add an account info to an ImExporterContext for the given account.
 */
AB_IMEXPORTER_ACCOUNTINFO *AB_Provider_GetOrAddAccountInfoForAccount(AB_IMEXPORTER_CONTEXT *ctx, const AB_ACCOUNT *a);


/*@}*/




typedef AB_PROVIDER *(*AB_PLUGIN_PROVIDER_FACTORY_FN)(GWEN_PLUGIN *pl, AB_BANKING *ab);


GWEN_PLUGIN *AB_Plugin_Provider_new(GWEN_PLUGIN_MANAGER *pm,
                                    const char *name,
                                    const char *fileName);


AB_PROVIDER *AB_Plugin_Provider_Factory(GWEN_PLUGIN *pl, AB_BANKING *ab);

void AB_Plugin_Provider_SetFactoryFn(GWEN_PLUGIN *pl, AB_PLUGIN_PROVIDER_FACTORY_FN fn);




/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif




#endif /* AQBANKING_PROVIDER_BE_H */









