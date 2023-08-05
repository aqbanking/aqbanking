/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_MSGCRYPT_PINTAN_VERIFY_H
#define AH_MSGCRYPT_PINTAN_VERIFY_H

#include "aqhbci/msglayer/msgcrypt_pintan.h"


int AH_Msg_VerifyPinTan(AH_MSG *hmsg, GWEN_DB_NODE *gr);


#endif
