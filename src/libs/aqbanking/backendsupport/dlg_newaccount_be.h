/***************************************************************************
 begin       : Wed Apr 14 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AB_DLG_NEW_ACCOUNT_BE_H
#define AB_DLG_NEW_ACCOUNT_BE_H


#include <aqbanking/banking.h>
#include <aqbanking/account.h>

#include <gwenhywfar/dialog.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *AB_NewAccountDialog_new(AB_BANKING *ab, const char *dname);

AB_ACCOUNT *AB_NewAccountDialog_GetAccount(const GWEN_DIALOG *dlg);
void AB_NewAccountDialog_SetAccount(GWEN_DIALOG *dlg, AB_ACCOUNT *a);


#ifdef __cplusplus
}
#endif



#endif

