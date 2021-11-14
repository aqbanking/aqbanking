/***************************************************************************
 begin       : Sun Oct 27 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_SESSION_SIGN_PINTAN_H
#define AQFINTS_SESSION_SIGN_PINTAN_H


#include "libaqfints/session/session.h"
#include "libaqfints/msg/message.h"
#include "libaqfints/msg/keydescr.h"
#include "libaqfints/parser/segment.h"



int AQFINTS_Session_SignSegmentPinTan(AQFINTS_SESSION *sess,
                                      AQFINTS_MESSAGE *message,
                                      const AQFINTS_KEYDESCR *keyDescr,
                                      AQFINTS_SEGMENT *segFirstToSign,
                                      AQFINTS_SEGMENT *segLastToSign,
                                      int sigHeadNum,
                                      int sigTailNum);

#endif

