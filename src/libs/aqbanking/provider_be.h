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

/** @file provider_be.h
 * @short This file is used by provider backends.
 */


#ifndef AQBANKING_PROVIDER_BE_H
#define AQBANKING_PROVIDER_BE_H

#include <aqbanking/provider.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef AB_PROVIDER* (*AB_PROVIDER_FACTORY_FN)(AB_BANKING *ab,
                                               GWEN_DB_NODE *db);

/** @name Prototypes For Virtual Functions
 *
 */
/*@{*/
/**
 * Allow the backend to initialize itself.
 */
typedef int (*AB_PROVIDER_INIT_FN)(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);

/**
 * Allow the backend to deinitialize itself.
 */
typedef int (*AB_PROVIDER_FINI_FN)(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);

/**
 * This function should check for the availability of the given job and
 * prepare it for the next call to @ref AB_PROVIDER_ADDJOB_FN.
 * If the job is available with this backend it should also set the job
 * parameters (such as the maximum number of purpose lines for transfer jobs
 * etc).
 * This function is called from the constructor AB_Job_new() in AqBanking.
 */
typedef int (*AB_PROVIDER_UPDATEJOB_FN)(AB_PROVIDER *pro, AB_JOB *j);

/**
 * Add the given job to the backend's internal queue. This is an immediate
 * queue, it is not persistent. The queue is flushed by
 * @ref AB_PROVIDER_EXECUTE_FN. The added job is removed in any case
 * after @ref AB_PROVIDER_EXECUTE_FN has been called.
 */
typedef int (*AB_PROVIDER_ADDJOB_FN)(AB_PROVIDER *pro, AB_JOB *j);

/**
 * Executes all jobs in the queue which have just been added via
 * @ref AB_PROVIDER_ADDJOB_FN. After calling this function @b all jobs are
 * removed from the backend's queue in any case.
 */
typedef int (*AB_PROVIDER_EXECUTE_FN)(AB_PROVIDER *pro);

/**
 * Returns a list of accounts managed by this backend. This is called by
 * AqBanking directly after activating a backend (upon every
 * @ref AB_Banking_Init).
 */
typedef AB_ACCOUNT_LIST2* (*AB_PROVIDER_GETACCOUNTLIST_FN)(AB_PROVIDER *pro);

/**
 * Gives the backend the opportunity to update account information (such as
 * account name etc).
 */
typedef int (*AB_PROVIDER_UPDATEACCOUNT_FN)(AB_PROVIDER *pro, AB_ACCOUNT *a);

/**
 * This function is currently unused. It may later be used to add an account
 * to the backend which has been prepared by an application.
 */
typedef int (*AB_PROVIDER_ADDACCOUNT_FN)(AB_PROVIDER *pro, AB_ACCOUNT *a);
/*@}*/




AQBANKING_API
AB_PROVIDER *AB_Provider_new(AB_BANKING *ab,
                             const char *name);

int AB_Provider_IsInit(const AB_PROVIDER *pro);


/** @name Setters For Virtual Functions
 *
 */
/*@{*/
AQBANKING_API
void AB_Provider_SetInitFn(AB_PROVIDER *pro, AB_PROVIDER_INIT_FN f);
AQBANKING_API
void AB_Provider_SetFiniFn(AB_PROVIDER *pro, AB_PROVIDER_FINI_FN f);

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
/*@}*/


#ifdef __cplusplus
}
#endif




#endif /* AQBANKING_PROVIDER_BE_H */









