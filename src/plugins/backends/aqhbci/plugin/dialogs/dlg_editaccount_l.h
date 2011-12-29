/***************************************************************************
 begin       : Thu Apr 15 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_EDITACCOUNT_DIALOG_L_H
#define AQHBCI_EDITACCOUNT_DIALOG_L_H


#include <aqhbci/aqhbci.h>
#include <aqbanking/banking.h>
#include <aqbanking/account.h>

#include <gwenhywfar/dialog.h>


#ifdef __cplusplus
extern "C" {
#endif



GWEN_DIALOG *AH_EditAccountDialog_new(AB_BANKING *ab, AB_ACCOUNT *a, int doLock);




#ifdef __cplusplus
}
#endif



#endif

