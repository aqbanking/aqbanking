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


#ifndef AQBANKING_BANKING_OB_H
#define AQBANKING_BANKING_OB_H

#include <aqbanking/provider.h>
#include <aqbanking/user.h>
#include <aqbanking/job.h>


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup G_AB_ONLINE_BANKING
 *
 */
/*@{*/

/** @name User Management Functions
 *
 * AqBanking controls a list of users. You can ask it for the full list
 * (@ref AB_Banking_GetUsers) or directly request a specific account
 * (@ref AB_Banking_GetUser).
 * AB_USERs contain all information needed to identify a user to the bank's
 * server.
 */
/*@{*/

AQBANKING_API
AB_USER_LIST2 *AB_Banking_GetUsers(const AB_BANKING *ab);

/**
 * Returns the user with the given unique id.
 */
AQBANKING_API
AB_USER *AB_Banking_GetUser(const AB_BANKING *ab, GWEN_TYPE_UINT32 uniqueId);


/**
 * This function returns the first user which matches the given parameters.
 * For all parameters wildcards ("*") and joker ("?") are allowed.
 */
AQBANKING_API
AB_USER *AB_Banking_FindUser(const AB_BANKING *ab,
                             const char *backendName,
                             const char *country,
                             const char *bankId,
                             const char *userId,
                             const char *customerId);

/**
 * This function returns the a list of users which match the given parameters.
 * For all parameters wildcards ("*") and joker ("?") are allowed.
 * If no user matches (or there simply are no users) then NULL is returned.
 * The caller is responsible for freeing the list returned (ifany) by calling
 * @ref AB_User_List2_free.
 * AqBanking still remains the owner of every user reported via this
 * function, so you MUST NOT call @ref AB_User_List2_freeAll.
 */
AQBANKING_API
AB_USER_LIST2 *AB_Banking_FindUsers(const AB_BANKING *ab,
				    const char *backendName,
                                    const char *country,
                                    const char *bankId,
				    const char *userId,
				    const char *customerId);

/**
 * Creates a user and presents it to the backend (which might want to extend
 * the newly created user in order to associate some data with it).
 */
AQBANKING_API
AB_USER *AB_Banking_CreateUser(AB_BANKING *ab, const char *backendName);

/**
 * Enqueues the given user with AqBanking.
 */
AQBANKING_API
int AB_Banking_AddUser(AB_BANKING *ab, AB_USER *u);
/*@}*/



/** @name Account Management Functions
 *
 * AqBanking controls a list of accounts. You can ask it for the full list
 * (@ref AB_Banking_GetAccounts) or directly request a specific account
 * (@ref AB_Banking_GetAccount).
 */
/*@{*/
/**
 * Returns a list of currently known accounts, or NULL if there are no
 * accounts. The returned list is owned by the caller, so he is
 * responsible for freeing it (using @ref AB_Account_List2_free).
 *
 * Please note that even while the list is owned by the caller the accounts
 * in that list are not! Sou you may not free any of those accounts in the
 * list (e.g. by calling @ref AB_Account_List2_FreeAll).
 *
 * @return The list of accounts, or NULL if there are none.
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API 
AB_ACCOUNT_LIST2 *AB_Banking_GetAccounts(const AB_BANKING *ab);

/**
 * This function does an account lookup based on the given unique id.
 * This id is assigned by AqBanking when an account is added to AqBanking
 * via @ref AB_Banking_AddAccount.
 *
 * AqBanking remains the owner of the object returned (if any), so you must
 * not free it.
 *
 * Please also note that the object returned is only valid until
 * @ref AB_Banking_Fini() has been called (or until the corresponding backend
 * for this particular account has been deactivated).
 *
 * @return The account, or NULL if it is not found.
 * @param ab pointer to the AB_BANKING object
 * @param uniqueId unique id of the account assigned by AqBanking
 */
AQBANKING_API 
AB_ACCOUNT *AB_Banking_GetAccount(const AB_BANKING *ab,
                                  GWEN_TYPE_UINT32 uniqueId);

/**
 * This function does an account lookup based on the given bank code and
 * account number. No wildards or jokers allowed.
 *
 * AqBanking remains the owner of the object returned (if any), so you must
 * not free it.
 *
 * Please also note that the object returned is only valid until
 * @ref AB_Banking_Fini() has been called (or until the corresponding backend
 * for this particular account has been deactivated).
 *
 * @return The account, or NULL if it is not found.
 * @param ab pointer to the AB_BANKING object
 * @param bankCode bank code (use 0 if your country does not use bank codes)
 * @param accountId account number
 */
AQBANKING_API 
AB_ACCOUNT *AB_Banking_GetAccountByCodeAndNumber(const AB_BANKING *ab,
                                                 const char *bankCode,
                                                 const char *accountId);

/**
 * This function returns the first account which matches the given parameters.
 * For all parameters wildcards ("*") and joker ("?") are allowed.
 */
AQBANKING_API
AB_ACCOUNT *AB_Banking_FindAccount(const AB_BANKING *ab,
                                   const char *backendName,
                                   const char *country,
                                   const char *bankId,
                                   const char *accountId);

/**
 * This function returns the a list of accounts which match the given
 * parameters.
 * For all parameters wildcards ("*") and joker ("?") are allowed.
 * If no account matches (or there simply are no accounts) then NULL is
 * returned.
 * The caller is responsible for freeing the list returned (ifany) by calling
 * @ref AB_Account_List2_free.
 * AqBanking still remains the owner of every account reported via this
 * function, so you MUST NOT call @ref AB_Account_List2_FreeAll.
 */
AQBANKING_API
AB_ACCOUNT_LIST2 *AB_Banking_FindAccounts(const AB_BANKING *ab,
                                          const char *backendName,
                                          const char *country,
                                          const char *bankId,
                                          const char *accountId);

/**
 * Creates an account and shows it to the backend (which might want to extend
 * the newly created account in order to associate some data with it).
 * The newly created account does not have a unique id yet. This id is
 * assigned upon @ref AB_Banking_AddAccount. The caller becomes the owner
 * of the object returned, so you must either call @ref AB_Banking_AddAccount
 * or @ref AB_Account_free on it.
 */
AQBANKING_API 
AB_ACCOUNT *AB_Banking_CreateAccount(AB_BANKING *ab, const char *backendName);

/**
 * Adds the given account to the internal list of accounts. Only now it gets a
 * unique id assigned to it.
 * AqBanking takes over the ownership of the given account, so you MUST NOT
 * call @ref AB_Account_free on it!
 */
AQBANKING_API 
int AB_Banking_AddAccount(AB_BANKING *ab, AB_ACCOUNT *a);
/*@}*/



/** @name Enqueueing, Dequeueing and Executing Jobs
 *
 * <p>
 * AqBanking has several job lists:
 * </p>
 * <ul>
 *  <li><b>enqueued jobs</b></li>
 *  <li>finished jobs</li>
 *  <li>pending jobs</li>
 *  <li>archived jobs</li>
 * </ul>
 * <p>
 * Enqued jobs are different from all the other jobs, because AqBanking
 * holds a unique list of those enqueued jobs.
 * </p>
 * <p>
 * For example if you ask AqBanking for a list of finished jobs it reads the
 * folder which contains those jobs and loads them one by one. This means
 * if you call @ref AB_Banking_GetFinishedJobs multiple times you get
 * multiple representations of the finished jobs. You should have this in mind
 * when manipulating those jobs/job lists returned.
 * </p>
 * <p>
 * <b>Enqueued</b> jobs however have only <b>one</b> representation, i.e. if
 * you call @ref AB_Banking_GetEnqueuedJobs multiple times you will always get
 * a list which contains pointers to the very same enqueued jobs.
 * </p>
 * <p>
 * However, if you enqueue a job, execute the queue and later call
 * @ref AB_Banking_GetFinishedJobs you will again have multiple
 * representations of the jobs you once had in the enqueued list, because
 * @ref AB_Banking_GetFinishedJobs always creates a new
 * representation of a job.
 * </p>
 * <p>
 * Enqueued jobs are preserved across shutdowns. As soon as a job has been
 * sent to the appropriate backend it will be removed from the queue.
 * </p>
 */
/*@{*/
/**
 * Enqueues a job. This function does not take over the ownership of the
 * job. However, this function makes sure that the job will not be deleted
 * as long as it is in the queue (by calling @ref AB_Job_Attach).
 * So it is safe for you to call @ref AB_Job_free on an enqueued job directly
 * after enqueuing it (but it doesn't make much sense since you would not be
 * able to check for a result).
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param j job to be enqueued
 *
 */
AQBANKING_API 
int AB_Banking_EnqueueJob(AB_BANKING *ab, AB_JOB *j);

/**
 * Removes a job from the queue. This function does not free the given
 * job, the caller still is the owner.
 * Dequeued jobs however are NOT preserved across shutdowns.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param j job to be dequeued
 */
AQBANKING_API 
int AB_Banking_DequeueJob(AB_BANKING *ab, AB_JOB *j);


/**
 * This function enqueues all pending jobs so that they will be send the
 * next time @ref AB_Banking_ExecuteQueue is called.
 * You should call this function directly before calling
 * @ref AB_Banking_ExecuteQueue to let the backend update the status of
 * the pending jobs.
 * There is special treatment for "pending" marked jobs in backends: The
 * backends check for the final result of them instead of executing them
 * again when added to the queue. This is used by jobs which do not return
 * an immediate result (such as transfer jobs which are first accepted by the
 * bank and actually checked later. In such a case the bank sends a temporary
 * result code which might be replaced by a final result later).
 * @param ab pointer to the AB_BANKING object
 * @param mineOnly if 0 then all pending jobs for all applications are
 * enqueued, otherwise only the pending jobs for the currently running
 * application are enqueued
 */
AQBANKING_API 
int AB_Banking_EnqueuePendingJobs(AB_BANKING *ab, int mineOnly);

/**
 * <p>
 * This function sends all jobs in the queue to their corresponding backends
 * and allows those backends to process them.
 * </p>
 * <p>
 * The queue is always empty upon return.
 * </p>
 * <p>
 * Jobs which have been finished (even errornous jobs) are moved from the
 * queue to the list of finished jobs. Those jobs are preserved across
 * shutdowns.
 * </p>
 * <p>
 * This means that if you are handling the response for a just
 * executed job directly after queue execution you should remove the
 * finished job by calling @ref AB_Banking_DelFinishedJob on it after handling
 * its results.
 * </p>
 * <p>
 * If a job is marked as <i>pending</i> after execution it will be moved to
 * the list of pending jobs, so @ref AB_Banking_DelFinishedJob will not work
 * on this job.
 * Those jobs are also preserved across shutdowns.
 * </p>
 * <p>
 * You should call @ref AB_Banking_Save() before calling this function here
 * to make sure that a crash in a backend will not destroy too much data.
 * </p>
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API AQBANKING_DEPRECATED
int AB_Banking_ExecuteQueue(AB_BANKING *ab);

AQBANKING_API 
int AB_Banking_ExecuteQueueWithCtx(AB_BANKING *ab,
                                   AB_IMEXPORTER_CONTEXT *ctx);

/**
 * <p>
 * This function sends all enqueued jobs from the given list to their
 * respective backend.
 * </p>
 * <p>
 * The jobs in the list MUST BE enqueued jobs! It is not allowed to use any
 * arbitrary jobs.
 * </p>
 * <p>
 * The purpose of this function is to let the application directly
 * influence the list of jobs to be executed. For instance an application
 * might decide to only execute it's own jobs, or only jobs for a special
 * account etc.
 * </p>
 * <p>
 * The way to use this function is to call @ref AB_Banking_GetEnqueuedJobs,
 * select the jobs out of the list returned (or the whole list) and use this
 * list.
 * </p>
 * <p>
 * This function calls @ref AB_Job_free on every one of the jobs in the
 * given list (because there is a central list of enqueued jobs).
 * So if you are still interested in the jobs in the list after this function
 * has been called you should call @ref AB_Job_Attach on those jobs you want
 * to keep.
 * </p>
 * <p>
 * Also, upon return all jobs in this list are removed from the todo queue.
 * For the rest see info for function @ref AB_Banking_ExecuteQueue.
 * </p>
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param jl2 list of enqueued jobs to execute
 */
AQBANKING_API AQBANKING_DEPRECATED
int AB_Banking_ExecuteJobList(AB_BANKING *ab, AB_JOB_LIST2 *jl2);

AQBANKING_API 
int AB_Banking_ExecuteJobListWithCtx(AB_BANKING *ab, AB_JOB_LIST2 *jl2,
				     AB_IMEXPORTER_CONTEXT *ctx);

/**
 * <p>
 * Returns a new list which contains the currently enqueued jobs.
 * If the queue is empty NULL is returned.
 * </p>
 * <p>
 * Please note that AqBanking still remains the owner of those jobs in the
 * list returned (if any). So you MUST NOT call @ref AB_Job_List2_FreeAll on
 * this list. However, you MUST call @ref AB_Job_List2_free when you no
 * longer need the @b list to avoid memory leaks.
 * </p>
 * <p>
 * This list is only valid until one of the following functions is called:
 * </p>
 * <ul>
 *  <li>@ref AB_Banking_ExecuteQueue</li>
 *  <li>@ref AB_Banking_ExecuteQueueWithCtx</li>
 *  <li>@ref AB_Banking_Fini</li>
 * </ul>
 * After one of these functions has been called you are only allowed to call
 * @ref AB_Job_List2_free on that list, the elements of this list are no
 * longer valid.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API 
AB_JOB_LIST2 *AB_Banking_GetEnqueuedJobs(const AB_BANKING *ab);
/*@}*/



/** @name Handling Finished Jobs
 *
 * <p>
 * Finished jobs are those which have been handled by
 * @ref AB_Banking_ExecuteQueue.
 * </p>
 * <p>
 * Those jobs are saved into their particular folder directly after the queue
 * has been executed. There is only one function you can call for those
 * jobs: @ref AB_Banking_DelFinishedJob.
 * </p>
 * <p>
 * Applications might use this group to check for the results of jobs formerly
 * marked as <i>pending</i> and to apply these results to the corresponding
 * accounts.
 * </p>
 */
/*@{*/

/**
 * <p>
 * Loads all finished jobs from their folder. The caller is responsible for
 * freeing the jobs returned (as opposed to @ref AB_Banking_GetEnqueuedJobs).
 * </p>
 * <p>
 * Please note that since this function loads all jobs from their folder
 * the returned list might contain another representation of jobs you once
 * created and enqueued into the execution queue.
 * </p>
 */
AQBANKING_API 
AB_JOB_LIST2 *AB_Banking_GetFinishedJobs(AB_BANKING *ab);

/**
 * Removes a finished job from its folder. You can use either a job returned
 * via @ref AB_Banking_GetFinishedJobs or a job you previously added to
 * the execution queue after the queue has been executed.
 */
AQBANKING_API 
int AB_Banking_DelFinishedJob(AB_BANKING *ab, AB_JOB *j);

/*@}*/



/** @name Handling Pending Jobs
 *
 * <p>
 * Pending jobs are those which have been handled by
 * @ref AB_Banking_ExecuteQueue but which did not yet return a result (like
 * transfer jobs with some backends, which might be accepted by the bank but
 * which are not immediately executed).
 * </p>
 * <p>
 * Those jobs are saved into their particular folder directly after the queue
 * has been executed. You may requeue those jobs later (using
 * @ref AB_Banking_EnqueueJob)
 * to allow the corresponding backend to check whether there is a result
 * available for a pending job. If that's the case the job will be moved
 * from the <i>pending</i> folder to the <i>finished</i> folder (accessible
 * via @ref AB_Banking_GetFinishedJobs).
 * </p>
 */
/*@{*/

/**
 * <p>
 * Loads all pending jobs from their folder. The caller is responsible for
 * freeing the jobs returned (as opposed to @ref AB_Banking_GetEnqueuedJobs).
 * </p>
 * <p>
 * Please note that since this function loads all jobs from their folder
 * the returned list might contain another representation of jobs you once
 * created and enqueued into the execution queue.
 * </p>
 * <p>
 * So you should only use jobs returned by this function for other functions
 * in this particular function group.
 * </p>
 */
AQBANKING_API 
AB_JOB_LIST2 *AB_Banking_GetPendingJobs(AB_BANKING *ab);

/**
 * Removes a pending job from its folder. This function does not free the job.
 * The job MUST be one of those returned via @ref AB_Banking_GetPendingJobs.
 */
AQBANKING_API 
int AB_Banking_DelPendingJob(AB_BANKING *ab, AB_JOB *j);

/*@}*/


/** @name Handling Archived Jobs
 *
 * <p>
 * Archived jobs are those which have been handled by
 * @ref AB_Banking_ExecuteQueue and then later deleted from the list of
 * finished jobs via any AB_Banking_Del(-XYZ-)Job function except
 * @ref AB_Banking_DelArchivedJob
 * </p>
 */
/*@{*/

/**
 * <p>
 * Loads all archived jobs from their folder. The caller is responsible for
 * freeing the jobs returned (as opposed to @ref AB_Banking_GetEnqueuedJobs).
 * </p>
 * <p>
 * Archived jobs are jobs which have been deleted via
 * any AB_Banking_Del(-XYZ-)Job function except
 * @ref AB_Banking_DelArchivedJob
 * </p>
 * <p>
 * Please note that since this function loads all jobs from their folder
 * the returned list might contain another representation of jobs you once
 * created and enqueued into the execution queue.
 * </p>
 */
AQBANKING_API 
AB_JOB_LIST2 *AB_Banking_GetArchivedJobs(AB_BANKING *ab);

/**
 * Removes a finished job from its folder. You can use either a job returned
 * via @ref AB_Banking_GetFinishedJobs or a job you previously added to
 * the execution queue after the queue has been executed.
 */
AQBANKING_API 
int AB_Banking_DelArchivedJob(AB_BANKING *ab, AB_JOB *j);
/*@}*/



/*@}*/ /* addtogroup */

#ifdef __cplusplus
}
#endif

#endif

