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


#define AB_PROVIDER_FLAGS_COMPLETE_DAY_REPORTS 0x00000001


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
GWEN_LIST_FUNCTION_LIB_DEFS(AB_PROVIDER_DESCRIPTION, AB_ProviderDescription,
                            AQBANKING_API)
GWEN_LIST2_FUNCTION_LIB_DEFS(AB_PROVIDER_DESCRIPTION, AB_ProviderDescription,
                             AQBANKING_API)

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

/** @defgroup G_AB_PROVIDER AB_PROVIDER (Online Banking Backends)
 * @ingroup G_AB_C_INTERFACE
 *
 * @brief This group represents backends.
 */
/*@{*/


AQBANKING_API
const char *AB_Provider_GetName(const AB_PROVIDER *pro);
AQBANKING_API
const char *AB_Provider_GetEscapedName(const AB_PROVIDER *pro);
AQBANKING_API
AB_BANKING *AB_Provider_GetBanking(const AB_PROVIDER *pro);


AQBANKING_API
GWEN_TYPE_UINT32 AB_Provider_GetFlags(const AB_PROVIDER *pro);

/**
 * This copies the name of the folder for AqBanking's backend data into
 * the given GWEN_Buffer. This folder is reserved for this backend.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param pro pointer to the provider object
 * @param buf buffer to append the path name to
 */
AQBANKING_API
int AB_Provider_GetUserDataDir(const AB_PROVIDER *pro, GWEN_BUFFER *buf);

/**
 * Store backend specific data with AqBanking. This data is not specific
 * to an application, it will rather be used with every application (since
 * it doesn't depend on the application but on the backend).
 * @param pro pointer to the backend for which the data is to be returned
 */
AQBANKING_API
GWEN_DB_NODE *AB_Provider_GetData(AB_PROVIDER *pro);



/*@}*/ /* defgroup */

#ifdef __cplusplus
}
#endif




#endif /* AQBANKING_PROVIDER_H */









