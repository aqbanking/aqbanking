/***************************************************************************
 begin       : Tue Apr 13 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_DLG_SELECTBANKINFO_P_H
#define AQBANKING_DLG_SELECTBANKINFO_P_H


#include "dlg_selectbankinfo.h"



typedef struct AB_SELECTBANKINFO_DIALOG AB_SELECTBANKINFO_DIALOG;
struct AB_SELECTBANKINFO_DIALOG {
  AB_BANKING *banking;

  char *country;
  char *bankCode;
  AB_BANKINFO_LIST2 *matchingBankInfos;
  AB_BANKINFO *selectedBankInfo;
};


static GWENHYWFAR_CB void AB_SelectBankInfoDialog_FreeData(void *bp, void *p);




static int GWENHYWFAR_CB AB_SelectBankInfoDialog_SignalHandler(GWEN_DIALOG *dlg,
							       GWEN_DIALOG_EVENTTYPE t,
							       const char *sender);



#endif



