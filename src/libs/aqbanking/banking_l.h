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


#ifndef AQBANKING_BANKING_L_H
#define AQBANKING_BANKING_L_H


#include <aqbanking/banking.h>


GWEN_TYPE_UINT32 AB_Banking_GetUniqueId(AB_BANKING *ab);

/**
 * Loads a backend with the given name. You can use
 * @ref AB_Banking_GetProviderDescrs to retrieve a list of available
 * backends. Such a backend can then be asked to return an account list.
 */
AB_PROVIDER *AB_Banking_GetProvider(AB_BANKING *ab, const char *name);



/** @name Functions For Loading Provider Plugins
 *
 */
/*@{*/
AB_PROVIDER *AB_Banking_LoadProviderPluginFile(AB_BANKING *ab,
                                               const char *modname,
                                               const char *fname);


AB_PROVIDER *AB_Banking_LoadProviderPlugin(AB_BANKING *ab,
                                           const char *modname);
/*@}*/



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
/*@}*/









#endif /* AQBANKING_BANKING_L_H */
