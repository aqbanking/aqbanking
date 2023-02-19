/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_MSGCRYPT_PINTAN_H
#define AH_MSGCRYPT_PINTAN_H

#include "aqhbci/msglayer/message_l.h"


int AH_MsgPinTan_PrepareCryptoSeg(AH_MSG *hmsg,
                                  AB_USER *u,
                                  GWEN_DB_NODE *cfg,
                                  int crypt,
                                  int createCtrlRef);
int AH_Msg_SignPinTan(AH_MSG *hmsg, GWEN_UNUSED GWEN_BUFFER *rawBuf, const char *signer);
int AH_Msg_EncryptPinTan(AH_MSG *hmsg);
int AH_Msg_DecryptPinTan(AH_MSG *hmsg, GWEN_DB_NODE *gr);
int AH_Msg_VerifyPinTan(AH_MSG *hmsg, GWEN_DB_NODE *gr);


#endif
