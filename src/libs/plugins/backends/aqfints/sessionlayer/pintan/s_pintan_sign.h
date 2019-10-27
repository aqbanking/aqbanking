/***************************************************************************
 begin       : Sun Oct 27 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQFINTS_SESSION_PINTAN_SIGN_H
#define AQFINTS_SESSION_PINTAN_SIGN_H


#include "aqfints/sessionlayer/session.h"


int AQFINTS_SessionPinTan_WrapSignatures(AQFINTS_SESSION *sess,
                                         AQFINTS_KEYNAME_LIST *keyNameList,
                                         int firstSegNum,
                                         int lastSegNum,
                                         GWEN_BUFFER *msgBuffer);





#endif
