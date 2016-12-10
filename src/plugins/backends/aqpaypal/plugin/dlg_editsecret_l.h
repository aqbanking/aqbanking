/***************************************************************************
 begin       : Thu Aug 19 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQPAYPAL_DLG_OFX_SPECIAL_L_H
#define AQPAYPAL_DLG_OFX_SPECIAL_L_H


#include <aqpaypal/aqpaypal.h>
#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *APY_EditSecretDialog_new(AB_BANKING *ab);

void APY_EditSecretDialog_SetApiUserId(GWEN_DIALOG *dlg, const char *s);
char *APY_EditSecretDialog_GetApiUserId(const GWEN_DIALOG *dlg);

void APY_EditSecretDialog_SetApiPassword(GWEN_DIALOG *dlg, const char *s);
char *APY_EditSecretDialog_GetApiPassword(const GWEN_DIALOG *dlg);

void APY_EditSecretDialog_SetApiSignature(GWEN_DIALOG *dlg, const char *s);
char *APY_EditSecretDialog_GetApiSignature(const GWEN_DIALOG *dlg);



#ifdef __cplusplus
}
#endif



#endif

