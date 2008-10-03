/***************************************************************************
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

#include <gwenhywfar/configmgr.h>


int AB_Banking_IsOnlineInit(const AB_BANKING *ab);

int AB_Banking_PluginSystemInit(void);
int AB_Banking_PluginSystemFini(void);

GWEN_CONFIGMGR *AB_Banking_GetConfigMgr(AB_BANKING *ab);


#endif /* AQBANKING_BANKING_L_H */
