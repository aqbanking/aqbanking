/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/** @file aqbanking/provider.h
 * @short This file is used by AqBanking and provider backends.
 */


#ifndef AQBANKING_PROVIDER_H
#define AQBANKING_PROVIDER_H

#include <aqbanking/error.h> /* for AQBANKING_API */

#include <gwenhywfar/misc.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/xml.h>


#define AB_PROVIDER_FLAGS_COMPLETE_DAY_REPORTS 0x00000001


#ifdef __cplusplus
extern "C" {
#endif

typedef struct AB_PROVIDER AB_PROVIDER;
GWEN_INHERIT_FUNCTION_DEFS(AB_PROVIDER)
GWEN_LIST2_FUNCTION_DEFS(AB_PROVIDER, AB_Provider)

typedef struct AB_PROVIDER_DESCRIPTION AB_PROVIDER_DESCRIPTION;
GWEN_INHERIT_FUNCTION_DEFS(AB_PROVIDER_DESCRIPTION)
GWEN_LIST_FUNCTION_DEFS(AB_PROVIDER_DESCRIPTION, AB_ProviderDescription)
GWEN_LIST2_FUNCTION_DEFS(AB_PROVIDER_DESCRIPTION, AB_ProviderDescription)

#ifdef __cplusplus
}
#endif


#include <aqbanking/banking.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup G_AB_PROVIDER
 *
 * @brief This group represents backends. (Don't use in applications)
 *
 * (<i>Provider</i> is simply another word for <i>backend</i>.)
 *
 * See https://www.aquamaniac.de/rdm/projects/aqbanking/wiki/Develop_backend
 */
/*@{*/

/**
 * Returns the name of the backend (e.g. "aqhbci").
 */
const char *AB_Provider_GetName(const AB_PROVIDER *pro);

/**
 * Returns the escaped name of the backend. This is needed when using the
 * name of the backend to form a file path.
 */
const char *AB_Provider_GetEscapedName(const AB_PROVIDER *pro);
/**
 * Returns the Banking object that this Provider belongs to.
 */
AB_BANKING *AB_Provider_GetBanking(const AB_PROVIDER *pro);


uint32_t AB_Provider_GetFlags(const AB_PROVIDER *pro);

/**
 * This copies the name of the folder for AqBanking's backend data into
 * the given GWEN_Buffer. This folder is reserved for this backend.
 * Please note that this folder does not necessarily exist, but the backend
 * is free to create it.
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param pro pointer to the provider object
 * @param buf buffer to append the path name to
 */
int AB_Provider_GetUserDataDir(const AB_PROVIDER *pro, GWEN_BUFFER *buf);


/*@}*/ /* defgroup */

#ifdef __cplusplus
}
#endif




#endif /* AQBANKING_PROVIDER_H */









