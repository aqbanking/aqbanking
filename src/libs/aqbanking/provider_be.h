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
#include <aqbanking/user.h>


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup G_AB_PROVIDER
 *
 */
/*@{*/

/**
 * <p>
 * This is the prototype for the provider factory function expected in
 * provider plugins.
 * Every provider plugin must have at least this function which returns
 * a new provider object.
 * </p>
 * <p>
 * This function must contain the name of the provider in all-lowercase
 * letters. For AqHBCI this function is called "aqhbci_factory".
 * </p>
 * @param ab AqBanking main object
 */
typedef AB_PROVIDER* (*AB_PROVIDER_FACTORY_FN)(AB_BANKING *ab);


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
  AB_ProviderExtendMode_Save
} AB_PROVIDER_EXTEND_MODE;


/** @name Prototypes For Virtual Functions
 *
 */
/*@{*/
/**
 * See @ref AB_Provider_Init.
 */
typedef int (*AB_PROVIDER_INIT_FN)(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);

/**
 * See @ref AB_Provider_Fini.
 */
typedef int (*AB_PROVIDER_FINI_FN)(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);

/**
 * See @ref AB_Provider_UpdateJob
 */
typedef int (*AB_PROVIDER_UPDATEJOB_FN)(AB_PROVIDER *pro, AB_JOB *j);

/**
 * See @ref AB_Provider_AddJob.
 */
typedef int (*AB_PROVIDER_ADDJOB_FN)(AB_PROVIDER *pro, AB_JOB *j);

/**
 * See @ref AB_Provider_Execute
 */
typedef int (*AB_PROVIDER_EXECUTE_FN)(AB_PROVIDER *pro);


/**
 * See @ref AB_Provider_ResetQueue.
 */
typedef int (*AB_PROVIDER_RESETQUEUE_FN)(AB_PROVIDER *pro);


/**
 * See @ref AB_Provider_ExtendUser.
 */
typedef int (*AB_PROVIDER_EXTEND_USER_FN)(AB_PROVIDER *pro, AB_USER *u,
                                          AB_PROVIDER_EXTEND_MODE um);


/**
 * See @ref AB_Provider_ExtendAccount.
 */
typedef int (*AB_PROVIDER_EXTEND_ACCOUNT_FN)(AB_PROVIDER *pro,
                                             AB_ACCOUNT *a,
                                             AB_PROVIDER_EXTEND_MODE um);

typedef int (*AB_PROVIDER_UPDATE_FN)(AB_PROVIDER *pro,
                                     GWEN_TYPE_UINT32 lastVersion,
                                     GWEN_TYPE_UINT32 currentVersion);


/*@}*/





AQBANKING_API
AB_PROVIDER *AB_Provider_new(AB_BANKING *ab,
                             const char *name);

AQBANKING_API
int AB_Provider_IsInit(const AB_PROVIDER *pro);


/** @name Virtual Functions
 *
 */
/*@{*/

/**
 * Allow the backend to initialize itself.
 * @param pro backend object
 * @param dbData GWEN_DB group for the data of the backend (as returned by
 *   @ref AB_Provider_GetData). This group MUST NOT be freed or unlinked, it
 *   is a group inside AqBankings config database.
 */
AQBANKING_API
int AB_Provider_Init(AB_PROVIDER *pro);

/**
 * Allow the backend to deinitialize itself.
 * @param pro backend object
 * @param dbData GWEN_DB group for the data of the backend (as returned by
 *   @ref AB_Provider_GetData). This group MUST NOT be freed or unlinked, it
 *   is a group inside AqBankings config database.
 */
AQBANKING_API
int AB_Provider_Fini(AB_PROVIDER *pro);

/**
 * This function should check for the availability of the given job and
 * prepare it for the next call to @ref AB_PROVIDER_ADDJOB_FN.
 * If the job is available with this backend it should also set the job
 * parameters (such as the maximum number of purpose lines for transfer jobs
 * etc).
 * This function is called from the constructor AB_Job_new() in AqBanking.
 * The value returned here is stored within the job in question and becomes
 * available via @ref AB_Job_CheckAvailability.
 * @param pro backend object
 */
AQBANKING_API
int AB_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);

/**
 * <p>
 * Add the given job to the backend's internal queue. This is an immediate
 * queue, it is not persistent. The queue is flushed by
 * @ref AB_PROVIDER_EXECUTE_FN. The added job is removed in any case
 * after @ref AB_PROVIDER_EXECUTE_FN has been called.
 * </p>
 * <p>
 * This function should first check the job arguments (sanity checks etc).
 * If this function returns an error the job MUST NOT be enqueued in the
 * providers own queue. In this case the job will be marked "errornous".
 * </p>
 * <p>
 * However, if the backend prepares the job well enough (via
 * @ref AB_PROVIDER_UPDATEJOB_FN) then the application should have made sure
 * that the job complies to the rules laid out by the backend. So rejecting
 * a job here should be a rare case with well-designed applications and
 * backends.
 * </p>
 * @param pro backend object
 */
AQBANKING_API
int AB_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);

/**
 * Executes all jobs in the queue which have just been added via
 * @ref AB_PROVIDER_ADDJOB_FN. After calling this function @b all jobs are
 * removed from the backend's queue in any case.
 * @param pro backend object
 */
AQBANKING_API
int AB_Provider_Execute(AB_PROVIDER *pro);

/**
 * Resets the queue of the backend.
 * After calling this function @b all jobs are removed from the
 * backend's queue in any case.
 * @param pro backend object
 */
AQBANKING_API
int AB_Provider_ResetQueue(AB_PROVIDER *pro);


/**
 * Allows the backend to extend the given user (e.g. load backend-specific
 * data for the given user).
 */
AQBANKING_API
int AB_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
                           AB_PROVIDER_EXTEND_MODE em);


/**
 * Allows the backend to extend the given account (e.g. load backend-specific
 * data for the given account).
 */
AQBANKING_API
int AB_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
                              AB_PROVIDER_EXTEND_MODE em);


/**
 * Allows the backend to update AqBanking data.
 * This function is called for each active provider after all backends, users
 * and accounts have been loaded and initialised but before loading the
 * outbox jobs.
 */
AQBANKING_API
int AB_Provider_Update(AB_PROVIDER *pro,
                       GWEN_TYPE_UINT32 lastVersion,
                       GWEN_TYPE_UINT32 currentVersion);


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
void AB_Provider_SetUpdateJobFn(AB_PROVIDER *pro, AB_PROVIDER_UPDATEJOB_FN f);
AQBANKING_API
void AB_Provider_SetAddJobFn(AB_PROVIDER *pro, AB_PROVIDER_ADDJOB_FN f);
AQBANKING_API
void AB_Provider_SetExecuteFn(AB_PROVIDER *pro, AB_PROVIDER_EXECUTE_FN f);
AQBANKING_API
void AB_Provider_SetResetQueueFn(AB_PROVIDER *pro, AB_PROVIDER_RESETQUEUE_FN f);

AQBANKING_API
void AB_Provider_SetExtendUserFn(AB_PROVIDER *pro,
                                 AB_PROVIDER_EXTEND_USER_FN f);

AQBANKING_API
void AB_Provider_SetExtendAccountFn(AB_PROVIDER *pro,
                                    AB_PROVIDER_EXTEND_ACCOUNT_FN f);

AQBANKING_API
void AB_Provider_SetUpdateFn(AB_PROVIDER *pro, AB_PROVIDER_UPDATE_FN f);


/*@}*/


/*@}*/ /* addtogroup */


#ifdef __cplusplus
}
#endif




#endif /* AQBANKING_PROVIDER_BE_H */









