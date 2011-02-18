/***************************************************************************
 begin       : Sat Jun 26 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_NEWKEYFILE_P_H
#define AQHBCI_DLG_NEWKEYFILE_P_H


#include "dlg_newkeyfile_l.h"



typedef struct AH_NEWKEYFILE_DIALOG AH_NEWKEYFILE_DIALOG;
struct AH_NEWKEYFILE_DIALOG {
  AB_BANKING *banking;

  char *fileName;

  char *bankCode;
  char *bankName;

  char *userName;
  char *userId;
  char *customerId;
  char *url;

  int hbciVersion;
  int rdhVersion;

  uint32_t flags;

  AB_USER *user;
};


static void GWENHYWFAR_CB AH_NewKeyFileDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AH_NewKeyFileDialog_SignalHandler(GWEN_DIALOG *dlg,
							   GWEN_DIALOG_EVENTTYPE t,
							   const char *sender);

static int AH_NewKeyFileDialog_GetFilePageData(GWEN_DIALOG *dlg);
static int AH_NewKeyFileDialog_CheckBankIniLetter(GWEN_DIALOG *dlg, AB_USER *u);




#endif

