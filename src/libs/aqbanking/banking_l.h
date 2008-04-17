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


#ifndef AQBANKING_BANKING_L_H
#define AQBANKING_BANKING_L_H


#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/bankinfoplugin_be.h>



/**
 * This copies the name of the folder for AqBanking's backend data into
 * the given GWEN_Buffer (not including the provider's name).
 * @return 0 if ok, error code otherwise (see @ref AB_ERROR)
 * @param ab pointer to the AB_BANKING object
 * @param buf buffer to append the path name to
 */
int AB_Banking_GetProviderUserDataDir(const AB_BANKING *ab,
                                      const char *name,
                                      GWEN_BUFFER *buf);

/**
 * Store backend specific data with AqBanking. This data is not specific
 * to an application, it will rather be used with every application (since
 * it doesn't depend on the application but on the backend).
 * @param ab pointer to the AB_BANKING object
 * @param pro pointer to the backend for which the data is to be returned
 */
GWEN_DB_NODE *AB_Banking_GetProviderData(AB_BANKING *ab,
                                         AB_PROVIDER *pro);


int AB_Banking_IsOnlineInit(const AB_BANKING *ab);

int AB_Banking_PluginSystemInit(void);
int AB_Banking_PluginSystemFini(void);


#endif /* AQBANKING_BANKING_L_H */
