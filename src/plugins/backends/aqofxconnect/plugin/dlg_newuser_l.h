/***************************************************************************
 begin       : Thu Aug 19 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQOFXCONNECT_DLG_NEWUSER_L_H
#define AQOFXCONNECT_DLG_NEWUSER_L_H


#include <aqofxconnect/aqofxconnect.h>
#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *AO_NewUserDialog_new(AB_BANKING *ab);

const char *AO_NewUserDialog_GetUserName(const GWEN_DIALOG *dlg);
void AO_NewUserDialog_SetUserName(GWEN_DIALOG *dlg, const char *s);

const char *AO_NewUserDialog_GetUserId(const GWEN_DIALOG *dlg);
void AO_NewUserDialog_SetUserId(GWEN_DIALOG *dlg, const char *s);

const char *AO_NewUserDialog_GetClientUid(const GWEN_DIALOG *dlg);
void AO_NewUserDialog_SetClientUid(GWEN_DIALOG *dlg, const char *s);

const char *AO_NewUserDialog_GetFid(const GWEN_DIALOG *dlg);
void AO_NewUserDialog_SetFid(GWEN_DIALOG *dlg, const char *s);

const char *AO_NewUserDialog_GetOrg(const GWEN_DIALOG *dlg);
void AO_NewUserDialog_SetOrg(GWEN_DIALOG *dlg, const char *s);

const char *AO_NewUserDialog_GetAppId(const GWEN_DIALOG *dlg);
void AO_NewUserDialog_SetAppId(GWEN_DIALOG *dlg, const char *s);

const char *AO_NewUserDialog_GetAppVer(const GWEN_DIALOG *dlg);
void AO_NewUserDialog_SetAppVer(GWEN_DIALOG *dlg, const char *s);

const char *AO_NewUserDialog_GetHeaderVer(const GWEN_DIALOG *dlg);
void AO_NewUserDialog_SetHeaderVer(GWEN_DIALOG *dlg, const char *s);

const char *AO_NewUserDialog_GetBrokerId(const GWEN_DIALOG *dlg);
void AO_NewUserDialog_SetBrokerId(GWEN_DIALOG *dlg, const char *s);

const char *AO_NewUserDialog_GetUrl(const GWEN_DIALOG *dlg);
void AO_NewUserDialog_SetUrl(GWEN_DIALOG *dlg, const char *s);

int AO_NewUserDialog_GetHttpVMajor(const GWEN_DIALOG *dlg);
int AO_NewUserDialog_GetHttpVMinor(const GWEN_DIALOG *dlg);
void AO_NewUserDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor);

uint32_t AO_NewUserDialog_GetFlags(const GWEN_DIALOG *dlg);
void AO_NewUserDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AO_NewUserDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AO_NewUserDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl);

AB_USER *AO_NewUserDialog_GetUser(const GWEN_DIALOG *dlg);


#ifdef __cplusplus
}
#endif



#endif

