/***************************************************************************
 begin       : Tue Apr 20 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_ZKACARD_H
#define AQHBCI_DLG_ZKACARD_H


#include <aqhbci/aqhbci.h>

#include <aqbanking/banking.h>
#include <aqbanking/backendsupport/user.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/ct.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *AH_ZkaCardDialog_new(AB_PROVIDER *pro, GWEN_CRYPT_TOKEN *ct);

GWEN_CRYPT_TOKEN *AH_ZkaCardDialog_GetCryptToken(const GWEN_DIALOG *dlg);

const char *AH_ZkaCardDialog_GetPeerId(const GWEN_DIALOG *dlg);
void AH_ZkaCardDialog_SetPeerId(GWEN_DIALOG *dlg, const char *s);

const char *AH_ZkaCardDialog_GetBankCode(const GWEN_DIALOG *dlg);
void AH_ZkaCardDialog_SetBankCode(GWEN_DIALOG *dlg, const char *s);

const char *AH_ZkaCardDialog_GetBankName(const GWEN_DIALOG *dlg);
void AH_ZkaCardDialog_SetBankName(GWEN_DIALOG *dlg, const char *s);

const char *AH_ZkaCardDialog_GetUserName(const GWEN_DIALOG *dlg);
void AH_ZkaCardDialog_SetUserName(GWEN_DIALOG *dlg, const char *s);

const char *AH_ZkaCardDialog_GetUserId(const GWEN_DIALOG *dlg);
void AH_ZkaCardDialog_SetUserId(GWEN_DIALOG *dlg, const char *s);

const char *AH_ZkaCardDialog_GetCustomerId(const GWEN_DIALOG *dlg);
void AH_ZkaCardDialog_SetCustomerId(GWEN_DIALOG *dlg, const char *s);

const char *AH_ZkaCardDialog_GetUrl(const GWEN_DIALOG *dlg);
void AH_ZkaCardDialog_SetUrl(GWEN_DIALOG *dlg, const char *s);

int AH_ZkaCardDialog_GetHbciVersion(const GWEN_DIALOG *dlg);
void AH_ZkaCardDialog_SetHbciVersion(GWEN_DIALOG *dlg, int i);

uint32_t AH_ZkaCardDialog_GetFlags(const GWEN_DIALOG *dlg);
void AH_ZkaCardDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AH_ZkaCardDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AH_ZkaCardDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl);

AB_USER *AH_ZkaCardDialog_GetUser(const GWEN_DIALOG *dlg);


#ifdef __cplusplus
}
#endif



#endif

