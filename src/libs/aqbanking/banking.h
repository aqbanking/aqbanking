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
#include <aqbanking/error.h> /* for AQBANKING_API */

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup AB_BANKING AB_BANKING (Main Interface)
 * @ingroup AB_C_INTERFACE
 *
 * @short This group contains the main API function group.
 * <p>
 * A program should first call @ref AB_Banking_Init to allow AqBanking
 * to load its configuration files and initialize itself.
 * </p>
 * After that you may call any other function of this group (most likely
 * the program will request a list of managed account via
 * @ref AB_Banking_GetAccounts).
 * </p>
 * <p>
 * When the program has finished its work it should call @ref AB_Banking_Fini
 * as the last function of AqBanking (just before calling
 * @ref AB_Banking_free).
 * </p>
 */
/*@{*/

/**
 * Name of the default configuration file within the users home folder.
 */
#define AB_BANKING_CONFIGFILE ".aqbanking.conf"

/**
 * This value is used with @ref AB_Banking_ProgressAdvance to flag that
 * there really was no progress since the last call to that function but
 * that that function should simply check for user interaction (without
 * the need of updating the progress bar).
 */
#define AB_BANKING_PROGRESS_NONE 0xffffffff

/**
 * Object to be operated on by function in this group (@ref AB_BANKING).
 */
 typedef struct AB_BANKING AB_BANKING;

/**
 * This object is prepared to be inherited (using @ref GWEN_INHERIT_SETDATA).
 */
GWEN_INHERIT_FUNCTION_LIB_DEFS(AB_BANKING, AQBANKING_API)
/* Do not terminate these lines with semicolon because they are
   macros, not functions, and ISO C89 does not allow a semicolon
   there. */

/** @name Flags For AB_Banking_InputBox
 *
 * These flags are given to @ref AB_Banking_InputBox to modify its
 * behaviour.
 */
/*@{*/
/** input must be confirmed (e.g. by asking for the same input twice) */
#define AB_BANKING_INPUT_FLAGS_CONFIRM 0x00000001
/** input may be shown (otherwise it should be hidden, e.g. for passwords) */
#define AB_BANKING_INPUT_FLAGS_SHOW    0x00000002
/** numneric input is requested (e.g. for PINs) */
#define AB_BANKING_INPUT_FLAGS_NUMERIC 0x00000004
/*@}*/


/** @name Flags For AB_Banking_MessageBox
 *
 * These flags are given to @ref AB_Banking_MessageBox to modify its
 * behaviour. You may OR-combine the flags.<br>
 * Examples:
 * <ul>
 *  <li>
 *    normal error message, multiple buttons, first button confirms
 *    @code
 *      (AB_BANKING_MSG_FLAGS_TYPE_ERROR |
 *      AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
 *      AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL)
 *    @endcode
 *  </li>
 *  <li>
 *    dangerous error message, multiple buttons, first button confirms
 *    @code
 *      (AB_BANKING_MSG_FLAGS_TYPE_ERROR |
 *      AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
 *      AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS)
 *    @endcode
 *  </li>
 * </ul>
 */
/*@{*/
/**
 * Defines the mask to catch the message type. E.g. a check whether a
 * message is a warning could be performed by:
 * @code
 * if ( ( flags & AB_BANKING_MSG_FLAGS_TYPE_MASK) ==
 *      AB_BANKING_MSG_FLAGS_TYPE_WARN) {
 *      fprintf(stderr, "This is a warning.\n");
 * }
 * @endcode
 */
#define AB_BANKING_MSG_FLAGS_TYPE_MASK           0x07
/** The message is a simple information. */
#define AB_BANKING_MSG_FLAGS_TYPE_INFO         0
/** check whether the given flags represent an info message */
#define AB_BANKING_MSG_FLAGS_TYPE_IS_INFO(fl) \
  ((fl & AB_BANKING_MSG_FLAGS_TYPE_MASK)==AB_BANKING_MSG_FLAGS_TYPE_INFO)

/** The message is a warning */
#define AB_BANKING_MSG_FLAGS_TYPE_WARN         1
/** check whether the given flags represent a warning message */
#define AB_BANKING_MSG_FLAGS_TYPE_IS_WARN(fl)  \
  ((fl & AB_BANKING_MSG_FLAGS_TYPE_MASK)==AB_BANKING_MSG_FLAGS_TYPE_WARN)

/** The message is a error message */
#define AB_BANKING_MSG_FLAGS_TYPE_ERROR        2
/** check whether the given flags represent an error message */
#define AB_BANKING_MSG_FLAGS_TYPE_IS_ERROR     \
  ((fl & AB_BANKING_MSG_FLAGS_TYPE_MASK)==AB_BANKING_MSG_FLAGS_TYPE_ERROR)

/** button 1 is the confirmation button */
#define AB_BANKING_MSG_FLAGS_CONFIRM_B1         (1<<3)
/** button 2 is the confirmation button */
#define AB_BANKING_MSG_FLAGS_CONFIRM_B2         (2<<3)
/** button 3 is the confirmation button */
#define AB_BANKING_MSG_FLAGS_CONFIRM_B3         (3<<3)
/** Determine which button is the confirmation button */
#define AB_BANKING_MSG_FLAGS_CONFIRM_BUTTON(fl) ((flags & 0x3)>>3)


/**
 * <p>
 * Check for the severity of the message. This allows non-interactive
 * backends to react upon the message accordingly.
 * The backend calling this function thus allows the frontend to detect
 * when the message is important regarding data security.
 * E.g. a message like "Shall I delete this file" should be considered
 * dangerous (since this might result in a data loss). However, the messae
 * "Just started" is not that dangerous ;-)
 * </p>
 * <p>
 * The following example allows to determine whether a message is
 * dangerous:
 * @code
 * if ( ( flags & AB_BANKING_MSG_FLAGS_SEVERITY_MASK) ==
 *      AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS) {
 *      fprintf(stderr, "This is a dangerous.\n");
 * }
 * @endcode
 * </p>
 */
#define AB_BANKING_MSG_FLAGS_SEVERITY_MASK       (0x7<<5)
/** Message does not affect security or induce any problem to the system */
#define AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL      (0x0<<5)
#define AB_BANKING_MSG_FLAGS_SEVERITY_IS_NORMAL(fl) \
  ((fl & AB_BANKING_MSG_FLAGS_SEVERITY_MASK)==\
  AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL)
/** Message is considered dangerous and thus should be attended to by a
 * humanoid ;-) */
#define AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS   (0x1<<5)
#define AB_BANKING_MSG_FLAGS_SEVERITY_IS_DANGEROUS  \
  ((fl & AB_BANKING_MSG_FLAGS_SEVERITY_MASK)==\
  AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS)
/*@}*/


/** @name Flags For AB_Banking_ShowBox
 *
 */
/*@{*/

#define AB_BANKING_SHOWBOX_FLAGS_BEEP 0x00000001


/*@}*/

/**
 * This is used with @ref AB_Banking_ProgressLog to tell the function
 * about the severity of the message. The implementation of this function
 * may then decide on this argument about whether to show or repress this
 * message.
 */
typedef enum {
  /** extremely important message (just before abort()) */
  AB_Banking_LogLevelPanic=0,
  /** very important message */
  AB_Banking_LogLevelEmergency,
  /** error message */
  AB_Banking_LogLevelError,
  /** warning */
  AB_Banking_LogLevelWarn,
  /** notice (important information) */
  AB_Banking_LogLevelNotice,
  /** information (not that important) */
  AB_Banking_LogLevelInfo,
  /** simple debug message */
  AB_Banking_LogLevelDebug,
  /** verbous debug message */
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
/**
 * Please see @ref AB_Banking_MessageBox for details.
 */
 typedef int (*AB_BANKING_MESSAGEBOX_FN)(AB_BANKING *ab,
                                        GWEN_TYPE_UINT32 flags,
                                        const char *title,
                                        const char *text,
                                        const char *b1,
                                        const char *b2,
                                        const char *b3);

/**
 * Please see @ref AB_Banking_InputBox for details.
 */
typedef int (*AB_BANKING_INPUTBOX_FN)(AB_BANKING *ab,
                                      GWEN_TYPE_UINT32 flags,
                                      const char *title,
                                      const char *text,
                                      char *buffer,
                                      int minLen,
                                      int maxLen);

/**
 * Please see @ref AB_Banking_ShowBox for details.
 */
typedef GWEN_TYPE_UINT32 (*AB_BANKING_SHOWBOX_FN)(AB_BANKING *ab, 
                                                  GWEN_TYPE_UINT32 flags,
                                                  const char *title,
                                                  const char *text);
/**
 * Please see @ref AB_Banking_HideBox for details.
 */
typedef void (*AB_BANKING_HIDEBOX_FN)(AB_BANKING *ab, GWEN_TYPE_UINT32 id);

/**
 * Please see @ref AB_Banking_ProgressStart for details.
 */
typedef GWEN_TYPE_UINT32
  (*AB_BANKING_PROGRESS_START_FN)(AB_BANKING *ab, 
                                  const char *title,
                                  const char *text,
                                  GWEN_TYPE_UINT32 total);

/**
 * Please see @ref AB_Banking_ProgressAdvance for details.
 */
typedef int (*AB_BANKING_PROGRESS_ADVANCE_FN)(AB_BANKING *ab, 
                                              GWEN_TYPE_UINT32 id,
                                              GWEN_TYPE_UINT32 progress);
/**
 * Please see @ref AB_Banking_ProgressLog for details.
 */
typedef int (*AB_BANKING_PROGRESS_LOG_FN)(AB_BANKING *ab, 
                                          GWEN_TYPE_UINT32 id,
                                          AB_BANKING_LOGLEVEL level,
                                          const char *text);
/**
 * Please see @ref AB_Banking_ProgressEnd for details.
 */
typedef int (*AB_BANKING_PROGRESS_END_FN)(AB_BANKING *ab, 
                                          GWEN_TYPE_UINT32 id);
/*@}*/






/** @name Constructor, Destructor, Init, Fini
 *
 */
/*@{*/
/**
 * <p>
 * Creates an instance of AqBanking. Though AqBanking is quite object
 * oriented (and thus allows multiple instances of AB_BANKING to co-exist)
 * you should avoid having multiple AB_BANKING object in parallel.
 * </p>
 * <p>
 * This is just because the backends are loaded dynamically and might not like
 * to be used with multiple instances of AB_BANKING in parallel.
 * </p>
 * <p>
 * You should later free this object using @ref AB_Banking_free.
 * </p>
 * <p>
 * This function does not actually load the configuration file or setup
 * AqBanking, that ist performed by @ref AB_Banking_Init.
 * </p>
 * @return new instance of AB_BANKING
 * @param appName name of the application which wants to use AqBanking.
 * This allows AqBanking to separate settings and data for multiple
 * applications.
 * @param fname path and name of the configuration file to use. You should
 * in most cases present a NULL for this parameter (which let's AqBanking
 * load the users default configuration file, see @ref AB_BANKING_CONFIGFILE).
 */
AQBANKING_API 
AB_BANKING *AB_Banking_new(const char *appName,
                           const char *fname);

/**
 * Destroys the given instance of AqBanking. Please note that if
 * @ref AB_Banking_Init has been called on this object then
 * @ref  AB_Banking_Fini should be called before this function.
 */
AQBANKING_API 
void AB_Banking_free(AB_BANKING *ab);

/**
 * Initializes AqBanking. This actually reads the configuration file,
 * thus loading account settings and backends as needed.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab banking interface
 */
AQBANKING_API 
int AB_Banking_Init(AB_BANKING *ab);

/**
 * Deinitializes AqBanking that allowing it to save its data and to unload
 * backends.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab banking interface
 */
AQBANKING_API 
int AB_Banking_Fini(AB_BANKING *ab);
/*@}*/


/** @name Working With Backends
 *
 * <p>
 * Working with backends - as far as the frontend is concerned - is very
 * simple.
 * </p>
 * <p>
 * An application typically does this upon initial setup:
 * <ul>
 *  <li>
 *    get a list of available backends (@ref AB_Banking_GetActiveProviders)
 *  </li>
 *  <li>
 *    present this list to the user, let him select which backend(s) to
 *    activate/deactivate
 *  </li>
 *  <li>
 *    activate (@ref AB_Banking_ActivateProvider) or
 *    deactivate (@ref AB_Banking_DeactivateProvider) the backends
 *    accordingly
 *  </li>
 *  <li>
 *    optionally: allow the user to setup a selected backend
 *    (@ref AB_Banking_GetWizard to get the required setup wizard) and
 *    then call @ref AB_ProviderWizard_Setup on that backend)
 *  </li>
 * </ul>
 * </p>
 * <p>
 * Please note that for security reasons a backend is only used when
 * it has explicitly been activated. So if any backend is added to AqBanking
 * by installing it you will still need to activate it to make use of it.
 * </p>
 * <p>
 * However, the activation state is preserved across shutdowns of
 * AqBanking, so a backend remains active until it is explicitly deactivated.
 * </p>
 */
/*@{*/

/**
 * Returns a list of the names of currently active providers.
 */
AQBANKING_API 
const GWEN_STRINGLIST *AB_Banking_GetActiveProviders(const AB_BANKING *ab);

/**
 * This function loads the given backend (if it not already has been) and
 * imports any account that backend might offer. You can use this function
 * to engage a backend which has not yet been used (but it doesn't hurt if you
 * use it on already active backends).
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab banking interface
 * @param backend name of the backend (such as "aqhbci". You can retrieve
 * such a name either from the list of active backends
 * (@ref AB_Banking_GetActiveProviders) or from an plugin description
 * retrieved via @ref AB_Banking_GetProviderDescrs (call
 * @ref GWEN_PluginDescription_GetName on that plugin description).
 */
AQBANKING_API 
int AB_Banking_ImportProviderAccounts(AB_BANKING *ab, const char *backend);

/**
 * <p>
 * Activates a backend. It remains active (even across shutdowns of
 * AqBanking) until it is explicitly deactivated (using
 * @ref AB_Banking_DeactivateProvider).
 * </p>
 * <p>
 * This function automatically imports all accounts presented by the
 * backend.
 * </p>
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab banking interface
 * @param backend name of the backend (such as "aqhbci". You can retrieve
 * such a backend either from the list of active backends
 * (@ref AB_Banking_GetActiveProviders) or from an plugin description
 * retrieved via @ref AB_Banking_GetProviderDescrs (call
 * @ref GWEN_PluginDescription_GetName on that plugin description).
 */
AQBANKING_API 
int AB_Banking_ActivateProvider(AB_BANKING *ab, const char *backend);

/**
 * Deactivates a backend. This is the default state for backends until they
 * have explicitly been activated by @ref AB_Banking_ActivateProvider.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab banking interface
 * @param backend name of the backend (such as "aqhbci". You can retrieve
 * such a name either from the list of active backends
 * (@ref AB_Banking_GetActiveProviders) or from an plugin description
 * retrieved via @ref AB_Banking_GetProviderDescrs (call
 * @ref GWEN_PluginDescription_GetName on that plugin description).
 */
AQBANKING_API 
int AB_Banking_DeactivateProvider(AB_BANKING *ab, const char *backend);

/**
 * Tries to load the wizard for the given backend which is of the given
 * type t.
 * Setup wizards are sorted by backends, since the wizards do things
 * very much dependant on the backend. Ideally they are shipped with the
 * backend.
 * @param ab pointer to the AB_BANKING object
 * @param backend name of the backend (such as "aqhbci". You can retrieve
 * such a name either from the list of active backends
 * (@ref AB_Banking_GetActiveProviders) or from an plugin description
 * retrieved via @ref AB_Banking_GetProviderDescrs (call
 * @ref GWEN_PluginDescription_GetName on that plugin description).
 * @param t wizard type. To allow keeping the API as open as possible you
 * may give a type name here. However, the following names are expected:
 * <ul>
 *  <li><b>kde_wizard</b> for a wizard running under KDE</li>
 *  <li><b>gnome_wizard</b> for a wizard running under GNOME</li>
 *  <li><b>console_wizard</b> for a wizard running in a console</li>
 *  <li><b>curses_wizard</b> for a wizard using (n)curses</li>
 * </ul>
 * When actually loading a wizard then this function searches inside the
 * plugin for a function called WIZARDNAME_factory() (WIZARDNAME is the
 * name given for argument <i>t</i>). This function is expected to return
 * a pointer to a @ref AB_PROVIDER_WIZARD.
 */
AQBANKING_API 
AB_PROVIDER_WIZARD *AB_Banking_GetWizard(AB_BANKING *ab,
                                         const char *backend,
                                         const char *t);
/*@}*/




/** @name Application Data
 *
 * Applications may let AqBanking store global application specific data.
 * In addition, account specific data can also be stored using
 * @ref AB_Account_GetAppData.
 */
/*@{*/
/**
 * Returns the application name as given to @ref AB_Banking_new.
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API 
const char *AB_Banking_GetAppName(const AB_BANKING *ab);

/**
 * Returns a GWEN_DB_NODE which can be used to store/retrieve data for
 * the currently running application. The group returned MUST NOT be
 * freed !
 * AqBanking is able to separate and store the data for every application.
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API 
GWEN_DB_NODE *AB_Banking_GetAppData(AB_BANKING *ab);

/**
 * Returns the name of the user folder for AqBanking's data.
 * Normally this is something like "/home/me/.banking".
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param buf GWEN_BUFFER to append the path name to
 */
AQBANKING_API 
int AB_Banking_GetUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf);

/**
 * Returns the name of the user folder for application data.
 * Normally this is something like "/home/me/.banking/apps".
 * Your application may choose to create folders below this one to store
 * user data. If you only add AqBanking to an existing program to add
 * home banking support you will most likely use your own folders and thus
 * won't need this function.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param buf GWEN_BUFFER to append the path name to
 */
AQBANKING_API 
int AB_Banking_GetAppUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf);


/*@}*/



/** @name Working With Accounts
 *
 * AqBanking controls a list of accounts. You can ask it for the full list
 * (@ref AB_Banking_GetAccounts) or directly request a specific account
 * (@ref AB_Banking_GetAccount).
 */
/*@{*/
/**
 * Returns a list of currently known accounts. The returned list is
 * owned by the caller, so he is responsible for freeing it (using
 * @ref AB_Account_List2_free).
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API 
AB_ACCOUNT_LIST2 *AB_Banking_GetAccounts(const AB_BANKING *ab);

/**
 * This function does an account lookup based on the given unique id.
 * This id is assigned by AqBanking when an account is created.
 * @param ab pointer to the AB_BANKING object
 * @param uniqueId unique id of the account assigned by AqBanking
 */
AQBANKING_API 
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
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API 
GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetProviderDescrs(AB_BANKING *ab);


/**
 * Returns a list2 of wizard descriptions for the given backend.
 * You must free this list after using it via
 * @ref GWEN_PluginDescription_List2_freeAll.
 * Please note that a simple @ref GWEN_PluginDescription_List2_free would
 * not suffice, since that would only free the list but not the objects
 * stored within the list !
 * @param ab pointer to the AB_BANKING object
 * @param pn name of the backend (such as "aqhbci". You can retrieve
 * such a name either from the list of active backends
 * (@ref AB_Banking_GetActiveProviders) or from an plugin description
 * retrieved via @ref AB_Banking_GetProviderDescrs (call
 * @ref GWEN_PluginDescription_GetName on that plugin description).
 */
AQBANKING_API 
GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetWizardDescrs(AB_BANKING *ab,
                                                          const char *pn);

/*@}*/




/** @name Functions Used by Backends
 *
 */
/*@{*/
/**
 * Store backend specific data with AqBanking. This data is not specific
 * to an application, it will rather be used with every application (since
 * it doesn't depend on the application but on the backend).
 * @param ab pointer to the AB_BANKING object
 * @param pro pointer to the backend for which the data is to be returned
 */
AQBANKING_API 
GWEN_DB_NODE *AB_Banking_GetProviderData(AB_BANKING *ab,
                                         const AB_PROVIDER *pro);

/**
 * This copies the name of the folder for AqBanking's backend data into
 * the given GWEN_Buffer.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param buf buffer to append the path name to
 */
AQBANKING_API 
int AB_Banking_GetProviderUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf);

/*@}*/




/** @name Enqueueing, Dequeueing and Executing Jobs
 *
 * <p>
 * Enqueued jobs are preserved across shutdowns. As soon as a job has been
 * sent to the appropriate backend it will be removed from the queue.
 * Only those jobs are saved/reloaded which have been enqueued but never
 * presented to the backend.
 * </p>
 * <p>
 * This means after calling @ref AB_Banking_ExecuteQueue only those jobs are
 * still in the queue which have not been processed (e.g. because they
 * belonged to a second backend but the user aborted while the jobs for
 * a first backend were in process).
 * </p>
 * <p>
 * Even with this being a quite rare case the application should be aware of
 * the possibilities so it is strongly advised to check the qeueue for unsent
 * jobs after calling @ref AB_Banking_ExecuteQueue and retry if appropriate.
 * </p>
 */
/*@{*/
/**
 * Enqueues a job. This function does not take over the ownership of the
 * job. However, this function makes sure that the job will not be deleted
 * as long as it is in the queue (by calling @ref AB_Job_Attach).
 * So it is safe for you to call @ref AB_Job_free on an enqueued job directly
 * after enqueuing it (but it doesn't make much sense since you would not be able to
 * check for a result).
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
 * This function sends all jobs in the queue to their corresponding backends
 * and allows that backend to process it.
 * If the user did not abort or there was no fatal error the queue is
 * empty upon return. You can verify this by calling
 * @ref AB_Banking_GetEnqueuedJobs.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API 
int AB_Banking_ExecuteQueue(AB_BANKING *ab);

/**
 * Returns the list of currently enqueued jobs. If the queue is empty
 * NULL is returned.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API 
AB_JOB_LIST2 *AB_Banking_GetEnqueuedJobs(const AB_BANKING *ab);
/*@}*/




/** @name Virtual User Interaction Functions
 *
 * All text passed to the frontend via one of the following functions
 * is expected to be an ISO-8859-15 string which may contain newlines but no
 * other control characters (especially no HTML tags since not all frontends
 * are supposed to be able to decode it).
 */
/*@{*/
/**
 * <p>
 * Show a message box with optional buttons.
 * The message box may either contain 1, 2 or three buttons.
 * If only one button is wanted then b1 should hold a pointer to the button
 * text (b2 and b3 must be NULL)
 * In two-button mode b1 and b2 must be valid (b3 must be NULL)
 * In three-button-mode b1, b2 and b3 must be valid pointers.
 * The return value tells which button the user pressed:
 * <ul>
 *  <li>1: button 1</li>
 *  <li>2: button 2</li>
 *  <li>3: button 3</li>
 * </ul>
 * If the frontend can not determine which button has been pressed (e.g. if
 * no button was pressed but the user rather aborted the dialog by simply
 * closing the box) it should return @b 0.
 * </p>
 * <p>
 *  This function is blocking.
 * </p>
 * @return the number of the button pressed (1=b1, 2=b2, 3=b3), any other
 *  value should be considered an error, including 0)
 * @param ab banking interface
 * @param flags flags, see @ref AB_BANKING_MSG_FLAGS_TYPE_MASK ff.
 * @param title title of the message box
 * @param text (see text restrictions note above)
 * @param b1 text for the first button (required), should be something
 *  like "Ok" (see text restrictions note above)
 * @param b2 text for the optional second button
 * @param b3 text for the optional third button
 */
AQBANKING_API 
int AB_Banking_MessageBox(AB_BANKING *ab,
                          GWEN_TYPE_UINT32 flags,
                          const char *title,
                          const char *text,
                          const char *b1,
                          const char *b2,
                          const char *b3);

/**
 * <p>
 * Ask the user for input.
 * </p>
 * <p>
 *  This function is blocking.
 * </p>
 * @param ab banking interface
 * @param flags flags, see @ref AB_BANKING_INPUT_FLAGS_CONFIRM ff.
 * @param title title of the input box
 * @param text (see text restrictions note above)
 * @param buffer buffer to store the response in. Must have at least room
 *  @b maxLen bytes
 * @param minLen minimal length of input (if 0 then there is no low limit)
 * @param maxLen size of the buffer including the trailing NULL character.
 * This means that if you want to ask the user for a PIN of at most 4
 * characters you need to supply a buffer of at least @b 5 bytes and provide
 * a 5 as maxLen.
 */
AQBANKING_API 
int AB_Banking_InputBox(AB_BANKING *ab,
                        GWEN_TYPE_UINT32 flags,
                        const char *title,
                        const char *text,
                        char *buffer,
                        int minLen,
                        int maxLen);

/**
 * <p>
 * Shows a box with the given text. This function should return immediately,
 * it should especially NOT wait for user input. This is used to show very
 * important notices the user should see but which don't need user
 * interaction. The message box will be removed later via
 * @ref AB_Banking_HideBox. It is ok to allow the user to prematurely
 * close the box.
 * </p>
 * <p>
 * It is required that for every call to this function to be followed later
 * by a corresponding call to @ref AB_Banking_HideBox.
 * </p>
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @return returns an id to be presented to @ref AB_Banking_HideBox.
 * @param ab banking interface
 * @param flags flags, see @ref AB_BANKING_SHOWBOX_FLAGS_BEEP ff
 * @param title title of the box
 * @param text (see text restrictions note above)
 */
AQBANKING_API 
GWEN_TYPE_UINT32 AB_Banking_ShowBox(AB_BANKING *ab,
                                    GWEN_TYPE_UINT32 flags,
                                    const char *title,
                                    const char *text);

/**
 * Hides a message box previously shown by a call to @ref AB_Banking_ShowBox.
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @param ab banking interface
 * @param id id returned by @ref AB_Banking_ShowBox. If @b 0 then the last
 * message shown is referred to.
 */
AQBANKING_API 
void AB_Banking_HideBox(AB_BANKING *ab, GWEN_TYPE_UINT32 id);


/**
 * <p>
 * This function is called when a long term operation is started.
 * Theoretically nesting is allowed, however, you should refrain from
 * opening multiple progress dialogs to avoid confusion of the user.
 * This function must return immediately (i.e. it MUST NOT wait for
 * user interaction).
 * </p>
 * <p>
 * On graphical user interfaces such a dialog should contain a widget
 * for the text presented here, a progress bar, a text widget to
 * collect the log messages received via @ref AB_Banking_ProgressLog and
 * button to allow the user to abort the current operation monitored by
 * this dialog window.
 * </p>
 * <p>
 * Between a call to this function and one to @ref AB_Banking_ProgressEnd
 * the user should not be allowed to close the dialog window.
 * </p>
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @return id to be used with the other AB_Banking_Progress functions.
 * @param ab banking interface
 * @param title title of the dialog
 * @param text (see text restrictions note above)
 * @param total total number of steps of the operation started (i.e. value
 *  which represents 100%)
 */
AQBANKING_API 
GWEN_TYPE_UINT32 AB_Banking_ProgressStart(AB_BANKING *ab,
                                          const char *title,
                                          const char *text,
                                          GWEN_TYPE_UINT32 total);

/**
 * <p>
 * Advances the progress bar an application might present to the user and
 * checks whether the user wants to abort the operation currently in progress.
 * </p>
 * <p>
 * On graphical user interfaces this function should also check for user
 * interaction and/or update the GUI (e.g. by calling qApp->processEvents()
 * when using QT/KDE).
 * </p>
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @return 0 if ok, !=0 if the current operation is to be aborted
 * @param ab banking interface
 * @param id id assigned by @ref AB_Banking_ProgressStart (if 0 then the
 * last started progress dialog is referred to)
 * @param progress new value for progress. A special value is
 *  AB_BANKING_PROGRESS_NONE which means that the progress is unchanged.
 * This might be used as a keepalive call to a GUI.
 */
AQBANKING_API 
int AB_Banking_ProgressAdvance(AB_BANKING *ab,
                               GWEN_TYPE_UINT32 id,
                               GWEN_TYPE_UINT32 progress);

/**
 * Adds a log message to the referred process dialog.
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @param ab banking interface
 * @param id id assigned by @ref AB_Banking_ProgressStart (if 0 then the
 * last started progress dialog is referred to)
 * @param level log level (see @ref AB_Banking_LogLevelPanic ff.)
 * @param text log text (see text restrictions note above)
 */
AQBANKING_API 
int AB_Banking_ProgressLog(AB_BANKING *ab,
                           GWEN_TYPE_UINT32 id,
                           AB_BANKING_LOGLEVEL level,
                           const char *text);

/**
 * <p>
 * Flags the end of the current operation. In graphical user interfaces
 * this call should allow the user to close the progress dialog window.
 * </p>
 * <p>
 * On graphical user interfaces a call to this function should disable the
 * <i>abort</i> button. It would be best not to close the dialog on
 * receiption of this call but to simple enable a dialog closing (otherwise
 * the user will not be able to see the log messages).
 * </p>
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @param ab banking interface
 * @param id id assigned by @ref AB_Banking_ProgressStart (if 0 then the
 * last started progress dialog is referred to)
 */
AQBANKING_API 
int AB_Banking_ProgressEnd(AB_BANKING *ab, GWEN_TYPE_UINT32 id);

/*@}*/



/** @name Setters For Virtual User Interaction Functions
 *
 * The functions in this group set the corresponding callback function
 * pointers.
 */
/*@{*/

AQBANKING_API 
void AB_Banking_SetMessageBoxFn(AB_BANKING *ab,
                                AB_BANKING_MESSAGEBOX_FN f);
AQBANKING_API 
void AB_Banking_SetInputBoxFn(AB_BANKING *ab,
                              AB_BANKING_INPUTBOX_FN f);
AQBANKING_API 
void AB_Banking_SetShowBoxFn(AB_BANKING *ab,
                             AB_BANKING_SHOWBOX_FN f);
AQBANKING_API 
void AB_Banking_SetHideBoxFn(AB_BANKING *ab,
                             AB_BANKING_HIDEBOX_FN f);

AQBANKING_API 
void AB_Banking_SetProgressStartFn(AB_BANKING *ab,
                                   AB_BANKING_PROGRESS_START_FN f);
AQBANKING_API 
void AB_Banking_SetProgressAdvanceFn(AB_BANKING *ab,
                                     AB_BANKING_PROGRESS_ADVANCE_FN f);
AQBANKING_API 
void AB_Banking_SetProgressLogFn(AB_BANKING *ab,
                                 AB_BANKING_PROGRESS_LOG_FN f);
AQBANKING_API 
void AB_Banking_SetProgressEndFn(AB_BANKING *ab,
                                 AB_BANKING_PROGRESS_END_FN f);

/*@}*/

/*@}*/ /* defgroup */


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_BANKING_H */






