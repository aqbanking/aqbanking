/***************************************************************************
 begin       : Sat Aug 07 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_IMPORTKEYFILE_P_H
#define AQHBCI_DLG_IMPORTKEYFILE_P_H


#include "dlg_importkeyfile_l.h"

#include <gwenhywfar/ct_context.h>


typedef struct AH_IMPORTKEYFILE_DIALOG AH_IMPORTKEYFILE_DIALOG;
struct AH_IMPORTKEYFILE_DIALOG {
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

  GWEN_CRYPT_TOKEN_CONTEXT_LIST *contextList;

  AB_USER *user;
};


static void GWENHYWFAR_CB AH_ImportKeyFileDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AH_ImportKeyFileDialog_SignalHandler(GWEN_DIALOG *dlg,
							      GWEN_DIALOG_EVENTTYPE t,
							      const char *sender);

static int AH_ImportKeyFileDialog_GetFilePageData(GWEN_DIALOG *dlg);
static int AH_ImportKeyFileDialog_CheckFileType(GWEN_DIALOG *dlg);

static int AH_ImportKeyFileDialog_HandleActivatedContext(GWEN_DIALOG *dlg);




#endif

