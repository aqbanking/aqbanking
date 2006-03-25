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


#ifndef AQBANKING_BANKING_VIRT_H
#define AQBANKING_BANKING_VIRT_H


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup G_AB_VIRTUAL
 *
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



/** @name Special Progress Values for AB_Banking_ProgressAdvance
 *
 */
/*@{*/
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


/*@}*/ /* addtogroup */

#ifdef __cplusplus
}
#endif


#endif
