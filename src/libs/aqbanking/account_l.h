/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_ACCOUNT_L_H
#define AQBANKING_ACCOUNT_L_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/fslock.h>

#include <aqbanking/account_be.h>


int AB_Account_Update(AB_ACCOUNT *a);

void AB_Account_SetProvider(AB_ACCOUNT *a, AB_PROVIDER *pro);


const char *AB_Account_GetDbId(const AB_ACCOUNT *a);
void AB_Account_SetDbId(AB_ACCOUNT *a, const char *s);


#endif /* AQBANKING_ACCOUNT_L_H */
