/***************************************************************************
 begin       : Mon Apr 12 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQEBICS_DLG_PINTAN_SPECIAL_H
#define AQEBICS_DLG_PINTAN_SPECIAL_H


#include <aqebics/aqebics.h>
#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *EBC_UserSpecialDialog_new(AB_BANKING *ab);

int EBC_UserSpecialDialog_GetHttpVMajor(const GWEN_DIALOG *dlg);
int EBC_UserSpecialDialog_GetHttpVMinor(const GWEN_DIALOG *dlg);
void EBC_UserSpecialDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor);

uint32_t EBC_UserSpecialDialog_GetFlags(const GWEN_DIALOG *dlg);
void EBC_UserSpecialDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl);
void EBC_UserSpecialDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl);
void EBC_UserSpecialDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl);

const char *EBC_UserSpecialDialog_GetEbicsVersion(const GWEN_DIALOG *dlg);
void EBC_UserSpecialDialog_SetEbicsVersion(GWEN_DIALOG *dlg, const char *s);

const char *EBC_UserSpecialDialog_GetSignVersion(const GWEN_DIALOG *dlg);
void EBC_UserSpecialDialog_SetSignVersion(GWEN_DIALOG *dlg, const char *s);

const char *EBC_UserSpecialDialog_GetCryptVersion(const GWEN_DIALOG *dlg);
void EBC_UserSpecialDialog_SetCryptVersion(GWEN_DIALOG *dlg, const char *s);

const char *EBC_UserSpecialDialog_GetAuthVersion(const GWEN_DIALOG *dlg);
void EBC_UserSpecialDialog_SetAuthVersion(GWEN_DIALOG *dlg, const char *s);

int EBC_UserSpecialDialog_GetHttpVMajor(const GWEN_DIALOG *dlg);
int EBC_UserSpecialDialog_GetHttpVMinor(const GWEN_DIALOG *dlg);
void EBC_UserSpecialDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor);

int EBC_UserSpecialDialog_GetSignKeySize(const GWEN_DIALOG *dlg);
void EBC_UserSpecialDialog_SetSignKeySize(GWEN_DIALOG *dlg, int i);

int EBC_UserSpecialDialog_GetCryptAndAuthKeySize(const GWEN_DIALOG *dlg);
void EBC_UserSpecialDialog_SetCryptAndAuthKeySize(GWEN_DIALOG *dlg, int i);

#ifdef __cplusplus
}
#endif



#endif

