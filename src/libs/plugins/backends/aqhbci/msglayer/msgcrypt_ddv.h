/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_MSGCRYPT_DDV_H
#define AH_MSGCRYPT_DDV_H

#include "aqhbci/msglayer/message_l.h"


int AH_MsgDdv_PrepareCryptoSeg(AH_MSG *hmsg,
                               AB_USER *u,
                               const GWEN_CRYPT_TOKEN_KEYINFO *ki,
                               GWEN_DB_NODE *cfg,
                               int crypt,
                               int createCtrlRef);
int AH_Msg_SignDdv(AH_MSG *hmsg, GWEN_BUFFER *rawBuf, const char *signer);
int AH_Msg_EncryptDdv(AH_MSG *hmsg);
int AH_Msg_DecryptDdv(AH_MSG *hmsg, GWEN_DB_NODE *gr);
int AH_Msg_VerifyDdv(AH_MSG *hmsg, GWEN_DB_NODE *gr);



#endif

