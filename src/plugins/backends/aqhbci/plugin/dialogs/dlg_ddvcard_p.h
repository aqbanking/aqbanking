/***************************************************************************
 begin       : Tue Apr 20 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_DLG_DDVCARD_P_H
#define AQHBCI_DLG_DDVCARD_P_H


#include "dlg_ddvcard_l.h"

#include <gwenhywfar/ct_context.h>


typedef struct AH_DDVCARD_DIALOG AH_DDVCARD_DIALOG;
struct AH_DDVCARD_DIALOG {
  AB_BANKING *banking;

  char *bankCode;
  char *bankName;

  char *userName;
  char *userId;
  char *customerId;
  char *url;

  char *peerId;

  int hbciVersion;

  uint32_t flags;

  GWEN_CRYPT_TOKEN *cryptToken;
  GWEN_CRYPT_TOKEN_CONTEXT_LIST *contextList;
  AB_USER *user;
};


static void GWENHYWFAR_CB AH_DdvCardDialog_FreeData(void *bp, void *p);

static int GWENHYWFAR_CB AH_DdvCardDialog_SignalHandler(GWEN_DIALOG *dlg,
							GWEN_DIALOG_EVENTTYPE t,
							const char *sender);


static int AH_DdvCardDialog_FromContext(GWEN_DIALOG *dlg, int i);



#endif

