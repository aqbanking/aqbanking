/***************************************************************************
 begin       : Mon Apr 12 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQPAYPAL_DLG_PINTAN_H
#define AQPAYPAL_DLG_PINTAN_H


#include <aqpaypal/aqpaypal.h>
#include <aqbanking/banking.h>
#include <aqbanking/user.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *APY_NewUserDialog_new(AB_PROVIDER *pro);

const char *APY_NewUserDialog_GetUserName(const GWEN_DIALOG *dlg);
void APY_NewUserDialog_SetUserName(GWEN_DIALOG *dlg, const char *s);

const char *APY_NewUserDialog_GetUserId(const GWEN_DIALOG *dlg);
void APY_NewUserDialog_SetUserId(GWEN_DIALOG *dlg, const char *s);

const char *APY_NewUserDialog_GetApiUserId(const GWEN_DIALOG *dlg);
void APY_NewUserDialog_SetApiUserId(GWEN_DIALOG *dlg, const char *s);

const char *APY_NewUserDialog_GetApiPassword(const GWEN_DIALOG *dlg);
void APY_NewUserDialog_SetApiPassword(GWEN_DIALOG *dlg, const char *s);

const char *APY_NewUserDialog_GetApiSignature(const GWEN_DIALOG *dlg);
void APY_NewUserDialog_SetApiSignature(GWEN_DIALOG *dlg, const char *s);

const char *APY_NewUserDialog_GetUrl(const GWEN_DIALOG *dlg);
void APY_NewUserDialog_SetUrl(GWEN_DIALOG *dlg, const char *s);

int APY_NewUserDialog_GetHttpVMajor(const GWEN_DIALOG *dlg);
int APY_NewUserDialog_GetHttpVMinor(const GWEN_DIALOG *dlg);
void APY_NewUserDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor);

uint32_t APY_NewUserDialog_GetFlags(const GWEN_DIALOG *dlg);
void APY_NewUserDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl);
void APY_NewUserDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl);
void APY_NewUserDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl);

AB_USER *APY_NewUserDialog_GetUser(const GWEN_DIALOG *dlg);


#ifdef __cplusplus
}
#endif



#endif

