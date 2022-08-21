/***************************************************************************
 begin       : Mon Apr 12 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_PINTAN_H
#define AQHBCI_DLG_PINTAN_H


#include <aqhbci/aqhbci.h>

#include <aqbanking/banking.h>
#include <aqbanking/backendsupport/user.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *AH_PinTanDialog_new(AB_PROVIDER *pro);

AB_USER *AH_PinTanDialog_GetUser(const GWEN_DIALOG *dlg);

#ifdef __cplusplus
}
#endif



#endif

