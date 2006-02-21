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



#ifndef GBANKING_H
#define GBANKING_H

#include <gtk/gtk.h>
#include <aqbanking/system.h>
#include <aqbanking/banking.h>
#include <aqbanking/imexporter.h>

#include <gwenhywfar/buffer.h>

#define GBANKING_LOGDOMAIN "gbanking"


#ifdef BUILDING_GBANKING
# /* building AqBanking */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define GBANKING_API __declspec (dllexport)
#   else /* if __declspec */
#     define GBANKING_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define GBANKING_API __attribute__((visibility("default")))
#   else
#     define GBANKING_API
#   endif
# endif
#else
# /* not building AqBanking */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define GBANKING_API __declspec (dllimport)
#   else /* if __declspec */
#     define GBANKING_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define GBANKING_API
# endif
#endif

#ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
# define GBANKING_EXPORT __attribute__((visibility("default")))
# define GBANKING_NOEXPORT __attribute__((visibility("hidden")))
#else
# define GBANKING_EXPORT
# define GBANKING_NOEXPORT
#endif


/** @addtogroup G_AB_G2BANKING
 *
 */
/*@{*/
typedef int (*GBANKING_IMPORTCONTEXT_FN)(AB_BANKING *ab,
                                         AB_IMEXPORTER_CONTEXT *ctx);

GBANKING_API 
AB_BANKING *GBanking_new(const char *appName,
                         const char *fname);


/** @name Character Set
 *
 * <p>
 * AqBanking internally uses UTF8 in all functions. Not all systems use this
 * for user interaction (e.g. not all desktops show UTF-8 data, some
 * systems might still use ISO-8859-1 or 15.
 * </p>
 * <p>
 * Therefore this frontend transforms interactive data (i.e. data that is
 * output to the desktop or input from it) between UTF-8 and a given character
 * set.
 * </p>
 *
 */
/*@{*/

/**
 * Returns the currently selected character set (or NULL if none is selected).
 */
GBANKING_API
const char *GBanking_GetCharSet(const AB_BANKING *ab);

/**
 * Set the character set to transform from/to.
 */
GBANKING_API 
void GBanking_SetCharSet(AB_BANKING *ab, const char *s);
/*@}*/



/** @name Helper Functions
 *
 */
/*@{*/

/**
 * AqBanking allows HTML text for some arguments of interactive functions
 * (e.g. @ref AB_Banking_MessageBox). If HTML is provided, then a text-only
 * version must be supplied so that frontends incapable of parsing HTML can
 * still output something usefull.
 * With GTK2 we use Pango which only supports a subset of HTML, so we use
 * the raw text version.
 * This function extracts the raw text from the argument and stores it
 * in the given buffer.
 */
GBANKING_API
void GBanking_GetRawText(AB_BANKING *ab,
                         const char *text,
                         GWEN_BUFFER *tbuf);


/**
 * See @ref GBanking_GetRawText.
 * This function extracts the HTML text from the argument and stores it
 * in the given buffer.
 */
GBANKING_API
void GBanking_GetHtmlText(AB_BANKING *ab,
                          const char *text,
                          GWEN_BUFFER *tbuf);

/**
 * This function extracts the raw text from the argument as the function
 * @ref GBanking_GetRawText would do. In addition this function converts
 * the raw text from UTF-8 to the character set selected via
 * @ref GBanking_SetCharSet.
 */
GBANKING_API 
void GBanking_GetUtf8Text(AB_BANKING *ab,
                          const char *text,
                          int len,
                          GWEN_BUFFER *tbuf);
/*@}*/



/** @name Update Tracker Functions
 *
 * The functions in this group just handle counters. E.g. if you call
 * @ref GBanking_AccountsUpdated then the internal account update counter is
 * incremented. Functions which want to check whether accounts have been
 * updated since the last time they checked just retrieve the account update
 * counter via @ref GBanking_GetLastAccountUpdate and compare it against
 * their own counter.
 */
/*@{*/

GBANKING_API 
void GBanking_AccountsUpdated(AB_BANKING *ab);

GBANKING_API 
void GBanking_QueueUpdated(AB_BANKING *ab);

GBANKING_API
GWEN_TYPE_UINT32 GBanking_GetLastAccountUpdate(const AB_BANKING *ab);

GBANKING_API 
GWEN_TYPE_UINT32 GBanking_GetLastQueueUpdate(const AB_BANKING *ab);
/*@}*/



/** @name Importing Contexts
 *
 * This frontend uses a callback function to let the application
 * import data. This is called from the job view after executing the job
 * queue.
 */
/*@{*/
GBANKING_API 
int GBanking_ImportContext(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx);

/**
 * Set the import callback function.
 */
GBANKING_API 
void GBanking_SetImportContextFn(AB_BANKING *ab,
                                 GBANKING_IMPORTCONTEXT_FN cb);
/*@}*/

/*@}*/


#endif /* GBANKING_H */









