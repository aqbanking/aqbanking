/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWHBCI_DIALOG_H
#define GWHBCI_DIALOG_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct AH_DIALOG AH_DIALOG;
#ifdef __cplusplus
}
#endif

#include <aqhbci/message.h>
#include <aqhbci/aqhbci.h>
#include <aqhbci/hbci.h>

#include <aqbanking/banking.h>
#include <aqbanking/user.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/netlayer.h>
#include <gwenhywfar/msgengine.h>


#ifdef __cplusplus
extern "C" {
#endif


AQHBCI_API
AH_DIALOG *AH_Dialog_new(AB_USER *owner);

AQHBCI_API
void AH_Dialog_free(AH_DIALOG *dlg);

AQHBCI_API
void AH_Dialog_Attach(AH_DIALOG *dlg);

AQHBCI_API
AH_HBCI *AH_Dialog_GetHbci(const AH_DIALOG *dlg);

AQHBCI_API
AB_BANKING *AH_Dialog_GetBankingApi(const AH_DIALOG *dlg);


AQHBCI_API
int AH_Dialog_Connect(AH_DIALOG *dlg, int timeout);

AQHBCI_API
int AH_Dialog_Disconnect(AH_DIALOG *dlg, int timeout);


#ifdef __cplusplus
}
#endif






#endif /* GWHBCI_DIALOG_H */
