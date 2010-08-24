/***************************************************************************
 begin       : Tue Aug 24 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQOFXCONNECT_DLG_EDITUSER_L_H
#define AQOFXCONNECT_DLG_EDITUSER_L_H

#include <aqofxconnect/aqofxconnect.h>
#include <aqbanking/banking.h>

#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>


#ifdef __cplusplus
extern "C" {
#endif


GWEN_DIALOG *AO_EditUserDialog_new(AB_BANKING *ab, AB_USER *u, int doLock);



#ifdef __cplusplus
}
#endif





#endif

