/***************************************************************************
 begin       : Wed Aug 18 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef OFXHOME_DLG_GETINST_H
#define OFXHOME_DLG_GETINST_H


#include <gwenhywfar/dialog.h>
#include <gwenhywfar/db.h>

#include <aqofxconnect/aqofxconnect.h>
#include <aqofxconnect/oh_institute_data.h>


#ifdef __cplusplus
extern "C" {
#endif


AQOFXCONNECT_API
GWEN_DIALOG *OH_GetInstituteDialog_new(const char *dataFolder, const char *name);

AQOFXCONNECT_API
const OH_INSTITUTE_DATA *OH_GetInstituteDialog_GetSelectedInstitute(GWEN_DIALOG *dlg);



/*@}*/




#ifdef __cplusplus
}
#endif



#endif



