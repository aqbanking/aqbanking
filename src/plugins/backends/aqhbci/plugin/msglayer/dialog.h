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

#define AH_DIALOG_FLAGS_INITIATOR     0x0001
#define AH_DIALOG_FLAGS_OPEN          0x0002
#define AH_DIALOG_FLAGS_AUTHENTICATED 0x0004
#define AH_DIALOG_FLAGS_SECURED       0x0008
#define AH_DIALOG_FLAGS_ANONYMOUS     0x0010
#define AH_DIALOG_FLAGS_HAVEKEYS      0x0020

#ifdef __cplusplus
extern "C" {
#endif
typedef struct AH_DIALOG AH_DIALOG;
#ifdef __cplusplus
}
#endif

#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <aqhbci/customer.h>
#include <aqhbci/bank.h>
#include <aqhbci/message.h>
#include <gwenhywfar/netlayer.h>
#include <gwenhywfar/msgengine.h>
#include <aqbanking/banking.h>


#ifdef __cplusplus
extern "C" {
#endif

GWEN_LIST_FUNCTION_DEFS(AH_DIALOG, AH_Dialog);
GWEN_INHERIT_FUNCTION_DEFS(AH_DIALOG);


AQHBCI_API
AH_DIALOG *AH_Dialog_new(AH_CUSTOMER *owner);
AQHBCI_API
void AH_Dialog_free(AH_DIALOG *dlg);

AQHBCI_API
void AH_Dialog_Attach(AH_DIALOG *dlg);

AQHBCI_API
GWEN_TYPE_UINT32 AH_Dialog_GetFlags(const AH_DIALOG *dlg);
AQHBCI_API
void AH_Dialog_SetFlags(AH_DIALOG *dlg, GWEN_TYPE_UINT32 f);
AQHBCI_API
void AH_Dialog_AddFlags(AH_DIALOG *dlg, GWEN_TYPE_UINT32 f);
AQHBCI_API
void AH_Dialog_SubFlags(AH_DIALOG *dlg, GWEN_TYPE_UINT32 f);

AQHBCI_API
const char *AH_Dialog_GetLogFile(const AH_DIALOG *dlg);

AQHBCI_API
AH_BANK *AH_Dialog_GetBank(const AH_DIALOG *dlg);

AQHBCI_API
AH_HBCI *AH_Dialog_GetHbci(const AH_DIALOG *dlg);
AQHBCI_API
AB_BANKING *AH_Dialog_GetBankingApi(const AH_DIALOG *dlg);

AQHBCI_API
GWEN_TYPE_UINT32 AH_Dialog_GetNextMsgNum(AH_DIALOG *dlg);
AQHBCI_API
GWEN_TYPE_UINT32 AH_Dialog_GetLastMsgNum(const AH_DIALOG *dlg);
AQHBCI_API
GWEN_TYPE_UINT32
  AH_Dialog_GetLastReceivedMsgNum(const AH_DIALOG *dlg);

AQHBCI_API
const char *AH_Dialog_GetDialogId(const AH_DIALOG *dlg);
AQHBCI_API
void AH_Dialog_SetDialogId(AH_DIALOG *dlg,
                                const char *s);

AQHBCI_API
AH_CUSTOMER *AH_Dialog_GetDialogOwner(const AH_DIALOG *dlg);

AQHBCI_API
GWEN_NETLAYER *AH_Dialog_GetNetLayer(const AH_DIALOG *dlg);
AQHBCI_API
GWEN_MSGENGINE *AH_Dialog_GetMsgEngine(const AH_DIALOG *dlg);
AQHBCI_API
void AH_Dialog_SetMsgEngine(AH_DIALOG *dlg, GWEN_MSGENGINE *e);

AQHBCI_API
GWEN_DB_NODE *AH_Dialog_GetGlobalValues(const AH_DIALOG *dlg);

AQHBCI_API
int AH_Dialog_CheckReceivedMsgNum(AH_DIALOG *dlg, GWEN_TYPE_UINT32 msgnum);

AQHBCI_API
int AH_Dialog_Connect(AH_DIALOG *dlg, int timeout);
AQHBCI_API
int AH_Dialog_Disconnect(AH_DIALOG *dlg, int timeout);

AQHBCI_API
AH_MSG *AH_Dialog_RecvMessage(AH_DIALOG *dlg);
AQHBCI_API
AH_MSG *AH_Dialog_RecvMessage_Wait(AH_DIALOG *dlg, int timeout);

AQHBCI_API
int AH_Dialog_SendMessage(AH_DIALOG *dlg, AH_MSG *msg);
AQHBCI_API
int AH_Dialog_SendMessage_Wait(AH_DIALOG *dlg, AH_MSG *msg, int timeout);


#ifdef __cplusplus
}
#endif






#endif /* GWHBCI_DIALOG_H */
