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

/** @file 
 * @short The main interface of the aqbanking library
 */

#ifndef AQBANKING_BANKING_H
#define AQBANKING_BANKING_H

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/plugindescr.h>
#include <aqbanking/error.h> /* for AQBANKING_API */
#include <aqbanking/version.h>


#define AB_PM_LIBNAME    "aqbanking"
#define AB_PM_SYSCONFDIR "sysconfdir"
#define AB_PM_DATADIR    "datadir"


#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup G_AB_BANKING AB_BANKING (Main Interface)
 * @ingroup G_AB_C_INTERFACE
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

/** @name Extensions supported by the application
 *
 */
/*@{*/
#define AB_BANKING_EXTENSION_NONE             0x00000000
/**
 * If this flag is set then the application allows nesting progress
 * dialogs. If the application does it might be best to use the toplevel
 * progress widget and just add a progress bar to it.
 * If this flag is not set then AqBanking keeps track of the nesting level
 * and dismisses further calls to @ref AB_Banking_ProgressStart until
 * @ref AB_Banking_ProgressEnd has been called often enough to reset the
 * nesting counter. I.e. the application will not receive nesting calls
 * to @ref AB_Banking_ProgressStart.
 */
#define AB_BANKING_EXTENSION_NESTING_PROGRESS 0x00000001
/*@}*/


/**
 * This value is used with @ref AB_Banking_ProgressAdvance to flag that
 * there really was no progress since the last call to that function but
 * that that function should simply check for user interaction (without
 * the need of updating the progress bar).
 */
#define AB_BANKING_PROGRESS_NONE 0xffffffff

/**
 * This value is used when the total number of steps is not known to the
 * caller and he just wants to advance the progress by one (e.g. backends
 * use this value when a job has been finished, but the backends do not know
 * how many jobs there still are in AqBanking's queue).
 */
#define AB_BANKING_PROGRESS_ONE  0xfffffffe


/**
 * Object to be operated on by functions in this group (@ref AB_BANKING).
 */
 typedef struct AB_BANKING AB_BANKING;

/**
 * This object is prepared to be inherited (using @ref GWEN_INHERIT_SETDATA).
 */
GWEN_INHERIT_FUNCTION_LIB_DEFS(AB_BANKING, AQBANKING_API)



/** @defgroup G_AB_BANKING_JOB_API AB_BANKING Job-API
 *
 * @short This group contains the job API ("main interface") function group.
 *
 * <p>
 * This is the preferred API for new programs. New features will first be
 * implemented here.
 * </p>
 */
/*@{*/


/** @name Flags For AB_Banking_InputBox
 *
 * These flags are given to @ref AB_Banking_InputBox to modify its
 * behaviour.
 */
/*@{*/
/** input must be confirmed (e.g. by asking for the same input twice) */
#define AB_BANKING_INPUT_FLAGS_CONFIRM        0x00000001
/** input may be shown (otherwise it should be hidden, e.g. for passwords) */
#define AB_BANKING_INPUT_FLAGS_SHOW           0x00000002
/** numeric input is requested (e.g. for PINs) */
#define AB_BANKING_INPUT_FLAGS_NUMERIC        0x00000004
/** if set then this is a retry (esp. when getting a PIN) */
#define AB_BANKING_INPUT_FLAGS_RETRY          0x00000008
/** allow a default value to be used instead of an entered one.
 * A graphical UI should add a "default" button to the dialog. */
#define AB_BANKING_INPUT_FLAGS_ALLOW_DEFAULT  0x00000010
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
 * <p>
 * A note about <i>confirmation buttons</i>: AqBanking has been designed with
 * non-interactive applications in mind. For such an application it is
 * important to know what button-press it has to simulate upon catching of a
 * messagebox callback. This is what the confimation button flags are for.
 * For informative messages the application may simply return the number of
 * the confirmation button and be done.
 * </p>
 * <p>
 * However, non-interactive applications should return an error (value 0)
 * for messages classified as <b>dangerous</b>
 * (see @ref AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS) to avoid data loss.
 * </p>
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
#define AB_BANKING_MSG_FLAGS_CONFIRM_BUTTON(fl) ((fl & 0x3)>>3)


/**
 * <p>
 * Check for the severity of the message. This allows non-interactive
 * backends to react on the message accordingly.
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
 *      fprintf(stderr, "This is dangerous.\n");
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
#define AB_BANKING_MSG_FLAGS_SEVERITY_IS_DANGEROUS(fl)  \
  ((fl & AB_BANKING_MSG_FLAGS_SEVERITY_MASK)==\
  AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS)
/*@}*/


/** @name Flags For AB_Banking_ShowBox
 *
 */
/*@{*/

/**
 * Make the frontend beep. This should rarely be used, and only in situations
 * where it is very important to get the users attention (e.g. when asking
 * for a PIN on a card reader).
 */
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
#include <aqbanking/imexporter.h>
#include <aqbanking/bankinfoplugin.h>
#include <aqbanking/bankinfo.h>
#include <aqbanking/country.h>
#include <aqbanking/user.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @name Prototypes For Virtual User Interaction Functions
 *
 */
/*@{*/
/**
 * Please see @ref AB_Banking_MessageBox for details.
 *
 * One way of passing arbitrary additional data to this callback is by
 * means of the @ref AB_Banking_GetUserData function.
 * However, the recommended way is to use Gwenhywfars' heritage functions
 * (see @ref GWEN_INHERIT_SETDATA).
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
 *
 * One way of passing arbitrary additional data to this callback is by
 * means of the @ref AB_Banking_GetUserData function.
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
 *
 * One way of passing arbitrary additional data to this callback is by
 * means of the @ref AB_Banking_GetUserData function.
 */
typedef GWEN_TYPE_UINT32 (*AB_BANKING_SHOWBOX_FN)(AB_BANKING *ab, 
                                                  GWEN_TYPE_UINT32 flags,
                                                  const char *title,
                                                  const char *text);
/**
 * Please see @ref AB_Banking_HideBox for details.
 *
 * One way of passing arbitrary additional data to this callback is by
 * means of the @ref AB_Banking_GetUserData function.
 */
typedef void (*AB_BANKING_HIDEBOX_FN)(AB_BANKING *ab, GWEN_TYPE_UINT32 id);

/**
 * Please see @ref AB_Banking_ProgressStart for details.
 *
 * One way of passing arbitrary additional data to this callback is by
 * means of the @ref AB_Banking_GetUserData function.
 */
typedef GWEN_TYPE_UINT32
  (*AB_BANKING_PROGRESS_START_FN)(AB_BANKING *ab, 
                                  const char *title,
                                  const char *text,
                                  GWEN_TYPE_UINT32 total);

/**
 * Please see @ref AB_Banking_ProgressAdvance for details.
 *
 * One way of passing arbitrary additional data to this callback is by
 * means of the @ref AB_Banking_GetUserData function.
 */
typedef int (*AB_BANKING_PROGRESS_ADVANCE_FN)(AB_BANKING *ab, 
                                              GWEN_TYPE_UINT32 id,
                                              GWEN_TYPE_UINT32 progress);
/**
 * Please see @ref AB_Banking_ProgressLog for details.
 *
 * One way of passing arbitrary additional data to this callback is by
 * means of the @ref AB_Banking_GetUserData function.
 */
typedef int (*AB_BANKING_PROGRESS_LOG_FN)(AB_BANKING *ab, 
                                          GWEN_TYPE_UINT32 id,
                                          AB_BANKING_LOGLEVEL level,
                                          const char *text);
/**
 * Please see @ref AB_Banking_ProgressEnd for details.
 *
 * One way of passing arbitrary additional data to this callback is by
 * means of the @ref AB_Banking_GetUserData function.
 */
typedef int (*AB_BANKING_PROGRESS_END_FN)(AB_BANKING *ab, 
                                          GWEN_TYPE_UINT32 id);

/**
 * This function is used to make the application print something.
 * The same restrictions noted above apply to the text parameter (utf-8,
 * maybe containing HTML).
 * Please see @ref AB_Banking_Print for details.
 */
typedef int (*AB_BANKING_PRINT_FN)(AB_BANKING *ab,
                                   const char *docTitle,
                                   const char *docType,
                                   const char *descr,
                                   const char *text);

/*@}*/



/** @name Prototypes For Virtual Security Functions
 *
 */
/*@{*/
typedef enum {
  AB_Banking_PinStatusBad=-1,
  AB_Banking_PinStatusUnknown,
  AB_Banking_PinStatusOk
} AB_BANKING_PINSTATUS;


typedef enum {
  AB_Banking_TanStatusBad=-1,
  AB_Banking_TanStatusUnknown,
  AB_Banking_TanStatusUsed,
  AB_Banking_TanStatusUnused,
} AB_BANKING_TANSTATUS;


typedef int (*AB_BANKING_GETPIN_FN)(AB_BANKING *ab,
                                    GWEN_TYPE_UINT32 flags,
                                    const char *token,
                                    const char *title,
                                    const char *text,
                                    char *buffer,
                                    int minLen,
                                    int maxLen);
typedef int (*AB_BANKING_SETPINSTATUS_FN)(AB_BANKING *ab,
                                          const char *token,
                                          const char *pin,
                                          AB_BANKING_PINSTATUS status);

typedef int (*AB_BANKING_GETTAN_FN)(AB_BANKING *ab,
                                    const char *token,
                                    const char *title,
                                    const char *text,
                                    char *buffer,
                                    int minLen,
                                    int maxLen);

typedef int (*AB_BANKING_SETTANSTATUS_FN)(AB_BANKING *ab,
                                          const char *token,
                                          const char *tan,
                                          AB_BANKING_TANSTATUS status);


/*@}*/



/** @name Constructor, Destructor, Init, Fini
 *
 */
/*@{*/

/**
 * <p>
 * Creates an instance of AqBanking. Though AqBanking is quite object
 * oriented (and thus allows multiple instances of AB_BANKING to co-exist)
 * you should avoid having multiple AB_BANKING objects in parallel.
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
 * AqBanking, that is performed by @ref AB_Banking_Init.
 * </p>
 * <p>
 * Please note that this function sets a new handler for incoming SSL
 * certificates using GWEN_NetTransportSSL_SetAskAddCertFn2(), so if you
 * want to handle this in your application yourself you must overwrite this
 * handler @b after calling this function here.
 * </p>
 * <p>
 * Please note: This function internally calls @ref AB_Banking_newExtended
 * with the value 0 for <i>extensions</i>. So if your program supports any
 * extension you should call @ref AB_Banking_newExtended instead of this
 * function.
 * </p>
 * @return new instance of AB_BANKING
 *
 * @param appName name of the application which wants to use AqBanking.
 * This allows AqBanking to separate settings and data for multiple
 * applications.
 *
 * @param dname Path for the directory containing the user data of
 * AqBanking. You should in most cases present a NULL for this
 * parameter, which means AqBanking will choose the default user
 * data folder which is "$HOME/.banking".  The configuration file
 * "settings.conf" file is searched for in this folder. NOTE:
 * Versions of AqBanking before 1.2.0.16 used this argument to
 * specify the path and name (!) of the configuration file,
 * whereas now this specifies only the path. It is now impossible
 * to specify the name; aqbanking will always use its default name
 * "settings.conf". For AqBanking < 1.2.0.16, the default
 * configuration file was "$HOME/.aqbanking.conf". This file is
 * now also searched for, but if it exists it will be moved to the
 * new default path and name upon AB_Banking_Fini. The new path
 * will be "$HOME/.banking/settings.conf".
 */
AQBANKING_API
AB_BANKING *AB_Banking_new(const char *appName, const char *dname);

/**
 * This does the same as @ref AB_Banking_new. In addition, the application
 * may state here the extensions it supports.
 * See @ref AB_BANKING_EXTENSION_NONE and following.
 * This is used to keep the number of callbacks to the application small
 * (otherswise whenever we add a flag which changes the expected behaviour
 * of a GUI callback we would have to introduce a new callback in order to
 * maintain binary compatibility).
 */
AQBANKING_API
AB_BANKING *AB_Banking_newExtended(const char *appName,
                                   const char *dname,
                                   GWEN_TYPE_UINT32 extensions);


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
 * Deinitializes AqBanking thus allowing it to save its data and to unload
 * backends.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab banking interface
 */
AQBANKING_API 
int AB_Banking_Fini(AB_BANKING *ab);

/**
 * Saves all data. You may call this function periodically (especially
 * after doing setup stuff).
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab banking interface
 */
int AB_Banking_Save(AB_BANKING *ab);

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
 *    (@ref AB_Banking_FindWizard to get the required setup wizard) and
 *    then run that wizard)
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
 * <p>
 * This function checks whether the given backend is currently active.
 * It returns 0 if the backend is inactive.
 *
 */
AQBANKING_API 
int AB_Banking_IsProviderActive(AB_BANKING *ab, const char *backend);


/**
 * This function simpifies wizard handling. It searches for a wizard for
 * the given frontends.
 * @param ab pointer to the AB_BANKING object
 * @param backend no longer used
 * @param frontends This is a semicolon separated list of acceptable frontends
 * The following lists merely are suggestions:
 * <table>
 *  <tr>
 *    <td>KDE Applications</td>
 *    <td>kde;qt;gtk;gnome</td>
 *  </tr>
 *  <tr>
 *    <td>QT Applications</td>
 *    <td>qt;kde;gtk;gnome</td>
 *  </tr>
 *  <tr>
 *    <td>GNOME Applications</td>
 *    <td>gnome;gtk;qt;kde</td>
 *  </tr>
 *  <tr>
 *    <td>GTK Applications</td>
 *    <td>gtk;gnome;qt;kde</td>
 *  </tr>
 * </table>
 * You can always add an asterisk ("*") to the list to accept any other
 * frontend (or pass a NULL pointer to accept the first valid frontend).
 */
AQBANKING_API
int AB_Banking_FindWizard(AB_BANKING *ab,
                          const char *backend,
                          const char *frontends,
                          GWEN_BUFFER *pbuf);

/**
 * This function simpifies debugger handling. It searches for a debugger for
 * the given backend and the given frontends.
 * @param ab pointer to the AB_BANKING object
 * @param backend name of the backend (such as "aqhbci". You can retrieve
 * such a name either from the list of active backends
 * (@ref AB_Banking_GetActiveProviders) or from an plugin description
 * retrieved via @ref AB_Banking_GetProviderDescrs (call
 * @ref GWEN_PluginDescription_GetName on that plugin description).
 * @param frontends This is a semicolon separated list of acceptable frontends
 * The following lists merely are suggestions:
 * <table>
 *  <tr>
 *    <td>KDE Applications</td>
 *    <td>kde;qt;gtk;gnome</td>
 *  </tr>
 *  <tr>
 *    <td>QT Applications</td>
 *    <td>qt;kde;gtk;gnome</td>
 *  </tr>
 *  <tr>
 *    <td>GNOME Applications</td>
 *    <td>gnome;gtk;qt;kde</td>
 *  </tr>
 *  <tr>
 *    <td>GTK Applications</td>
 *    <td>gtk;gnome;qt;kde</td>
 *  </tr>
 * </table>
 * You can always add an asterisk ("*") to the list to accept any other
 * frontend (or pass a NULL pointer to accept the first valid frontend).
 */
AQBANKING_API
int AB_Banking_FindDebugger(AB_BANKING *ab,
			    const char *backend,
			    const char *frontends,
                            GWEN_BUFFER *pbuf);


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
 * Returns the escaped version of the application name. This name can
 * safely be used to create file paths since all special characters (like
 * '/', '.' etc) are escaped.
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API 
const char *AB_Banking_GetEscapedAppName(const AB_BANKING *ab);

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
 * Returns a GWEN_DB_NODE which can be used to store/retrieve data shared
 * across multiple applications.
 * @param ab pointer to the AB_BANKING object
 * @param name name of the share. Special names are those of frontends and
 * backends, so these names MUST NOT be used for applications (but they may be
 * used by the corresponding frontends, e.g. the QT frontend QBanking used
 * "qbanking" to store some frontend specific settings).
 */
AQBANKING_API 
GWEN_DB_NODE *AB_Banking_GetSharedData(AB_BANKING *ab, const char *name);

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

/**
 * Returns the path to a folder to which shared data can be stored.
 * This might be used by multiple applications if they wish to share some
 * of their data, e.g. QBankManager and AqMoney3 share their transaction
 * storage so that both may work with it.
 * Please note that this folder does not necessarily exist, but you are free
 * to create it.
 */
AQBANKING_API 
int AB_Banking_GetSharedDataDir(const AB_BANKING *ab,
                                const char *name,
                                GWEN_BUFFER *buf);

/** Returns the void pointer that was stored by
 * AB_Banking_SetUserData(). This might be useful for passing data to
 * the callback functions.
 *
 * On the other hand, we strongly encourage using the GWEN_INHERIT
 * macros to store non-trivial data structures in this object. 
 *
 * @param ab Pointer to the AB_BANKING object
 */
AQBANKING_API
void *AB_Banking_GetUserData(AB_BANKING *ab);

/** Save the void pointer that can be retrieved by
 * AB_Banking_GetUserData(). This might be useful for passing data to
 * the callback functions.
 *
 * On the other hand, we strongly encourage using the GWEN_INHERIT
 * macros to store non-trivial data structures in this object. 
 *
 * @param ab Pointer to the AB_BANKING object
 * @param user_data Arbitrary pointer to be stored in the AB_BANKING
 */
AQBANKING_API
void AB_Banking_SetUserData(AB_BANKING *ab, void *user_data);

/*@}*/

/** @name User Management Functions
 *
 * AqBanking controls a list of users. You can ask it for the full list
 * (@ref AB_Banking_GetUserss) or directly request a specific account
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
 * list (e.g. by calling @ref AB_Account_List2_freeAll).
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
 * Returns a list2 of wizard descriptions.
 * You must free this list after using it via
 * @ref GWEN_PluginDescription_List2_freeAll.
 * Please note that a simple @ref GWEN_PluginDescription_List2_free would
 * not suffice, since that would only free the list but not the objects
 * stored within the list !
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API 
GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetWizardDescrs(AB_BANKING *ab);


/**
 * Returns a list2 of debugger descriptions for the given backend.
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
GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetDebuggerDescrs(AB_BANKING *ab,
                                                            const char *pn);

/**
 * Returns a list2 of available importers and exporters.
 * You must free this list after using it via
 * @ref GWEN_PluginDescription_List2_freeAll.
 * Please note that a simple @ref GWEN_PluginDescription_List2_free would
 * not suffice, since that would only free the list but not the objects
 * stored within the list !
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API
GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetImExporterDescrs(AB_BANKING *ab);

/**
 * Loads an importer/exporter backend with the given name. You can use
 * @ref AB_Banking_GetImExporterDescrs to retrieve a list of available
 * backends.
 * AqBanking remains the owner of the object returned (if any), so you
 * <b>must not</b> free it.
 */
AQBANKING_API 
AB_IMEXPORTER *AB_Banking_GetImExporter(AB_BANKING *ab, const char *name);

/**
 * <p>
 * Loads all available profiles for the given importer/exporter.
 * This includes global profiles as well as local ones.
 * </p>
 * <p>
 * Local profiles overwrite global ones, allowing the user to customize the
 * profiles.
 * </p>
 * <p>
 * The GWEN_DB returned contains one group for every loaded profile. Every
 * group has the name of the profile it contains. Every group contains at
 * least a variable called <i>name</i> which contains the name of the
 * profile, too. The remaining content of each group is completely defined by
 * the importer/exporter.
 * </p>
 * <p>
 * You can use @ref GWEN_DB_GetFirstGroup and @ref GWEN_DB_GetNextGroup
 * to browse the profiles.
 * </p>
 * <p>
 * The caller becomes the new owner of the object returned (if any).
 * This makes him/her responsible for freeing it via
 *  @ref GWEN_DB_Group_free.
 * </p>
 * <p>
 * You can use any of the subgroups below the returned one as argument
 * to @ref AB_ImExporter_Import.
 * </p>
 * @param ab pointer to the AB_BANKING object
 * @param name name of the importer whose profiles are to be read
 */
AQBANKING_API
GWEN_DB_NODE *AB_Banking_GetImExporterProfiles(AB_BANKING *ab,
                                               const char *name);
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
AQBANKING_API 
int AB_Banking_ExecuteQueue(AB_BANKING *ab);

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
AQBANKING_API 
int AB_Banking_ExecuteJobList(AB_BANKING *ab, AB_JOB_LIST2 *jl2);

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



/** @name Virtual User Interaction Functions
 *
 * <p>
 * All text passed to the frontend via one of the following functions
 * is expected to be an UTF-8 string which may contain newlines but no other
 * control characters.
 * Text delivered as argument called <i>text</i> throughout the documentation
 * in this group may contain HTML tags.
 * If it does a non-HTML version must be supplied, too.
 * The text MUST begin with the non-HTML version, so that a frontend not
 * capable of parsing HTML can simply exclude the HTML part by cutting
 * before "<html".
 *
 * </p>
 * <p>
 * This is an example for HTML and non-HTML text:
 * </p>
 * @code
 * const char *text;
 *
 * text="This is the non-HTML text"
 *      "<html>"
 *      "And this is the <b>HTML</b> version."
 *      "</html>"
 * @endcode
 * <p>
 * Frontends capable of parsing HTML (such as the KDE frontend) will
 * extract the HTML information and show only that part of the string.
 * </p>
 * <p>
 * Other frontends have to extract the non-HTML information and show only
 * that.
 * </p>
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
 * @param text Text of the box: UTF-8, with both a normal text and a HTML variant of the text in the same string. See text restrictions note above.
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
 * @param text Text of the box: UTF-8, with both a normal text and a HTML variant of the text in the same string. See text restrictions note above.
 * @param buffer buffer to store the response in. Must have at least room for
 *  @b maxLen bytes
 * @param minLen minimal length of input (if 0 then there is no low limit)
 * @param maxLen size of the buffer including the trailing NULL character.
 * This means that if you want to ask the user for a PIN of at most 4
 * characters you need to supply a buffer of at least @b 5 bytes and provide
 * a 5 as maxLen.
 *
 * @return Zero on success, nonzero when the user requested abort or there was
 * any error. The special value AB_ERROR_DEFAULT_VALUE should be returned if
 * the flag AB_BANKING_INPUT_FLAGS_ALLOW_DEFAULT was given and the user has
 * chosen to use the default value (e.g. pressed the "default" button in a
 * GUI).
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
 * It is required for every call to this function to be followed later
 * by a corresponding call to @ref AB_Banking_HideBox.
 * </p>
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @return returns an id to be presented to @ref AB_Banking_HideBox.
 * @param ab banking interface
 * @param flags flags, see @ref AB_BANKING_SHOWBOX_FLAGS_BEEP ff
 * @param title title of the box
 * @param text Text of the box: UTF-8, with both a normal text and a HTML variant of the text in the same string. See text restrictions note above.
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
 * </p>
 * <p>
 * This function must return immediately (i.e. it MUST NOT wait for
 * user interaction).
 * </p>
 * <p>
 * On graphical user interfaces such a dialog should contain a widget
 * for the text presented here, a progress bar, a text widget to
 * collect the log messages received via @ref AB_Banking_ProgressLog and
 * a button to allow the user to abort the current operation monitored by
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
 * @param text Text of the box: UTF-8, with both a normal text and a HTML variant of the text in the same string. See text restrictions note above.
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
 * @param text Text of the box: UTF-8, with both a normal text and a HTML variant of the text in the same string. See text restrictions note above.
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
 * receiption of this call but to simply enable a dialog closing (otherwise
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


/**
 * This function makes the application print something.
 * @param ab banking interface
 * @param docTitle title of the document. This might be presented to the user
 * @param docType an unique identifier of the document to be printed. This can
 *   be used by the application to separate printer settings for different
 *   document types. The name itself has no meaning and can be choosen freely
 *   by the caller. However, backends should append their name and a colon
 *   to keep this argument unique. This argument should not be translated.
 * @param descr an optional description about what the document contains. This
 *   might be shown to the user (see text restriction notes above).
 * @param text text to be printed (see text restriction notes above).
 */
AQBANKING_API 
int AB_Banking_Print(AB_BANKING *ab,
                     const char *docTitle,
                     const char *docType,
                     const char *descr,
                     const char *text);

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
AQBANKING_API 
void AB_Banking_SetPrintFn(AB_BANKING *ab,
                           AB_BANKING_PRINT_FN f);

/*@}*/


/** @name Virtual Security Functions And Associated Functions
 *
 */
/*@{*/

/**
 * This function retrieves the PIN for the given token. If the
 * application hasn't set any other GetPin function, this function
 * will call AB_Banking_InputBox for the user input.
 *
 * @param ab Banking interface
 * @param flags Flags, see @ref AB_Banking_InputBox
 *   and @ref AB_BANKING_INPUT_FLAGS_CONFIRM
 * @param token A unique identification of what PIN is required.
 *   To be used for automated PIN lookup.
 * @param title Title of the input box (in UTF-8)
 * @param text Text of the box: UTF-8, with both a normal text and a HTML
 *   variant of the text in the same string. See text restrictions note above.
 * @param buffer Buffer to store the response in. Must have at least room for
 *  @b maxLen bytes
 * @param minLen Minimal length of input that is required before the returned
 *   answer is accepted (if 0 then there is no low limit)
 * @param maxLen Size of the buffer including the trailing NULL character.
 * This means that if you want to ask the user for a PIN of at most 4
 * characters you need to supply a buffer of at least @b 5 bytes and provide
 * a 5 as maxLen.
 */
AQBANKING_API
int AB_Banking_GetPin(AB_BANKING *ab,
                      GWEN_TYPE_UINT32 flags,
                      const char *token,
                      const char *title,
                      const char *text,
                      char *buffer,
                      int minLen,
                      int maxLen);

/** 
 * Enable or disable the internal caching of PINs across jobs during
 * one full session. If the parameter is nonzero (TRUE), then every
 * entered PIN will be cached throughout the rest of this session and
 * is not asked for again. If the parameter is zero (FALSE), then
 * PINs will only be cached during one queue execution, but not across
 * several queue executions or several jobs. In other words, if this
 * is FALSE, then after each AB_Banking_ExecuteQueue() the internal
 * PIN cache will be cleared.
 *
 * Note: There has to be *some* Pin caching during the execution of
 * one job, because the medium is accessed several time. It would
 * therefore be very inconvenient for the user having to enter the
 * PIN several times during one job. Therefore we decided to implement
 * an internal PIN cache.
 *
 * @param ab The banking object
 *
 * @param enabled If nonzero (TRUE), then caching across jobs is
 * enabled. If zero (FALSE), caching across jobs is disabled.
 */
AQBANKING_API
void AB_Banking_SetPinCacheEnabled(AB_BANKING *ab, int enabled);

/** 
 * Returns nonzero (TRUE) if PIN caching across jobs until the end of
 * this session is enabled. Returns zero (FALSE) if caching across
 * jobs is disabled.
 */
AQBANKING_API
int AB_Banking_GetPinCacheEnabled(const AB_BANKING *ab);

/**
 * Sets a status for the given token and its given pin. This way,
 * aqbanking will keep track of whether an entered PIN might have been
 * wrong so it isn't used again.
 */
AQBANKING_API 
int AB_Banking_SetPinStatus(AB_BANKING *ab,
                            const char *token,
                            const char *pin,
                            AB_BANKING_PINSTATUS status);

/**
 * This function retrieves a TAN for the given token. If the
 * application hasn't set any other GetTan function, this function
 * will call AB_Banking_InputBox for the user input.
 */
AQBANKING_API 
int AB_Banking_GetTan(AB_BANKING *ab,
                      const char *token,
                      const char *title,
                      const char *text,
                      char *buffer,
                      int minLen,
                      int maxLen);

/**
 * Sets a status for the given token and its given TAN. This way,
 * applications can keep track of whether an entered TAN might have been
 * used.
 */
AQBANKING_API 
int AB_Banking_SetTanStatus(AB_BANKING *ab,
                            const char *token,
                            const char *tan,
                            AB_BANKING_TANSTATUS status);

/**
 * This influences the behaviour of AqBanking when new certificates are
 * received. If the returned value is !=0 then the user will be asked for
 * every single certificate received.
 */
AQBANKING_API
int AB_Banking_GetAlwaysAskForCert(const AB_BANKING *ab);

/**
 * This influences the behaviour of AqBanking when new certificates are
 * received. If the given value is !=0 then the user will be asked for
 * every single certificate received.
 */
AQBANKING_API 
void AB_Banking_SetAlwaysAskForCert(AB_BANKING *ab, int i);

/*@}*/



/** @name Setters For Virtual Security Functions
 *
 */
/*@{*/
AQBANKING_API 
void AB_Banking_SetGetPinFn(AB_BANKING *ab,
                            AB_BANKING_GETPIN_FN f);
AQBANKING_API 
void AB_Banking_SetSetPinStatusFn(AB_BANKING *ab,
                                  AB_BANKING_SETPINSTATUS_FN f);

AQBANKING_API 
void AB_Banking_SetGetTanFn(AB_BANKING *ab,
                            AB_BANKING_GETTAN_FN f);
AQBANKING_API 
void AB_Banking_SetSetTanStatusFn(AB_BANKING *ab,
                                  AB_BANKING_SETTANSTATUS_FN f);

/*@}*/



/** @name Getting Bank/Account Information
 *
 * Functions in this group retrieve information about credit institutes and
 * allow checking of bank code/account id combinations.
 * These functions load the appropriate checker plugins for selected
 * countries.
 */
/*@{*/
/**
 * This functions retrieves information about a given bank. It loads the
 * appropriate bank checker module and asks it for information about the given
 * bank. The caller is responsible for freeing the object returned (if any)
 * by calling @ref AB_BankingInfo_free.
 * @param ab AqBanking main object
 * @param country ISO country code ("de" for Germany, "at" for Austria etc)
 * @param branchId optional branch id (not needed for "de")
 * @param bankId bank id ("Bankleitzahl" for "de")
 */
AQBANKING_API 
AB_BANKINFO *AB_Banking_GetBankInfo(AB_BANKING *ab,
                                    const char *country,
                                    const char *branchId,
                                    const char *bankId);

/**
 * This function retrieves information about banks. It loads the
 * appropriate bank checker module and asks it for a list of AB_BANKINFO
 * objects which match the given template. Empty fields in this template
 * always match. Service entries (AB_BANKINFO_SERVICE) are not compared.
 * Matching entries are added to the given list.
 * The caller is responsible for freeing the objects returned (if any)
 * by calling @ref AB_BankingInfo_free (or by calling
 *  @ref AB_BankingInfo_List_freeAll).
 * @param ab AqBanking main object
 * @param country ISO country code ("de" for Germany, "at" for Austria etc)
 * @param tbi template to compare against
 * @param bl list to which matching banks are added
 */
AQBANKING_API 
int AB_Banking_GetBankInfoByTemplate(AB_BANKING *ab,
                                     const char *country,
                                     AB_BANKINFO *tbi,
                                     AB_BANKINFO_LIST2 *bl);


/**
 * This function checks whether the given combination represents a valid
 * account. It loads the appropriate bank checker module and lets it check
 * the information.
 * @param ab AqBanking main object
 * @param country ISO country code ("de" for Germany, "at" for Austria etc)
 * @param branchId optional branch id (not needed for "de")
 * @param bankId bank id ("Bankleitzahl" for "de")
 * @param accountId account id
 */
AQBANKING_API 
AB_BANKINFO_CHECKRESULT
AB_Banking_CheckAccount(AB_BANKING *ab,
                        const char *country,
                        const char *branchId,
                        const char *bankId,
                        const char *accountId);

/**
 * Checks whether a given international bank account number (IBAN) is
 * valid or not.
 * @return 0 if valid, 1 if not and -1 on error
 * @param iban IBAN (e.g. "DE88 2008 0000 09703 7570 0")
 */
AQBANKING_API
int AB_Banking_CheckIban(const char *iban);

/*@}*/


/** @name Getting Country Information
 *
 * Functions in this group retrieve information about countries (name,
 * code, numeric code).
 */
/*@{*/

/**
 * Searches for information about a country by its international name
 * (in English).
 * The name may contain jokers ("?") and wildcards ("*") and is case
 * insensitive.
 */
AQBANKING_API 
const AB_COUNTRY *AB_Banking_FindCountryByName(AB_BANKING *ab,
                                               const char *name);
/**
 * Searches for information about a country by its local name
 * (in the currently selected language).
 * The name may contain jokers ("?") and wildcards ("*") and is case
 * insensitive.
 */
AQBANKING_API 
const AB_COUNTRY *AB_Banking_FindCountryByLocalName(AB_BANKING *ab,
                                                    const char *name);
/**
 * Searches for information about a country by its ISO country code
 * (e.g. "DE"=Germany, "AT"=Austria etc).
 * The code may contain jokers ("?") and wildcards ("*") and is case
 * insensitive.
 */
AQBANKING_API 
const AB_COUNTRY *AB_Banking_FindCountryByCode(AB_BANKING *ab,
                                               const char *code);

/**
 * Searches for information about a country by its ISO numeric code
 * (e.g. 280=Germany etc).
 */
AQBANKING_API 
const AB_COUNTRY *AB_Banking_FindCountryByNumeric(AB_BANKING *ab,
                                                  int numid);

/**
 * Returns a list of informations about countries whose international name
 * (in English) matches the given argument.
 * The list returned must be freed using @ref AB_Country_ConstList2_free()
 * by the caller. The elements of that list are all const.
 * The name may contain jokers ("?") and wildcards ("*") and is case
 * insensitive.
 */
AQBANKING_API 
AB_COUNTRY_CONSTLIST2 *AB_Banking_ListCountriesByName(AB_BANKING *ab,
                                                      const char *name);
/**
 * Returns a list of informations about countries whose local name
 * (in the currently selected language) matches the given argument.
 * The list returned must be freed using @ref AB_Country_ConstList2_free()
 * by the caller. The elements of that list are all const.
 * The name may contain jokers ("?") and wildcards ("*") and is case
 * insensitive.
 */
AQBANKING_API 
AB_COUNTRY_CONSTLIST2 *AB_Banking_ListCountriesByLocalName(AB_BANKING *ab,
                                                           const char *name);



/*@}*/




/*@}*/ /* defgroup Middle Level API */


/** @defgroup G_AB_BANKING_IMEXPORT_API Im/Exporter API
 *
 * @short This group contains a very simple API.
 *
 * <p>
 * This API is intended to be used when adding support for AqBanking to
 * already existing applications. Basically this API just defines some
 * functions which enqueue requests (such as "Get balance") and a function
 * which gathers answers to all requests into an importer/exporter context.
 * </p>
 * <p>
 * This way your application only needs to provide the functionality to
 * import such a context.
 * </p>
 * <p>
 * Mixing the Job-API with this API is allowed.
 * </p>
 * <p>
 * A program should first call @ref AB_Banking_Init to allow AqBanking
 * to load its configuration files and initialize itself.
 * </p>
 * <p>
 * You need to setup AqBanking before using functions of this group (i.e.
 * accounts must be setup).
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
 * <p>
 * The following is a small example of how the High Level API might be used.
 * It requests the status (e.g. balance etc) of an account.
 * </p>
 * @code
 * TODO: Add example
 * @endcode
 */
/*@{*/



/** @name Request Functions
 *
 */
/*@{*/
/**
 * This function enqueues a request for the balance of an account.
 * You need to call @ref AB_Banking_ExecuteQueue to actually perform the
 * job.
 */
AQBANKING_API 
int AB_Banking_RequestBalance(AB_BANKING *ab,
                              const char *bankCode,
                              const char *accountNumber);

/**
 * This function enqueues a request for the transactions of an account.
 * You need to call @ref AB_Banking_ExecuteQueue to actually perform the
 * job.
 * Please note that not all backends and all banks allow a time span to be
 * given to this function. In such cases the dates are simply ignored.
 */
AQBANKING_API 
int AB_Banking_RequestTransactions(AB_BANKING *ab,
                                   const char *bankCode,
                                   const char *accountNumber,
                                   const GWEN_TIME *firstDate,
                                   const GWEN_TIME *lastDate);

/**
 * This function enqueues a request for the standing orders of an account.
 * You need to call @ref AB_Banking_ExecuteQueue to actually perform the
 * job.
 */
AQBANKING_API 
int AB_Banking_RequestStandingOrders(AB_BANKING *ab,
                                     const char *bankCode,
                                     const char *accountNumber);

/**
 * This function enqueues a request for the list of pending dated transfers of
 * an account.
 * You need to call @ref AB_Banking_ExecuteQueue to actually perform the
 * job.
 */
AQBANKING_API 
int AB_Banking_RequestDatedTransfers(AB_BANKING *ab,
                                     const char *bankCode,
                                     const char *accountNumber);


/**
 * <p>
 * This functions gathers all available information from the results of
 * all currently finished jobs. Jobs which have been evaluated by this
 * function and which had been created by the calling application are
 * automatically removed from the queue of finished jobs.
 * </p>
 * <p>
 * Please note that this function even returns information extracted from
 * requests which have been issued by other applications (those finished
 * jobs are @b not removed from the finished queue to allow the requesting
 * application to later gather its own responses).
 * </p>
 * <p>
 * All information is added to the given import context. Thus you only need
 * a single function in your application to import data read from a file
 * (via the Import/Export API) and from online requests.
 * </p>
 * @return 0 if ok, !=0 on error (see @ref AB_ERROR)
 * @param ab AqBanking main object
 * @param ctx import/export context to which all information is to be added
 */
AQBANKING_API 
int AB_Banking_GatherResponses(AB_BANKING *ab,
                               AB_IMEXPORTER_CONTEXT *ctx);
/*@}*/



/** @name Mapping Application Accounts to Online Accounts
 *
 * Functions in this group provide an optional service for account mapping.
 * Most applications assign unique ids to their own accounts. This unique
 * id can be mapped to an account of AqBanking.
 */
/*@{*/
/**
 * <p>
 * Sets an alias for the given AqBanking account. You can later use
 * @ref AB_Banking_GetAccountByAlias to refer to an online account by using
 * the unique id of your application's account.
 * </p>
 * <p>
 * AqBanking separates the aliases for each application.
 * </p>
 * @param ab AqBanking main object
 * @param a online account of AqBanking you wish to map your account to
 * @param alias unique id of your application's own account structure
 */
AQBANKING_API 
void AB_Banking_SetAccountAlias(AB_BANKING *ab,
                                AB_ACCOUNT *a, const char *alias);

/**
 * This function returns the AqBanking account to which the given
 * alias (=unique id of your application's own account data) has been
 * mapped.
 *
 * AqBanking remains the owner of the object returned (if any), so you must
 * not free it.
 *
 * Please also note that the object returned is only valid until
 * AB_Banking_Fini() has been called (or until the corresponding backend for
 * this particular account has been deactivated).
 *
 * @return corresponding AqBanking (or 0 if none)
 * @param ab AqBanking main object
 * @param alias unique id of your application's own account structure
 */
AQBANKING_API 
AB_ACCOUNT *AB_Banking_GetAccountByAlias(AB_BANKING *ab,
                                         const char *alias);
/*@}*/



/*@}*/ /* defgroup Application Level API */



/*@}*/ /* defgroup AB_BANKING */


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_BANKING_H */






