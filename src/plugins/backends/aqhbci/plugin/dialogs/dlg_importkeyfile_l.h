/***************************************************************************
 begin       : Sat Aug 07 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_IMPORTKEYFILE_H
#define AQHBCI_DLG_IMPORTKEYFILE_H


#include <aqhbci/aqhbci.h>
#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *AH_ImportKeyFileDialog_new(AB_BANKING *ab);

const char *AH_ImportKeyFileDialog_GetFileName(const GWEN_DIALOG *dlg);
void AH_ImportKeyFileDialog_SetFileName(GWEN_DIALOG *dlg, const char *s);


const char *AH_ImportKeyFileDialog_GetBankCode(const GWEN_DIALOG *dlg);
void AH_ImportKeyFileDialog_SetBankCode(GWEN_DIALOG *dlg, const char *s);

const char *AH_ImportKeyFileDialog_GetBankName(const GWEN_DIALOG *dlg);
void AH_ImportKeyFileDialog_SetBankName(GWEN_DIALOG *dlg, const char *s);

const char *AH_ImportKeyFileDialog_GetUserName(const GWEN_DIALOG *dlg);
void AH_ImportKeyFileDialog_SetUserName(GWEN_DIALOG *dlg, const char *s);

const char *AH_ImportKeyFileDialog_GetUserId(const GWEN_DIALOG *dlg);
void AH_ImportKeyFileDialog_SetUserId(GWEN_DIALOG *dlg, const char *s);

const char *AH_ImportKeyFileDialog_GetCustomerId(const GWEN_DIALOG *dlg);
void AH_ImportKeyFileDialog_SetCustomerId(GWEN_DIALOG *dlg, const char *s);

const char *AH_ImportKeyFileDialog_GetUrl(const GWEN_DIALOG *dlg);
void AH_ImportKeyFileDialog_SetUrl(GWEN_DIALOG *dlg, const char *s);

int AH_ImportKeyFileDialog_GetHbciVersion(const GWEN_DIALOG *dlg);
void AH_ImportKeyFileDialog_SetHbciVersion(GWEN_DIALOG *dlg, int i);

int AH_ImportKeyFileDialog_GetRdhVersion(const GWEN_DIALOG *dlg);
void AH_ImportKeyFileDialog_SetRdhVersion(GWEN_DIALOG *dlg, int i);

uint32_t AH_ImportKeyFileDialog_GetFlags(const GWEN_DIALOG *dlg);
void AH_ImportKeyFileDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AH_ImportKeyFileDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AH_ImportKeyFileDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl);

AB_USER *AH_ImportKeyFileDialog_GetUser(const GWEN_DIALOG *dlg);


#ifdef __cplusplus
}
#endif



#endif

