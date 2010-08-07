/***************************************************************************
 begin       : Sat Jun 26 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_RDH_SPECIAL_H
#define AQHBCI_DLG_RDH_SPECIAL_H


#include <aqhbci/aqhbci.h>
#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *AH_RdhSpecialDialog_new(AB_BANKING *ab);

int AH_RdhSpecialDialog_GetHbciVersion(const GWEN_DIALOG *dlg);
void AH_RdhSpecialDialog_SetHbciVersion(GWEN_DIALOG *dlg, int i);

int AH_RdhSpecialDialog_GetRdhVersion(const GWEN_DIALOG *dlg);
void AH_RdhSpecialDialog_SetRdhVersion(GWEN_DIALOG *dlg, int i);

uint32_t AH_RdhSpecialDialog_GetFlags(const GWEN_DIALOG *dlg);
void AH_RdhSpecialDialog_SetFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AH_RdhSpecialDialog_AddFlags(GWEN_DIALOG *dlg, uint32_t fl);
void AH_RdhSpecialDialog_SubFlags(GWEN_DIALOG *dlg, uint32_t fl);



#ifdef __cplusplus
}
#endif



#endif

