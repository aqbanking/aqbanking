/***************************************************************************
 begin       : Wed Aug 18 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef OFXHOME_DLG_GETINST_P_H
#define OFXHOME_DLG_GETINST_P_H


#include "dlg_getinst.h"

#include <aqofxconnect/oh_institute_spec.h>
#include <aqofxconnect/ofxhome.h>



typedef struct OH_GETINST_DIALOG OH_GETINST_DIALOG;
struct OH_GETINST_DIALOG {
  OFXHOME *ofxHome;
  OH_INSTITUTE_SPEC_LIST *matchingSpecList;
  OH_INSTITUTE_DATA *selectedData;
  char *name;
};


static GWENHYWFAR_CB void OH_GetInstituteDialog_FreeData(void *bp, void *p);




static int GWENHYWFAR_CB OH_GetInstituteDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                             GWEN_DIALOG_EVENTTYPE t,
                                                             const char *sender);


static OH_INSTITUTE_SPEC *OH_GetInstituteDialog_DetermineSelectedBankInfo(GWEN_DIALOG *dlg);
static void OH_GetInstituteDialog_UpdateList(GWEN_DIALOG *dlg);


#endif



