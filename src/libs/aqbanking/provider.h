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


AQBANKING_API
const char *AB_Provider_GetName(const AB_PROVIDER *pro);
AQBANKING_API
AB_BANKING *AB_Provider_GetBanking(const AB_PROVIDER *pro);



/*@}*/ /* defgroup */

#ifdef __cplusplus
}
#endif




#endif /* AQBANKING_PROVIDER_H */









