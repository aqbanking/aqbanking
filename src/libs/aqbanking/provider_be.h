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

#include <aqbanking/provider.h>
#include <aqbanking/ab_providerqueue.h>
#include <aqbanking/user.h>

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


/**
 * This type is used with @ref AB_Provider_ExtendAccount and
 * @ref AB_Provider_ExtendUser.
 */
typedef enum {
  /** Object to be extended has just been created. For some backends this
   * means that some settings are allowed to be missing at this point.*/
  AB_ProviderExtendMode_Create=0,
  /** Object to be extended has been read from the configuration file */
  AB_ProviderExtendMode_Extend,
  /** Object to be extended has just been added to internal lists.
   * For the backend this might mean that the object should be completely
   * setup at this point. */
  AB_ProviderExtendMode_Add,
  /** Object to be extended is just about to be removed from the internal
   * list. */
  AB_ProviderExtendMode_Remove,
  /** This extend mode just lets the backend store data which has not yet
   * been stored into the users/accounts DB.
   * Please note that in this mode the backend might no longer be
   * initialized, so you should not call any other provider function (or call
   * @ref AB_Provider_IsInit to see whether the backend still is initialized).
   */
  AB_ProviderExtendMode_Save,

  /** This mode tells the backend to reload its configuration from the given
   * DB.
   */
  AB_ProviderExtendMode_Reload
} AB_PROVIDER_EXTEND_MODE;


/**
 * See @ref AB_Provider_Init.
 */
typedef int (*AB_PROVIDER_INIT_FN)(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);

/**
 * See @ref AB_Provider_Fini.
 */
typedef int (*AB_PROVIDER_FINI_FN)(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);


typedef GWEN_DIALOG* (*AB_PROVIDER_GET_NEWUSER_DIALOG_FN)(AB_PROVIDER *pro, int i);

typedef GWEN_DIALOG* (*AB_PROVIDER_GET_EDITUSER_DIALOG_FN)(AB_PROVIDER *pro, AB_USER *u);

typedef GWEN_DIALOG* (*AB_PROVIDER_GET_NEWACCOUNT_DIALOG_FN)(AB_PROVIDER *pro);

typedef GWEN_DIALOG* (*AB_PROVIDER_GET_EDITACCOUNT_DIALOG_FN)(AB_PROVIDER *pro, AB_ACCOUNT *a);

typedef GWEN_DIALOG* (*AB_PROVIDER_GET_USERTYPE_DIALOG_FN)(AB_PROVIDER *pro);



typedef int (*AB_PROVIDER_SENDCOMMANDS_FN)(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx);


/*@}*/





AQBANKING_API
AB_PROVIDER *AB_Provider_new(AB_BANKING *ab,
                             const char *name);

/**
 * @return 0 if the backend is not initialized, !=0 if it is
 */
AQBANKING_API
int AB_Provider_IsInit(const AB_PROVIDER *pro);


AQBANKING_API
void AB_Provider_AddFlags(AB_PROVIDER *pro, uint32_t fl);


/** @name Virtual Functions
 *
 */
/*@{*/

/**
 * Allow the backend to initialize itself.
 * @param pro backend object
 * @param db db of the config group for this backend
 */
AQBANKING_API
int AB_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *db);

/**
 * Allow the backend to deinitialize itself.
 * @param pro backend object
 * @param db db of the config group for this backend
 */
AQBANKING_API
int AB_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *db);




/**
 * Create a dialog which allows to create a new user.
 * The dialog returned (if any) must be derived via @ref AB_NewUserDialog_new().
 * @param pro pointer to the backend for which a new user is to be created
 * @param i additional parameter depending on the backend. it can be used
 *   to specify the user type to be created (e.g. for HBCI those values
 *   specify whether PIN/TAN, keyfile or chipcard users are to be created).
 *   Use value 0 for the generic dialog.
 */
AQBANKING_API
GWEN_DIALOG *AB_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i);

AQBANKING_API
GWEN_DIALOG *AB_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u);

/**
 * Create a dialog which allows to create a new account.
 * The dialog returned (if any) must be derived via @ref AB_NewAccountDialog_new().
 */
AQBANKING_API
GWEN_DIALOG *AB_Provider_GetNewAccountDialog(AB_PROVIDER *pro);

AQBANKING_API
GWEN_DIALOG *AB_Provider_GetEditAccountDialog(AB_PROVIDER *pro, AB_ACCOUNT *a);


AQBANKING_API
GWEN_DIALOG *AB_ProviderGetUserTypeDialog(AB_PROVIDER *pro);



/**
 * Send commands to backends.
 * The given queue can be modified by the provider in this function as it will be deleted upon return from this function.
 * @return 0 if okay, error code otherwise
 * @param pro pointer to the provider
 * @param pq provider queue which contains the commands to send, sorted by account (may be modified by provider)
 * @param ctx context to receive results
 */
AQBANKING_API int AB_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx);


/*@}*/



/** @name Setters For Virtual Functions
 *
 */
/*@{*/
AQBANKING_API
void AB_Provider_SetInitFn(AB_PROVIDER *pro, AB_PROVIDER_INIT_FN f);
AQBANKING_API
void AB_Provider_SetFiniFn(AB_PROVIDER *pro, AB_PROVIDER_FINI_FN f);

AQBANKING_API
void AB_Provider_SetGetNewUserDialogFn(AB_PROVIDER *pro, AB_PROVIDER_GET_NEWUSER_DIALOG_FN f);

AQBANKING_API
void AB_Provider_SetGetEditUserDialogFn(AB_PROVIDER *pro, AB_PROVIDER_GET_EDITUSER_DIALOG_FN f);

AQBANKING_API
void AB_Provider_SetGetNewAccountDialogFn(AB_PROVIDER *pro, AB_PROVIDER_GET_NEWACCOUNT_DIALOG_FN f);

AQBANKING_API
void AB_Provider_SetGetEditAccountDialogFn(AB_PROVIDER *pro, AB_PROVIDER_GET_EDITACCOUNT_DIALOG_FN f);

AQBANKING_API
void AB_Provider_SetGetUserTypeDialogFn(AB_PROVIDER *pro, AB_PROVIDER_GET_USERTYPE_DIALOG_FN f);


AQBANKING_API void AB_Provider_SetSendCommandsFn(AB_PROVIDER *pro, AB_PROVIDER_SENDCOMMANDS_FN f);

/*@}*/




typedef AB_PROVIDER* (*AB_PLUGIN_PROVIDER_FACTORY_FN)(GWEN_PLUGIN *pl,
						      AB_BANKING *ab);


AQBANKING_API
GWEN_PLUGIN *AB_Plugin_Provider_new(GWEN_PLUGIN_MANAGER *pm,
				    const char *name,
				    const char *fileName);


AQBANKING_API
AB_PROVIDER *AB_Plugin_Provider_Factory(GWEN_PLUGIN *pl, AB_BANKING *ab);

AQBANKING_API
void AB_Plugin_Provider_SetFactoryFn(GWEN_PLUGIN *pl,
				     AB_PLUGIN_PROVIDER_FACTORY_FN fn);




/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif




#endif /* AQBANKING_PROVIDER_BE_H */









