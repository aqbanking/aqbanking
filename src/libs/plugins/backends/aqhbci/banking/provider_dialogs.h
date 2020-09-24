/***************************************************************************
    begin       : Tue Jun 03 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_DIALOGS_H
#define AH_PROVIDER_DIALOGS_H

#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>
#include <aqbanking/backendsupport/account.h>


GWEN_DIALOG *AH_Provider_GetNewCardUserDialog(AB_PROVIDER *pro);
GWEN_DIALOG *AH_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u);
GWEN_DIALOG *AH_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i);
GWEN_DIALOG *AH_Provider_GetEditAccountDialog(AB_PROVIDER *pro, AB_ACCOUNT *a);
GWEN_DIALOG *AH_Provider_GetUserTypeDialog(AB_PROVIDER *pro);


#endif
