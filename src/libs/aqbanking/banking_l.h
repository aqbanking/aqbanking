/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_BANKING_L_H
#define AQBANKING_BANKING_L_H


#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/backendsupport/bankinfoplugin_be.h>

#include <gwenhywfar/configmgr.h>
#include <gwenhywfar/stringlist.h>



/* ========================================================================================================================
 *                                                banking_account.c
 * ========================================================================================================================
 */

int AB_Banking_Read_AccountConfig(const AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE **pDb);
int AB_Banking_Has_AccountConfig(const AB_BANKING *ab, uint32_t uid);
int AB_Banking_Write_AccountConfig(AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE *db);
int AB_Banking_Delete_AccountConfig(AB_BANKING *ab, uint32_t uid);
int AB_Banking_Unlock_AccountConfig(AB_BANKING *ab, uint32_t uid);


/* ========================================================================================================================
 *                                                banking_user.c
 * ========================================================================================================================
 */

int AB_Banking_Read_UserConfig(const AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE **pDb);
int AB_Banking_Has_UserConfig(const AB_BANKING *ab, uint32_t uid);
int AB_Banking_Write_UserConfig(AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE *db);
int AB_Banking_Delete_UserConfig(AB_BANKING *ab, uint32_t uid);
int AB_Banking_Unlock_UserConfig(AB_BANKING *ab, uint32_t uid);



#endif /* AQBANKING_BANKING_L_H */
