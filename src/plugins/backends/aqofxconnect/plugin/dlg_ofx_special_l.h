/***************************************************************************
 begin       : Thu Aug 19 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQOFXCONNECT_DLG_OFX_SPECIAL_L_H
#define AQOFXCONNECT_DLG_OFX_SPECIAL_L_H


#include <aqofxconnect/aqofxconnect.h>
#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *AO_OfxSpecialDialog_new(AB_BANKING *ab);

int AO_OfxSpecialDialog_GetHttpVMajor(const GWEN_DIALOG *dlg);
int AO_OfxSpecialDialog_GetHttpVMinor(const GWEN_DIALOG *dlg);
void AO_OfxSpecialDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor);

uint32_t AO_OfxSpecialDialog_GetFlags(const GWEN_DIALOG *dlg);
void AO_OfxSpecialDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AO_OfxSpecialDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AO_OfxSpecialDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl);

void AO_OfxSpecialDialog_SetClientUid(GWEN_DIALOG *dlg, const char *s);
const char *AO_OfxSpecialDialog_GetClientUid(const GWEN_DIALOG *dlg);

void AO_OfxSpecialDialog_SetSecurityType(GWEN_DIALOG *dlg, const char *s);
const char *AO_OfxSpecialDialog_GetSecurityType(const GWEN_DIALOG *dlg);



#ifdef __cplusplus
}
#endif



#endif

