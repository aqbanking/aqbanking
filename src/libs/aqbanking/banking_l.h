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


GWEN_DB_NODE *AB_Banking_GetWizzardData(AB_BANKING *ab,
                                        const AB_PROVIDER_WIZZARD *pw);

GWEN_TYPE_UINT32 AB_Banking_GetUniqueId(AB_BANKING *ab);

/**
 * Loads a backend with the given name. You can use
 * @ref AB_Banking_GetProviderDescrs to retrieve a list of available
 * backends. Such a backend can then be asked to return an account list.
 */
AB_PROVIDER *AB_Banking_GetProvider(AB_BANKING *ab, const char *name);


#endif /* AQBANKING_BANKING_L_H */
