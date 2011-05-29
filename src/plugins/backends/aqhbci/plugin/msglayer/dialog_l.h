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


#ifndef GWHBCI_DIALOG_L_H
#define GWHBCI_DIALOG_L_H

typedef struct AH_DIALOG AH_DIALOG;

#define AH_DIALOG_FLAGS_INITIATOR     0x0001
#define AH_DIALOG_FLAGS_OPEN          0x0002
#define AH_DIALOG_FLAGS_AUTHENTICATED 0x0004
#define AH_DIALOG_FLAGS_SECURED       0x0008
#define AH_DIALOG_FLAGS_ANONYMOUS     0x0010
#define AH_DIALOG_FLAGS_HAVEKEYS      0x0020

#include "hbci_l.h"
#include "message_l.h"
#include "tanmethod.h"

#include <aqhbci/aqhbci.h>

#include <aqbanking/banking.h>
#include <aqbanking/user.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/syncio.h>
#include <gwenhywfar/msgengine.h>



AH_DIALOG *AH_Dialog_new(AB_USER *owner);
void AH_Dialog_free(AH_DIALOG *dlg);

void AH_Dialog_Attach(AH_DIALOG *dlg);

AB_BANKING *AH_Dialog_GetBankingApi(const AH_DIALOG *dlg);

int AH_Dialog_Connect(AH_DIALOG *dlg);

int AH_Dialog_Disconnect(AH_DIALOG *dlg);


uint32_t AH_Dialog_GetFlags(const AH_DIALOG *dlg);
void AH_Dialog_SetFlags(AH_DIALOG *dlg, uint32_t f);
void AH_Dialog_AddFlags(AH_DIALOG *dlg, uint32_t f);
void AH_Dialog_SubFlags(AH_DIALOG *dlg, uint32_t f);
const char *AH_Dialog_GetLogFile(const AH_DIALOG *dlg);

uint32_t AH_Dialog_GetNextMsgNum(AH_DIALOG *dlg);
uint32_t AH_Dialog_GetLastMsgNum(const AH_DIALOG *dlg);
uint32_t AH_Dialog_GetLastReceivedMsgNum(const AH_DIALOG *dlg);

const char *AH_Dialog_GetDialogId(const AH_DIALOG *dlg);
void AH_Dialog_SetDialogId(AH_DIALOG *dlg, const char *s);

AB_USER *AH_Dialog_GetDialogOwner(const AH_DIALOG *dlg);

GWEN_MSGENGINE *AH_Dialog_GetMsgEngine(const AH_DIALOG *dlg);
void AH_Dialog_SetMsgEngine(AH_DIALOG *dlg, GWEN_MSGENGINE *e);

GWEN_DB_NODE *AH_Dialog_GetGlobalValues(const AH_DIALOG *dlg);

int AH_Dialog_CheckReceivedMsgNum(AH_DIALOG *dlg, uint32_t msgnum);
int AH_Dialog_RecvMessage(AH_DIALOG *dlg, AH_MSG **pMsg);
int AH_Dialog_SendMessage(AH_DIALOG *dlg, AH_MSG *msg);


AH_HBCI *AH_Dialog_GetHbci(const AH_DIALOG *dlg);

void AH_Dialog_SetItanMethod(AH_DIALOG *dlg, uint32_t i);
uint32_t AH_Dialog_GetItanMethod(const AH_DIALOG *dlg);

int AH_Dialog_GetItanProcessType(const AH_DIALOG *dlg);
void AH_Dialog_SetItanProcessType(AH_DIALOG *dlg, int i);

int AH_Dialog_GetTanJobVersion(const AH_DIALOG *dlg);
void AH_Dialog_SetTanJobVersion(AH_DIALOG *dlg, int i);


const AH_TAN_METHOD *AH_Dialog_GetTanMethodDescription(const AH_DIALOG *dlg);
void AH_Dialog_SetTanMethodDescription(AH_DIALOG *dlg, const AH_TAN_METHOD *tm);


int AH_Dialog_TestServer_Https(AH_DIALOG *dlg);


#endif /* GWHBCI_DIALOG_L_H */
