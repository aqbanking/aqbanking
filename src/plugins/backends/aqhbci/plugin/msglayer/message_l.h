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

#ifndef AH_MESSAGE_L_H
#define AH_MESSAGE_L_H

#include <gwenhywfar/misc.h>

typedef struct AH_MSG AH_MSG;
GWEN_LIST_FUNCTION_DEFS(AH_MSG, AH_Msg);

#include "dialog_l.h"

#include <gwenhywfar/xml.h>
#include <stdio.h>


AH_MSG *AH_Msg_new(AH_DIALOG *dlg);
void AH_Msg_free(AH_MSG *hmsg);

const GWEN_STRINGLIST *AH_Msg_GetSignerIdList(const AH_MSG *hmsg);
unsigned int AH_Msg_GetSignerCount(AH_MSG *hmsg);
int AH_Msg_AddSignerId(AH_MSG *hmsg, const char *s);
int AH_Msg_IsSignedBy(const AH_MSG *hmsg, const char *s);
const char *AH_Msg_GetCrypterId(const AH_MSG *hmsg);
void AH_Msg_SetCrypterId(AH_MSG *hmsg, const char *s);


GWEN_BUFFER *AH_Msg_GetBuffer(AH_MSG *hmsg);
GWEN_BUFFER *AH_Msg_TakeBuffer(AH_MSG *hmsg);
void AH_Msg_SetBuffer(AH_MSG *hmsg, GWEN_BUFFER *bf);

unsigned int AH_Msg_GetMsgNum(const AH_MSG *hmsg);

unsigned int AH_Msg_GetMsgRef(const AH_MSG *hmsg);
void AH_Msg_SetMsgRef(AH_MSG *hmsg,
                           unsigned int i);
unsigned int AH_Msg_GetNodes(AH_MSG *hmsg);

AH_DIALOG *AH_Msg_GetDialog(const AH_MSG *hmsg);

int AH_Msg_EnableInsert(AH_MSG *hmsg);
int AH_Msg_HasWarnings(const AH_MSG *hmsg);
void AH_Msg_SetHasWarnings(AH_MSG *hmsg, int i);
int AH_Msg_HasErrors(const AH_MSG *hmsg);
void AH_Msg_SetHasErrors(AH_MSG *hmsg, int i);

int AH_Msg_GetResultCode(const AH_MSG *hmsg);
void AH_Msg_SetResultCode(AH_MSG *hmsg, int i);

const char *AH_Msg_GetResultText(const AH_MSG *hmsg);
void AH_Msg_SetResultText(AH_MSG *hmsg, const char *s);

const char *AH_Msg_GetResultParam(const AH_MSG *hmsg);
void AH_Msg_SetResultParam(AH_MSG *hmsg, const char *s);

unsigned int AH_Msg_GetHbciVersion(const AH_MSG *hmsg);
void AH_Msg_SetHbciVersion(AH_MSG *hmsg, unsigned int i);

int AH_Msg_GetSecurityProfile(const AH_MSG *hmsg);
void AH_Msg_SetSecurityProfile(AH_MSG *hmsg, int i);

int AH_Msg_GetSecurityClass(const AH_MSG *hmsg);
void AH_Msg_SetSecurityClass(AH_MSG *hmsg, int i);

const char *AH_Msg_GetTan(const AH_MSG *hmsg);
void AH_Msg_SetTan(AH_MSG *hmsg, const char *s);

int AH_Msg_GetNeedTan(const AH_MSG *hmsg);
void AH_Msg_SetNeedTan(AH_MSG *hmsg, int i);

int AH_Msg_NoSysId(const AH_MSG *hmsg);
void AH_Msg_SetNoSysId(AH_MSG *hmsg, int i);

unsigned int AH_Msg_GetCurrentSegmentNumber(AH_MSG *hmsg);

unsigned int AH_Msg_AddNode(AH_MSG *hmsg,
                            GWEN_XMLNODE *node,
                            GWEN_DB_NODE *data);

void AH_Msg_SetItanMethod(AH_MSG *hmsg, uint32_t i);
uint32_t AH_Msg_GetItanMethod(const AH_MSG *hmsg);

void AH_Msg_SetItanHashMode(AH_MSG *hmsg, int i);
int AH_Msg_GetItanHashMode(const AH_MSG *hmsg);

void AH_Msg_SetItanHashBuffer(AH_MSG *hmsg, GWEN_BUFFER *hbuf);
GWEN_BUFFER *AH_Msg_GetItanHashBuffer(const AH_MSG *hmsg);

unsigned int AH_Msg_InsertNode(AH_MSG *hmsg,
                               GWEN_XMLNODE *node,
                               GWEN_DB_NODE *data);

int AH_Msg_EncodeMsg(AH_MSG *hmsg);

int AH_Msg_DecodeMsg(AH_MSG *hmsg,
                     GWEN_DB_NODE *gr,
                     unsigned int flags);

void AH_Msg__Dump(const AH_MSG *hmsg,
                  unsigned int indent);

#define AH_Msg_Dump(hmsg, indent) \
  {fprintf(stderr, "Dumping message from "__FILE__" %d\n", __LINE__);\
  AH_Msg__Dump(hmsg, indent);}


GWEN_BUFFER *AH_Msg_GetOrigBuffer(AH_MSG *hmsg);
void AH_Msg_LogMessage(AH_MSG *msg,
                       GWEN_BUFFER *buf,
                       int rec,
                       int crypt);

const char *AH_Msg_GetExpectedSigner(const AH_MSG *msg);
void AH_Msg_SetExpectedSigner(AH_MSG *msg, const char *s);

const char *AH_Msg_GetExpectedCrypter(const AH_MSG *msg);
void AH_Msg_SetExpectedCrypter(AH_MSG *msg, const char *s);

const char *AH_Msg_GetPin(const AH_MSG *msg);

#endif /* AH_MESSAGE_L_H */



