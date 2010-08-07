/***************************************************************************
 begin       : Sat Jun 26 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_NEWKEYFILE_H
#define AQHBCI_DLG_NEWKEYFILE_H


#include <aqhbci/aqhbci.h>
#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *AH_NewKeyFileDialog_new(AB_BANKING *ab);

const char *AH_NewKeyFileDialog_GetFileName(const GWEN_DIALOG *dlg);
void AH_NewKeyFileDialog_SetFileName(GWEN_DIALOG *dlg, const char *s);


const char *AH_NewKeyFileDialog_GetBankCode(const GWEN_DIALOG *dlg);
void AH_NewKeyFileDialog_SetBankCode(GWEN_DIALOG *dlg, const char *s);

const char *AH_NewKeyFileDialog_GetBankName(const GWEN_DIALOG *dlg);
void AH_NewKeyFileDialog_SetBankName(GWEN_DIALOG *dlg, const char *s);

const char *AH_NewKeyFileDialog_GetUserName(const GWEN_DIALOG *dlg);
void AH_NewKeyFileDialog_SetUserName(GWEN_DIALOG *dlg, const char *s);

const char *AH_NewKeyFileDialog_GetUserId(const GWEN_DIALOG *dlg);
void AH_NewKeyFileDialog_SetUserId(GWEN_DIALOG *dlg, const char *s);

const char *AH_NewKeyFileDialog_GetCustomerId(const GWEN_DIALOG *dlg);
void AH_NewKeyFileDialog_SetCustomerId(GWEN_DIALOG *dlg, const char *s);

const char *AH_NewKeyFileDialog_GetUrl(const GWEN_DIALOG *dlg);
void AH_NewKeyFileDialog_SetUrl(GWEN_DIALOG *dlg, const char *s);

int AH_NewKeyFileDialog_GetHbciVersion(const GWEN_DIALOG *dlg);
void AH_NewKeyFileDialog_SetHbciVersion(GWEN_DIALOG *dlg, int i);

int AH_NewKeyFileDialog_GetRdhVersion(const GWEN_DIALOG *dlg);
void AH_NewKeyFileDialog_SetRdhVersion(GWEN_DIALOG *dlg, int i);

uint32_t AH_NewKeyFileDialog_GetFlags(const GWEN_DIALOG *dlg);
void AH_NewKeyFileDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AH_NewKeyFileDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AH_NewKeyFileDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl);

AB_USER *AH_NewKeyFileDialog_GetUser(const GWEN_DIALOG *dlg);


#ifdef __cplusplus
}
#endif



#endif

