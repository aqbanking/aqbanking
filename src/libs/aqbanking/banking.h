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


#ifndef AQBANKING_BANKING_H
#define AQBANKING_BANKING_H

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/plugindescr.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup AB_BANKING AB_BANKING (Main Interface)
 * @ingroup AB_C_INTERFACE
 *
 */
/*@{*/

#define AB_BANKING_CONFIGFILE ".aqbanking.conf"

#define AB_BANKING_PROGRESS_NONE 0xffffffff


typedef struct AB_BANKING AB_BANKING;
GWEN_INHERIT_FUNCTION_DEFS(AB_BANKING);

/** @name Flags For AB_Banking_InputBox
 *
 */
/*@{*/
#define AB_BANKING_INPUT_FLAGS_CONFIRM 0x00000001
#define AB_BANKING_INPUT_FLAGS_SHOW    0x00000002
#define AB_BANKING_INPUT_FLAGS_NUMERIC 0x00000004
/*@}*/



typedef enum {
  AB_Banking_MsgTypeInfo=0,
  AB_Banking_MsgTypeWarn,
  AB_Banking_MsgTypeError
} AB_BANKING_MSGTYPE;


typedef enum {
  AB_Banking_LogLevelPanic=0,
  AB_Banking_LogLevelEmergency,
  AB_Banking_LogLevelError,
  AB_Banking_LogLevelWarn,
  AB_Banking_LogLevelNotice,
  AB_Banking_LogLevelInfo,
  AB_Banking_LogLevelDebug,
  AB_Banking_LogLevelVerbous
} AB_BANKING_LOGLEVEL;

#ifdef __cplusplus
}
#endif


#include <aqbanking/job.h>
#include <aqbanking/provider.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @name Prototypes For Virtual User Interaction Functions
 *
 */
/*@{*/
typedef int (*AB_BANKING_MESSAGEBOX_FN)(AB_BANKING *ab,
                                        AB_BANKING_MSGTYPE mt,
                                        const char *title,
                                        const char *text,
                                        const char *b1,
                                        const char *b2,
                                        const char *b3);

typedef int (*AB_BANKING_INPUTBOX_FN)(AB_BANKING *ab, 
                                      const char *title,
                                      const char *text,
                                      char *buffer,
                                      int minLen,
                                      int maxLen,
                                      GWEN_TYPE_UINT32 flags);

typedef GWEN_TYPE_UINT32 (*AB_BANKING_SHOWBOX_FN)(AB_BANKING *ab, 
                                                  const char *title,
                                                  const char *text);
typedef void (*AB_BANKING_HIDEBOX_FN)(AB_BANKING *ab, GWEN_TYPE_UINT32 id);

typedef GWEN_TYPE_UINT32
  (*AB_BANKING_PROGRESS_START_FN)(AB_BANKING *ab, 
                                  const char *title,
                                  const char *text,
                                  GWEN_TYPE_UINT32 total);

typedef int (*AB_BANKING_PROGRESS_ADVANCE_FN)(AB_BANKING *ab, 
                                              GWEN_TYPE_UINT32 id,
                                              GWEN_TYPE_UINT32 progress);
typedef int (*AB_BANKING_PROGRESS_LOG_FN)(AB_BANKING *ab, 
                                          GWEN_TYPE_UINT32 id,
                                          AB_BANKING_LOGLEVEL level,
                                          const char *text);
typedef int (*AB_BANKING_PROGRESS_END_FN)(AB_BANKING *ab, 
                                          GWEN_TYPE_UINT32 id);
/*@}*/






/** @name Constructor, Destructor, Init, Fini
 *
 */
/*@{*/
AB_BANKING *AB_Banking_new(const char *appName,
                           const char *fname);
void AB_Banking_free(AB_BANKING *ab);

int AB_Banking_Init(AB_BANKING *ab);
int AB_Banking_Fini(AB_BANKING *ab);
/*@}*/


/** @name Working With Backends
 *
 */
/*@{*/
/**
 * This function loads the given backend (if it not already has been) and
 * imports any account that backend might offer. You can use this function
 * to engage a backend which has not yet been used (but it doesn't hurt if you
 * use it on already active backends).
 */
int AB_Banking_ImportProviderAccounts(AB_BANKING *ab, const char *backend);

int AB_Banking_ActivateProvider(AB_BANKING *ab, const char *pname);
int AB_Banking_DeactivateProvider(AB_BANKING *ab, const char *pname);
const GWEN_STRINGLIST *AB_Banking_GetActiveProviders(const AB_BANKING *ab);

/**
 * Tries to load the wizard for the given backend which is of the given
 * type t.
 * Setup wizards are sorted by backends, since the wizard do things
 * very much dependant on the backend. Ideally they are shipped with the
 * backend.
 * @param ab pointer to the AB_BANKING object
 * @param pn name of the backend
 * @param t wizard type. To allow keeping the API as open as possible you
 * may give a type name here. However, the following names are expected:
 * <ul>
 *  <li><b>kde</b> for a wizard running under KDE</li>
 *  <li><b>gnome</b> for a wizard running under GNOME</li>
 *  <li><b>console</b> for a wizard running in a console</li>
 *  <li><b>curses</b> for a wizard using (n)curses</li>
 * </ul>
 */
AB_PROVIDER_WIZARD *AB_Banking_GetWizard(AB_BANKING *ab,
                                           const char *pn,
                                           const char *t);
/*@}*/




/** @name Application Data
 *
 */
/*@{*/
/**
 * Returns the application name as given to @ref AB_Banking_new.
 */
const char *AB_Banking_GetAppName(const AB_BANKING *ab);

/**
 * Returns a GWEN_DB_NODE which can be used to store/retrieve data for
 * the currently running application. The group returned MUST NOT be
 * freed !
 * AqBanking is able to separate and store the data for every application.
 */
GWEN_DB_NODE *AB_Banking_GetAppData(AB_BANKING *ab);

/**
 * Returns the name of the user folder for AqBanking's data.
 * Normally this is something like "/home/me/.banking".
 */
int AB_Banking_GetUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf);

/**
 * Returns the name of the user folder for application data.
 * Normally this is something like "/home/me/.banking/apps".
 * Your application may choose to create folders below this one to store
 * user data. If you only add AqBanking to an existing program to add
 * home banking support you will most likely use your own folders and thus
 * won't need this function.
 */
int AB_Banking_GetAppUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf);


/*@}*/



/** @name Working With Accounts
 *
 */
/*@{*/
/**
 * Returns a list of currently known accounts. The returned list is
 * owned by the caller, so he is responsible for freeing it (using
 * @ref AB_Account_List2_free).
 */
AB_ACCOUNT_LIST2 *AB_Banking_GetAccounts(const AB_BANKING *ab);

/**
 * This function does an account lookup based on the given unique id.
 * This id is assigned by AqBanking when an account is created.
 */
AB_ACCOUNT *AB_Banking_GetAccount(const AB_BANKING *ab,
                                  GWEN_TYPE_UINT32 uniqueId);
/*@}*/



/** @name Plugin Handling
 *
 */
/*@{*/
/**
 * Returns a list2 of provider descriptions. You must free this list after
 * using it via @ref GWEN_PluginDescription_List2_freeAll.
 * Please note that a simple @ref GWEN_PluginDescription_List2_free would
 * not suffice, since that would only free the list but not the objects
 * stored within the list !
 */
GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetProviderDescrs(AB_BANKING *ab);


/**
 * Returns a list2 of wizard descriptions for the given backend.
 * You must free this list after using it via
 * @ref GWEN_PluginDescription_List2_freeAll.
 * Please note that a simple @ref GWEN_PluginDescription_List2_free would
 * not suffice, since that would only free the list but not the objects
 * stored within the list !
 */
GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetWizardDescrs(AB_BANKING *ab,
                                                          const char *pn);

/*@}*/




/** @name Functions Used by Backends
 *
 */
/*@{*/
/**
 * This is used by backends to store their specific data with AqBanking.
 */
GWEN_DB_NODE *AB_Banking_GetProviderData(AB_BANKING *ab,
                                         const AB_PROVIDER *pro);

/**
 * This copies the name of the folder for AqBanking's backend data into
 * the given GWEN_Buffer.
 */
int AB_Banking_GetProviderUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf);

/*@}*/




/** @name Enqueueing, Dequeueing and Executing Jobs
 *
 * Enqueued jobs are preserved across shutdowns. As soon as a job has been
 * sent to the appropriate backend it will be removed from the queue.
 * Only those jobs are saved/reloaded which have been enqueued but never
 * presented to the backend. This means after calling
 * @ref AB_Banking_ExecuteQueue only those jobs are still in the queue which
 * have not been processed (e.g. because they belonged to a second backend
 * but the user aborted while the jobs for a first backend were in process).
 */
/*@{*/
/**
 * Enqueues a job. This function does not take over the ownership of the
 * job. However, this function makes sure that the job will not be deleted
 * as long as it is in the queue (by calling @ref AB_Job_Attach).
 * So it is safe for you to call @ref AB_Job_free on an enqueued job directly
 * after enqueuing it (but it doesn't make much sense since you would not be able to
 * check for a result).
 *
 */
int AB_Banking_EnqueueJob(AB_BANKING *ab, AB_JOB *j);

/**
 * Removes a job from the queue. This function does not free the given
 * job, the caller still is the owner.
 * Dequeued jobs however are NOT preserved across shutdowns.
 */
int AB_Banking_DequeueJob(AB_BANKING *ab, AB_JOB *j);

/**
 * This function sends all jobs in the queue to their corresponding backends
 * and allows that backend to process it.
 * If the user did not abort or there was no fatal error the queue is
 * empty upon return. You can verify this by calling
 * @ref AB_Banking_GetEnqueuedJobs.
 */
int AB_Banking_ExecuteQueue(AB_BANKING *ab);

/**
 * Returns the list of currently enqueued jobs. If the queue is empty
 * NULL is returned.
 */
AB_JOB_LIST2 *AB_Banking_GetEnqueuedJobs(const AB_BANKING *ab);
/*@}*/




/** @name Virtual User Interaction Functions
 *
 */
/*@{*/
int AB_Banking_MessageBox(AB_BANKING *ab,
                          AB_BANKING_MSGTYPE mt,
                          const char *title,
                          const char *text,
                          const char *b1,
                          const char *b2,
                          const char *b3);

/**
 * @param maxLen size of the buffer -1 (the -1 is to allow for the trailing
 * null character to be appended)
 */
int AB_Banking_InputBox(AB_BANKING *ab,
                        const char *title,
                        const char *text,
                        char *buffer,
                        int minLen,
                        int maxLen,
                        GWEN_TYPE_UINT32 flags);

GWEN_TYPE_UINT32 AB_Banking_ShowBox(AB_BANKING *ab,
                                    const char *title,
                                    const char *text);
void AB_Banking_HideBox(AB_BANKING *ab,GWEN_TYPE_UINT32 id);

GWEN_TYPE_UINT32 AB_Banking_ProgressStart(AB_BANKING *ab,
                                          const char *title,
                                          const char *text,
                                          GWEN_TYPE_UINT32 total);

/**
 * Advances the progress bar an application might present to the user.
 * @param id id assigned by @ref AB_Banking_ProgressStart
 * @param progress new value for progress. A special value is
 *  AB_BANKING_PROGRESS_NONE which means that the progress is unchanged.
 * This might be used as a keepalive call to a GUI.
 */
int AB_Banking_ProgressAdvance(AB_BANKING *ab,
                               GWEN_TYPE_UINT32 id,
                               GWEN_TYPE_UINT32 progress);
int AB_Banking_ProgressLog(AB_BANKING *ab,
                           GWEN_TYPE_UINT32 id,
                           AB_BANKING_LOGLEVEL level,
                           const char *text);
int AB_Banking_ProgressEnd(AB_BANKING *ab, GWEN_TYPE_UINT32 id);

/*@}*/



/** @name Setters For Virtual User Interaction Functions
 *
 */
/*@{*/

void AB_Banking_SetMessageBoxFn(AB_BANKING *ab,
                                AB_BANKING_MESSAGEBOX_FN f);
void AB_Banking_SetInputBoxFn(AB_BANKING *ab,
                              AB_BANKING_INPUTBOX_FN f);
void AB_Banking_SetShowBoxFn(AB_BANKING *ab,
                             AB_BANKING_SHOWBOX_FN f);
void AB_Banking_SetHideBoxFn(AB_BANKING *ab,
                             AB_BANKING_HIDEBOX_FN f);

void AB_Banking_SetProgressStartFn(AB_BANKING *ab,
                                   AB_BANKING_PROGRESS_START_FN f);
void AB_Banking_SetProgressAdvanceFn(AB_BANKING *ab,
                                     AB_BANKING_PROGRESS_ADVANCE_FN f);
void AB_Banking_SetProgressLogFn(AB_BANKING *ab,
                                 AB_BANKING_PROGRESS_LOG_FN f);
void AB_Banking_SetProgressEndFn(AB_BANKING *ab,
                                 AB_BANKING_PROGRESS_END_FN f);

/*@}*/

/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_BANKING_H */






