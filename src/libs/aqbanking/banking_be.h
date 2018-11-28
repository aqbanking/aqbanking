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

#include <gwenhywfar/httpsession.h>
#include <gwenhywfar/ct.h>


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
 * the given GWEN_Buffer.
 *
 * An example path would be "/home/USER/.aqbanking/backends/aqhbci/data".
 *
 * @return 0 if ok, error code otherwise (see @ref GWEN_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param name name of the online banking provider (e.g. "aqhbci")
 * @param buf buffer to append the path name to
 */
AQBANKING_API
int AB_Banking_GetProviderUserDataDir(const AB_BANKING *ab,
                                      const char *name,
                                      GWEN_BUFFER *buf);



/**
 * Loads the given provider and initializes it.
 * Only after calling this function the provider can be used.
 * You need to call @ref AB_Banking_EndUseProvider() if you're done.
 *
 * @return 0 if ok, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object (needs to be initialized, i.e. @ref AB_Banking_Init called).
 * @param modname (e.g. "aqhbci")
 */
AQBANKING_API
AB_PROVIDER *AB_Banking_BeginUseProvider(AB_BANKING *ab, const char *modname);

/**
 * Call this as soon as the provider isn't actually needed anymore.
 * This probably unloads the plugin, at least it is deinitialized.
 *
 * @return 0 if ok, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object (needs to be initialized, i.e. @ref AB_Banking_Init called).
 * @param pro pointer to provider object returned by @ref AB_Banking_BeginUseProvider
 *
 */
AQBANKING_API
int AB_Banking_EndUseProvider(AB_BANKING *ab, AB_PROVIDER *pro);




/** @name Account Spec Management Functions
 *
 */
/*@{*/

AQBANKING_API int AB_Banking_ReadAccountSpec(AB_BANKING *ab, uint32_t uniqueId, AB_ACCOUNT_SPEC **pAccountSpec);
AQBANKING_API int AB_Banking_WriteAccountSpec(AB_BANKING *ab, const AB_ACCOUNT_SPEC *accountSpec);
AQBANKING_API int AB_Banking_DeleteAccountSpec(AB_BANKING *ab, uint32_t uid);

/*@}*/





/**
 * @return 0 if there are some groups, error code otherwise (especially GWEN_ERROR_PARTIAL if some groups couldn't be read
 *           and GWEN_ERROR_NOT_FOUND if there no groups found).
 * @param ab AQBANKING object
 * @param groupName name of the config group
 * @param uidField name of an integer variable in the config groups which MUST NOT be zero (NULL to skip this test)
 * @param matchVar name of a variable in the config groups whose value must match matchVal (NULL to skip this test)
 * @param matchVal value to match the matchVar variable (NULL for empty value)
 * @param pDb pointer to a variable to receive the newly created DB, each subgroup contains a config group
 */
AQBANKING_API int AB_Banking_ReadConfigGroups(AB_BANKING *ab,
                                              const char *groupName,
                                              const char *uidField,
                                              const char *matchVar,
                                              const char *matchVal,
                                              GWEN_DB_NODE **pDb);




#ifdef __cplusplus
}
#endif

/*@}*/


#endif /* AQBANKING_BANKING_BE_H */






