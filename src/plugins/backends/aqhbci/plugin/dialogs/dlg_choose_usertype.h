/***************************************************************************
 begin       : Fri Jul 30 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_CHOOSEUSERTYPE_DIALOG_H
#define AQHBCI_CHOOSEUSERTYPE_DIALOG_H


#include <gwenhywfar/dialog.h>
#include <aqbanking/banking.h>
#include <aqhbci/aqhbci.h>


#ifdef __cplusplus
extern "C" {
#endif


AQHBCI_API GWEN_DIALOG *AH_ChooseUserTypeDialog_new(AB_BANKING *ab);


#ifdef __cplusplus
}
#endif




#endif

