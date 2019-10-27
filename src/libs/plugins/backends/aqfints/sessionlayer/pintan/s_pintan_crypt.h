/***************************************************************************
 begin       : Sun Oct 27 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQFINTS_SESSION_PINTAN_CRYPT_H
#define AQFINTS_SESSION_PINTAN_CRYPT_H


#include "aqfints/sessionlayer/session.h"


int AQFINTS_SessionPinTan_WrapCrypt(AQFINTS_SESSION *sess,
                                    const AQFINTS_KEYNAME *keyName,
                                    const uint8_t *ptrEncryptedData,
                                    uint32_t lenEncryptedData,
                                    GWEN_BUFFER *msgBuffer);





#endif
