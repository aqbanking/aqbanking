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
#include <aqbanking/banking_be.h>


GWEN_TYPE_UINT32 AB_Banking_GetUniqueId(AB_BANKING *ab);


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




/** @name Functions For Loading Im/Exporters
 *
 */
/*@{*/
AB_IMEXPORTER *AB_Banking_LoadImExporterPluginFile(AB_BANKING *ab,
                                                   const char *modname,
                                                   const char *fname);
AB_IMEXPORTER *AB_Banking_LoadImExporterPlugin(AB_BANKING *ab,
                                               const char *modname);
/*@}*/



/**
 * This function loads the given backend (if it not already has been) and
 * imports any account that backend might offer. You can use this function
 * to engage a backend which has not yet been used (but it doesn't hurt if you
 * use it on already active backends).
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab banking interface
 * @param backend name of the backend (such as "aqhbci". You can retrieve
 * such a name either from the list of active backends
 * (@ref AB_Banking_GetActiveProviders) or from an plugin description
 * retrieved via @ref AB_Banking_GetProviderDescrs (call
 * @ref GWEN_PluginDescription_GetName on that plugin description).
 */
AQBANKING_API 
int AB_Banking_ImportProviderAccounts(AB_BANKING *ab, const char *backend);






#endif /* AQBANKING_BANKING_L_H */
