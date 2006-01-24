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


typedef int (*GBANKING_IMPORTCONTEXT_FN)(AB_BANKING *ab,
                                         AB_IMEXPORTER_CONTEXT *ctx);

GBANKING_API 
AB_BANKING *GBanking_new(const char *appName,
                         const char *fname);

GBANKING_API 
const char *GBanking_GetCharSet(const AB_BANKING *ab);

GBANKING_API 
void GBanking_SetCharSet(AB_BANKING *ab, const char *s);


GBANKING_API 
void GBanking_GetRawText(AB_BANKING *ab,
                         const char *text,
                         GWEN_BUFFER *tbuf);

GBANKING_API 
void GBanking_GetHtmlText(AB_BANKING *ab,
                          const char *text,
                          GWEN_BUFFER *tbuf);

GBANKING_API 
void GBanking_GetUtf8Text(AB_BANKING *ab,
                          const char *text,
                          int len,
                          GWEN_BUFFER *tbuf);



GBANKING_API 
GWEN_TYPE_UINT32 GBanking_GetLastAccountUpdate(const AB_BANKING *ab);

GBANKING_API 
GWEN_TYPE_UINT32 GBanking_GetLastQueueUpdate(const AB_BANKING *ab);

GBANKING_API 
void GBanking_AccountsUpdated(AB_BANKING *ab);

GBANKING_API 
void GBanking_QueueUpdated(AB_BANKING *ab);

GBANKING_API 
int GBanking_ImportContext(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx);

GBANKING_API 
void GBanking_SetImportContextFn(AB_BANKING *ab,
                                 GBANKING_IMPORTCONTEXT_FN cb);


#endif /* GBANKING_H */









