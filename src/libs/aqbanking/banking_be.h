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


#ifndef AQBANKING_BANKING_BE_H
#define AQBANKING_BANKING_BE_H

#include <aqbanking/banking.h>


#ifdef __cplusplus
extern "C" {
#endif


/** @name Functions Used by Backends And Wizards
 *
 */
/*@{*/

/**
 * Store backend specific data with AqBanking. This data is not specific
 * to an application, it will rather be used with every application (since
 * it doesn't depend on the application but on the backend).
 * @param ab pointer to the AB_BANKING object
 * @param pro pointer to the backend for which the data is to be returned
 */
AQBANKING_API 
GWEN_DB_NODE *AB_Banking_GetProviderData(AB_BANKING *ab,
                                         const AB_PROVIDER *pro);

/**
 * This copies the name of the folder for AqBanking's backend data into
 * the given GWEN_Buffer (not including the provider's name).
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param buf buffer to append the path name to
 */
AQBANKING_API 
int AB_Banking_GetProviderUserDataDir(const AB_BANKING *ab, GWEN_BUFFER *buf);


/**
 * Loads a backend with the given name. You can use
 * @ref AB_Banking_GetProviderDescrs to retrieve a list of available
 * backends. Such a backend can then be asked to return an account list.
 */
AQBANKING_API 
AB_PROVIDER *AB_Banking_GetProvider(AB_BANKING *ab, const char *name);

/*@}*/


#ifdef __cplusplus
}
#endif


#endif /* AQBANKING_BANKING_BE_H */






