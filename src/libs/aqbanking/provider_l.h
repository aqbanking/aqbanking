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


#ifndef AQBANKING_PROVIDER_L_H
#define AQBANKING_PROVIDER_L_H

#define AB_PROVIDER_FOLDER "providers"
#define AB_PROVIDER_WIZARD_FOLDER "wizards"
#define AB_PROVIDER_DEBUGGER_FOLDER "debugger"

#include <aqbanking/provider.h>
#include <aqbanking/provider_be.h>
#include <gwenhywfar/plugin.h>


/** @name Virtual Functions
 *
 */
/*@{*/

int AB_Provider_Init(AB_PROVIDER *pro);
int AB_Provider_Fini(AB_PROVIDER *pro);
int AB_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
int AB_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
int AB_Provider_Execute(AB_PROVIDER *pro);
int AB_Provider_ResetQueue(AB_PROVIDER *pro);
AB_ACCOUNT_LIST2 *AB_Provider_GetAccountList(AB_PROVIDER *pro);
int AB_Provider_UpdateAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);
int AB_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);
/*@}*/

void AB_Provider_SetPlugin(AB_PROVIDER *pro, GWEN_PLUGIN *pl);
void AB_Provider_free(AB_PROVIDER *pro);


#endif /* AQBANKING_PROVIDER_L_H */
