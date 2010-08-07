/***************************************************************************
 begin       : Thu Jul 08 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_EDITUSER_RDH_H
#define AQHBCI_DLG_EDITUSER_RDH_H

#include <aqhbci/aqhbci.h>

#include <aqbanking/banking.h>
#include <aqbanking/user.h>

#include <gwenhywfar/dialog.h>


#ifdef __cplusplus
extern "C" {
#endif


GWEN_DIALOG *AH_EditUserRdhDialog_new(AB_BANKING *ab, AB_USER *u, int doLock);



#ifdef __cplusplus
}
#endif





#endif

