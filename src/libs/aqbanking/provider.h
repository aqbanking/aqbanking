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


#ifndef AQBANKING_PROVIDER_H
#define AQBANKING_PROVIDER_H


#include <gwenhywfar/misc.h>
#include <gwenhywfar/misc2.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/bufferedio.h>
#include <aqbanking/error.h> /* for AQBANKING_API */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AB_PROVIDER AB_PROVIDER;
GWEN_INHERIT_FUNCTION_LIB_DEFS(AB_PROVIDER, AQBANKING_API)
GWEN_LIST_FUNCTION_LIB_DEFS(AB_PROVIDER, AB_Provider, AQBANKING_API)
/* Do not terminate these lines with semicolon because they are
   macros, not functions, and ISO C89 does not allow a semicolon
   there. */

typedef struct AB_PROVIDER_DESCRIPTION AB_PROVIDER_DESCRIPTION;
GWEN_INHERIT_FUNCTION_LIB_DEFS(AB_PROVIDER_DESCRIPTION, AQBANKING_API)
GWEN_LIST_FUNCTION_LIB_DEFS(AB_PROVIDER_DESCRIPTION, AB_ProviderDescription, AQBANKING_API)
GWEN_LIST2_FUNCTION_LIB_DEFS(AB_PROVIDER_DESCRIPTION, AB_ProviderDescription, AQBANKING_API)

typedef struct AB_PROVIDER_WIZARD AB_PROVIDER_WIZARD;
GWEN_INHERIT_FUNCTION_LIB_DEFS(AB_PROVIDER_WIZARD, AQBANKING_API)
GWEN_LIST_FUNCTION_LIB_DEFS(AB_PROVIDER_WIZARD, AB_ProviderWizard, AQBANKING_API)

#ifdef __cplusplus
}
#endif


#include <aqbanking/banking.h>
#include <aqbanking/error.h>
#include <aqbanking/job.h>
#include <aqbanking/account.h>
#include <aqbanking/transaction.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup AB_PROVIDER AB_PROVIDER (Online Banking Backends)
 * @ingroup AB_C_INTERFACE
 *
 * @brief This group represents backends.
 */
/*@{*/


typedef AB_PROVIDER* (*AB_PROVIDER_FACTORY_FN)(AB_BANKING *ab);

/** @name Prototypes For Virtual Functions
 *
 */
/*@{*/
typedef int (*AB_PROVIDER_UPDATEJOB_FN)(AB_PROVIDER *pro, AB_JOB *j);
typedef int (*AB_PROVIDER_ADDJOB_FN)(AB_PROVIDER *pro, AB_JOB *j);
typedef int (*AB_PROVIDER_EXECUTE_FN)(AB_PROVIDER *pro);
typedef AB_ACCOUNT_LIST2* (*AB_PROVIDER_GETACCOUNTLIST_FN)(AB_PROVIDER *pro);
typedef int (*AB_PROVIDER_UPDATEACCOUNT_FN)(AB_PROVIDER *pro, AB_ACCOUNT *a);
typedef int (*AB_PROVIDER_ADDACCOUNT_FN)(AB_PROVIDER *pro, AB_ACCOUNT *a);
typedef int (*AB_PROVIDER_IMPORTTRANSACTIONS_FN)(AB_PROVIDER *pro,
                                                 AB_TRANSACTION_LIST2 *tl,
                                                 GWEN_BUFFEREDIO *bio);

/*@}*/




AQBANKING_API
AB_PROVIDER *AB_Provider_new(AB_BANKING *ab,
                             const char *name);
AQBANKING_API
void AB_Provider_free(AB_PROVIDER *pro);

AQBANKING_API
const char *AB_Provider_GetName(const AB_PROVIDER *pro);
AQBANKING_API
AB_BANKING *AB_Provider_GetBanking(const AB_PROVIDER *pro);



/** @name Virtual Functions
 *
 */
/*@{*/

AQBANKING_API
int AB_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
AQBANKING_API
int AB_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
AQBANKING_API
int AB_Provider_Execute(AB_PROVIDER *pro);
AQBANKING_API
AB_ACCOUNT_LIST2 *AB_Provider_GetAccountList(AB_PROVIDER *pro);
AQBANKING_API
int AB_Provider_UpdateAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);
AQBANKING_API
int AB_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);

AQBANKING_API
int AB_Provider_ImportTransactions(AB_PROVIDER *pro,
                                   AB_TRANSACTION_LIST2 *tl,
                                   GWEN_BUFFEREDIO *bio);

/*@}*/



/** @name Setters For Virtual Functions
 *
 */
/*@{*/
AQBANKING_API
void AB_Provider_SetUpdateJobFn(AB_PROVIDER *pro, AB_PROVIDER_UPDATEJOB_FN f);
AQBANKING_API
void AB_Provider_SetAddJobFn(AB_PROVIDER *pro, AB_PROVIDER_ADDJOB_FN f);
AQBANKING_API
void AB_Provider_SetExecuteFn(AB_PROVIDER *pro, AB_PROVIDER_EXECUTE_FN f);
AQBANKING_API
void AB_Provider_SetGetAccountListFn(AB_PROVIDER *pro,
                                     AB_PROVIDER_GETACCOUNTLIST_FN f);
AQBANKING_API
void AB_Provider_SetUpdateAccountFn(AB_PROVIDER *pro,
                                    AB_PROVIDER_UPDATEACCOUNT_FN f);
AQBANKING_API
void AB_Provider_SetAddAccountFn(AB_PROVIDER *pro,
                                 AB_PROVIDER_ADDACCOUNT_FN f);
AQBANKING_API
void AB_Provider_SetImportTransactionsFn(AB_PROVIDER *pro,
                                         AB_PROVIDER_IMPORTTRANSACTIONS_FN f);
/*@}*/




/** @name Functions For Loading Provider Plugins
 *
 */
/*@{*/
AQBANKING_API
AB_PROVIDER *AB_Provider_LoadPluginFile(AB_BANKING *ab,
                                        const char *modname,
                                        const char *fname);
AQBANKING_API
AB_PROVIDER *AB_Provider_LoadPlugin(AB_BANKING *ab,
                                    const char *modname);


/*@}*/





typedef AB_PROVIDER_WIZARD*
  (*AB_PROVIDER_WIZARD_FACTORY_FN)(AB_PROVIDER *pro);

typedef int (*AB_PROVIDER_WIZARD_SETUP_FN)(AB_PROVIDER_WIZARD *pw);


AQBANKING_API
AB_PROVIDER_WIZARD *AB_ProviderWizard_new(AB_PROVIDER *pro,
					  const char *name);
AQBANKING_API
void AB_ProviderWizard_free(AB_PROVIDER_WIZARD *pw);

AQBANKING_API
const char *AB_ProviderWizard_GetName(const AB_PROVIDER_WIZARD *pw);
AQBANKING_API
AB_PROVIDER *AB_ProviderWizard_GetProvider(const AB_PROVIDER_WIZARD *pw);

AQBANKING_API
int AB_ProviderWizard_Setup(AB_PROVIDER_WIZARD *pw);


AQBANKING_API
void AB_ProviderWizard_SetSetupFn(AB_PROVIDER_WIZARD *pw,
				  AB_PROVIDER_WIZARD_SETUP_FN f);

AQBANKING_API
GWEN_DB_NODE *AB_ProviderWizard_GetData(AB_PROVIDER_WIZARD *pw);


/**
 * Loads a backend wizard.
 * When actually loading a wizard then this function searches inside the
 * plugin for a function called WIZARDNAME_factory() (WIZARDNAME is the
 * name given for argument <i>modname</i>).
 * That function is expected to return  a pointer to a
 * @ref AB_PROVIDER_WIZARD.
 */
AQBANKING_API
AB_PROVIDER_WIZARD *AB_ProviderWizard_LoadPluginFile(AB_PROVIDER *pro,
                                                     const char *modname,
                                                     const char *fname);

AQBANKING_API
AB_PROVIDER_WIZARD *AB_ProviderWizard_LoadPlugin(AB_PROVIDER *pro,
                                                 const char *modname);


/*@}*/ /* defgroup */

#ifdef __cplusplus
}
#endif




#endif /* AQBANKING_PROVIDER_H */









