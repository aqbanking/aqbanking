/***************************************************************************
 begin       : Thu Jul 08 2010
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQEBICS_DLG_EDITUSER_H
#define AQEBICS_DLG_EDITUSER_H

#include <aqebics/aqebics.h>

#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>

#include <gwenhywfar/dialog.h>


#ifdef __cplusplus
extern "C" {
#endif


GWEN_DIALOG *EBC_EditUserDialog_new(AB_PROVIDER *pro, AB_USER *u, int doLock);



#ifdef __cplusplus
}
#endif





#endif

