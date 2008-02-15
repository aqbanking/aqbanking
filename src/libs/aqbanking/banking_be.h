/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
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

#include <aqbanking/banking.h>



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
 * Loads a backend with the given name. You can use
 * @ref AB_Banking_GetProviderDescrs to retrieve a list of available
 * backends. Such a backend can then be asked to return an account list.
 */
AQBANKING_API 
AB_PROVIDER *AB_Banking_GetProvider(AB_BANKING *ab, const char *name);


/**
 * Returns the list of global data folders. In most cases this is something
 * like $PREFIX/share/. Plugins are required to use the folders
 * returned here + "aqbanking" when searching for their specific data instead
 * of using the compile time fixed values. This way it is easier under
 * windows to find data.
 */
AQBANKING_API
GWEN_STRINGLIST *AB_Banking_GetGlobalDataDirs();


AQBANKING_API
GWEN_STRINGLIST *AB_Banking_GetGlobalSysconfDirs();

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
void AB_Banking_ClearCryptTokenList(AB_BANKING *ab, uint32_t guiid);

AQBANKING_API 
int AB_Banking_CheckCryptToken(AB_BANKING *ab,
			       GWEN_CRYPT_TOKEN_DEVICE devt,
			       GWEN_BUFFER *typeName,
			       GWEN_BUFFER *tokenName,
			       uint32_t guiid);

/*@}*/


AQBANKING_API
int AB_Banking_ExecutionProgress(AB_BANKING *ab, uint32_t pid);



#ifdef __cplusplus
}
#endif

/*@}*/


#endif /* AQBANKING_BANKING_BE_H */






