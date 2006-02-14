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

#include <aqhbci/message.h>
#include <gwenhywfar/keyspec.h>


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



