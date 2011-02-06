/***************************************************************************
 begin       : Mon Apr 12 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_PINTAN_SPECIAL_H
#define AQHBCI_DLG_PINTAN_SPECIAL_H


#include <aqhbci/aqhbci.h>
#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *AH_PinTanSpecialDialog_new(AB_BANKING *ab);

int AH_PinTanSpecialDialog_GetHttpVMajor(const GWEN_DIALOG *dlg);
int AH_PinTanSpecialDialog_GetHttpVMinor(const GWEN_DIALOG *dlg);
void AH_PinTanSpecialDialog_SetHttpVersion(GWEN_DIALOG *dlg, int vmajor, int vminor);

int AH_PinTanSpecialDialog_GetHbciVersion(const GWEN_DIALOG *dlg);
void AH_PinTanSpecialDialog_SetHbciVersion(GWEN_DIALOG *dlg, int i);

uint32_t AH_PinTanSpecialDialog_GetFlags(const GWEN_DIALOG *dlg);
void AH_PinTanSpecialDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AH_PinTanSpecialDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AH_PinTanSpecialDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl);

const char *AH_PinTanSpecialDialog_GetTanMediumId(const GWEN_DIALOG *dlg);
void AH_PinTanSpecialDialog_SetTanMediumId(GWEN_DIALOG *dlg, const char *s);


#ifdef __cplusplus
}
#endif



#endif

