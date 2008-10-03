/***************************************************************************
 begin       : Sat Sep 27 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_BANKING_CFG_H
#define AQBANKING_BANKING_CFG_H

#include <aqbanking/provider.h>


#ifdef __cplusplus
extern "C" {
#endif



AQBANKING_API
int AB_Banking_LoadAppConfig(AB_BANKING *ab, GWEN_DB_NODE **pDb);

AQBANKING_API
int AB_Banking_SaveAppConfig(AB_BANKING *ab, GWEN_DB_NODE *db);


AQBANKING_API
int AB_Banking_LockAppConfig(AB_BANKING *ab);

AQBANKING_API
int AB_Banking_UnlockAppConfig(AB_BANKING *ab);




AQBANKING_API
int AB_Banking_LoadSharedConfig(AB_BANKING *ab,
				const char *name,
				GWEN_DB_NODE **pDb);

AQBANKING_API
int AB_Banking_SaveSharedConfig(AB_BANKING *ab,
				const char *name,
				GWEN_DB_NODE *db);


AQBANKING_API
int AB_Banking_LockSharedConfig(AB_BANKING *ab, const char *name);

AQBANKING_API
int AB_Banking_UnlockSharedConfig(AB_BANKING *ab, const char *name);


#ifdef __cplusplus
}
#endif


#endif
