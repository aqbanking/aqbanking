/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/** @file banking_be.h
 * @short This file is used by provider backends.
 */


#ifndef AQBANKING_BANKING_BE_H
#define AQBANKING_BANKING_BE_H

#include <gwenhywfar/db.h>

#include <aqbanking/banking.h>
#include <aqbanking/banking6_be.h>

#include <gwenhywfar/httpsession.h>


#define AB_CFG_GROUP_BACKENDS   "backends"
#define AB_CFG_GROUP_BANKINFO   "bankinfo"
#define AB_CFG_GROUP_IMEXPORTER "imexporter"

#define AB_CFG_GROUP_USERS      "users"
#define AB_CFG_GROUP_ACCOUNTS   "accounts"



/** @addtogroup G_AB_BE_BANKING
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif


/** @name Functions Used by Backends And Wizards
 *
 */
/*@{*/

/**
 * Returns the list of global data folders. In most cases this is something
 * like $PREFIX/share/. Plugins are required to use the folders
 * returned here + "aqbanking" when searching for their specific data instead
 * of using the compile time fixed values. This way it is easier under
 * windows to find data.
 */
AQBANKING_API
GWEN_STRINGLIST *AB_Banking_GetGlobalDataDirs(void);


AQBANKING_API
GWEN_STRINGLIST *AB_Banking_GetGlobalSysconfDirs(void);

/*@}*/



/** @name Administration of Crypt Token List
 *
 */
/*@{*/
AQBANKING_API
int AB_Banking_GetCryptToken(AB_BANKING *ab,
			     const char *tname,
			     const char *cname,
			     GWEN_CRYPT_TOKEN **pCt);

AQBANKING_API 
void AB_Banking_ClearCryptTokenList(AB_BANKING *ab);

AQBANKING_API 
int AB_Banking_CheckCryptToken(AB_BANKING *ab,
			       GWEN_CRYPT_TOKEN_DEVICE devt,
			       GWEN_BUFFER *typeName,
			       GWEN_BUFFER *tokenName);

/*@}*/


AQBANKING_API DEPRECATED
int AB_Banking_GetUniqueId(AB_BANKING *ab);

/**
 * Get a named unique id.
 * Previously there was only one source for unique ids which was used for everything, fastly increasing that id.
 * Now new id counters can be incremented separately.
 * @param ab pointer to AB_BANKING object
 * @param idName name of the id to get (e.g. "account", "user", "job" etc)
 * @param startAtStdUniqueId if the given id is zero and this var is !=0 start with the current standard uniqueId
 */
AQBANKING_API int AB_Banking_GetNamedUniqueId(AB_BANKING *ab, const char *idName, int startAtStdUniqueId);


AQBANKING_API
int AB_Banking_GetCert(AB_BANKING *ab,
                       const char *url,
                       const char *defaultProto,
                       int defaultPort,
                       uint32_t *httpFlags,
                       uint32_t pid);


/**
 * This copies the name of the folder for AqBanking's backend data into
 * the given GWEN_Buffer (not including the provider's name).
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param buf buffer to append the path name to
 */
AQBANKING_API
int AB_Banking_GetProviderUserDataDir(const AB_BANKING *ab,
                                      const char *name,
                                      GWEN_BUFFER *buf);




AQBANKING_API
AB_PROVIDER *AB_Banking_BeginUseProvider(AB_BANKING *ab, const char *modname);

AQBANKING_API
int AB_Banking_EndUseProvider(AB_BANKING *ab, AB_PROVIDER *pro);




#ifdef __cplusplus
}
#endif

/*@}*/


#endif /* AQBANKING_BANKING_BE_H */






