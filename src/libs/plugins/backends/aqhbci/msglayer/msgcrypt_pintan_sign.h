/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_MSGCRYPT_PINTAN_SIGN_H
#define AH_MSGCRYPT_PINTAN_SIGN_H

#include "aqhbci/msglayer/msgcrypt_pintan.h"


int AH_Msg_SignPinTan(AH_MSG *hmsg, GWEN_BUFFER *rawBuf, const char *signer);


#endif
