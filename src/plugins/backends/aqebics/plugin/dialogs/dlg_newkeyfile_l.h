/***************************************************************************
 begin       : Sat Jun 26 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQEBICS_DLG_NEWKEYFILE_H
#define AQEBICS_DLG_NEWKEYFILE_H


#include <aqebics/aqebics.h>
#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *EBC_NewKeyFileDialog_new(AB_BANKING *ab);

const char *EBC_NewKeyFileDialog_GetFileName(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetFileName(GWEN_DIALOG *dlg, const char *s);


const char *EBC_NewKeyFileDialog_GetBankCode(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetBankCode(GWEN_DIALOG *dlg, const char *s);

const char *EBC_NewKeyFileDialog_GetBankName(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetBankName(GWEN_DIALOG *dlg, const char *s);

const char *EBC_NewKeyFileDialog_GetUserName(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetUserName(GWEN_DIALOG *dlg, const char *s);

const char *EBC_NewKeyFileDialog_GetUserId(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetUserId(GWEN_DIALOG *dlg, const char *s);

const char *EBC_NewKeyFileDialog_GetCustomerId(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetCustomerId(GWEN_DIALOG *dlg, const char *s);

const char *EBC_NewKeyFileDialog_GetUrl(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetUrl(GWEN_DIALOG *dlg, const char *s);

const char *EBC_NewKeyFileDialog_GetEbicsVersion(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetEbicsVersion(GWEN_DIALOG *dlg, const char *s);

const char *EBC_NewKeyFileDialog_GetSignVersion(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetSignVersion(GWEN_DIALOG *dlg, const char *s);

const char *EBC_NewKeyFileDialog_GetCryptVersion(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetCryptVersion(GWEN_DIALOG *dlg, const char *s);

const char *EBC_NewKeyFileDialog_GetAuthVersion(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetAuthVersion(GWEN_DIALOG *dlg, const char *s);

const char *EBC_NewKeyFileDialog_GetHostId(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetHostId(GWEN_DIALOG *dlg, const char *s);

uint32_t EBC_NewKeyFileDialog_GetFlags(const GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl);
void EBC_NewKeyFileDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl);
void EBC_NewKeyFileDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl);

AB_USER *EBC_NewKeyFileDialog_GetUser(const GWEN_DIALOG *dlg);


#ifdef __cplusplus
}
#endif



#endif

