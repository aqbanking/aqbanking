/***************************************************************************
 begin       : Mon Apr 19 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_NEWUSER_DIALOG_H
#define AQHBCI_NEWUSER_DIALOG_H


#include "aqhbci/aqhbci.h"

#include "aqbanking/backendsupport/provider.h"

#include <gwenhywfar/dialog.h>


#ifdef __cplusplus
extern "C" {
#endif


GWEN_DIALOG *AH_NewUserDialog_new(AB_PROVIDER *pro);


#ifdef __cplusplus
}
#endif




#endif

