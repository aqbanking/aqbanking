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
int AB_Banking_LoadAppConfig(AB_BANKING *ab, GWEN_DB_NODE **pDb, uint32_t guiid);

AQBANKING_API
int AB_Banking_SaveAppConfig(AB_BANKING *ab, GWEN_DB_NODE *db, uint32_t guiid);


AQBANKING_API
int AB_Banking_LockAppConfig(AB_BANKING *ab, uint32_t guiid);

AQBANKING_API
int AB_Banking_UnlockAppConfig(AB_BANKING *ab, uint32_t guiid);




AQBANKING_API
int AB_Banking_LoadSharedConfig(AB_BANKING *ab,
				const char *name,
				GWEN_DB_NODE **pDb,
				uint32_t guiid);

AQBANKING_API
int AB_Banking_SaveSharedConfig(AB_BANKING *ab,
				const char *name,
				GWEN_DB_NODE *db,
				uint32_t guiid);


AQBANKING_API
int AB_Banking_LockSharedConfig(AB_BANKING *ab, const char *name, uint32_t guiid);

AQBANKING_API
int AB_Banking_UnlockSharedConfig(AB_BANKING *ab, const char *name, uint32_t guiid);



AQBANKING_API
int AB_Banking_BeginExclUseAccount(AB_BANKING *ab, AB_ACCOUNT *a, uint32_t guiid);

AQBANKING_API
int AB_Banking_EndExclUseAccount(AB_BANKING *ab, AB_ACCOUNT *a, int abandon, uint32_t guiid);



AQBANKING_API
int AB_Banking_BeginExclUseUser(AB_BANKING *ab, AB_USER *u, uint32_t guiid);

AQBANKING_API
int AB_Banking_EndExclUseUser(AB_BANKING *ab, AB_USER *u, int abandon, uint32_t guiid);



/** @name Checking Configuration for AqBanking4
 *
 */
/*@{*/
AQBANKING_API
int AB_Banking_HasConf4(AB_BANKING *ab, uint32_t guiid);
/*@}*/


/** @name Importing Configuration from AqBanking3
 *
 */
/*@{*/

AQBANKING_API
int AB_Banking_HasConf3(AB_BANKING *ab, uint32_t guiid);

/**
 * This function imports the configuration of AqBanking3.
 */
AQBANKING_API
int AB_Banking_ImportConf3(AB_BANKING *ab, uint32_t guiid);
/*@}*/


/** @name Importing Configuration from AqBanking2
 *
 */
/*@{*/

AQBANKING_API
int AB_Banking_HasConf2(AB_BANKING *ab, uint32_t guiid);

/**
 * This function imports the configuration of AqBanking2.
 */
AQBANKING_API
int AB_Banking_ImportConf2(AB_BANKING *ab, uint32_t guiid);

/*@}*/


#ifdef __cplusplus
}
#endif


#endif
