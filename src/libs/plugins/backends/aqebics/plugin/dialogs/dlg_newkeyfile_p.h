/***************************************************************************
 begin       : Sat Jun 26 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQEBICS_DLG_NEWKEYFILE_P_H
#define AQEBICS_DLG_NEWKEYFILE_P_H


#include "dlg_newkeyfile_l.h"



typedef struct EBC_NEWKEYFILE_DIALOG EBC_NEWKEYFILE_DIALOG;
struct EBC_NEWKEYFILE_DIALOG {
  AB_BANKING *banking;

  char *fileName;

  char *bankCode;
  char *bankName;
  char *url;
  char *hostId;

  char *userName;
  char *userId;
  char *customerId;

  char *ebicsVersion;
  char *signVersion;
  char *cryptVersion;
  char *authVersion;

  int httpVMajor;
  int httpVMinor;

  int signKeySize;
  int cryptAndAuthKeySize;

  uint32_t flags;

  AB_USER *user;
};


static void GWENHYWFAR_CB EBC_NewKeyFileDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB EBC_NewKeyFileDialog_SignalHandler(GWEN_DIALOG *dlg,
                                                            GWEN_DIALOG_EVENTTYPE t,
                                                            const char *sender);

static int EBC_NewKeyFileDialog_GetFilePageData(GWEN_DIALOG *dlg);


void EBC_NewKeyFileDialog_SetBankPageData(GWEN_DIALOG *dlg);
void EBC_NewKeyFileDialog_SetUserPageData(GWEN_DIALOG *dlg);


#endif

