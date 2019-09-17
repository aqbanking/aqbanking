/***************************************************************************
 begin       : Tue Sep 17 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_PINTAN_TANMODE_L_H
#define AQHBCI_DLG_PINTAN_TANMODE_L_H


#include <aqhbci/aqhbci.h>

#include <aqbanking/banking.h>
#include <aqbanking/backendsupport/provider_be.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *AH_PinTan_TanModeDialog_new(AB_PROVIDER *pro, AB_USER *u, int doLock);




#ifdef __cplusplus
}
#endif



#endif

