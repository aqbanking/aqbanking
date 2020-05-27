/***************************************************************************
 begin       : Sun Oct 27 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_SESSION_VERIFY_HBCI_H
#define AQFINTS_SESSION_VERIFY_HBCI_H


#include "sessionlayer/session.h"
#include "msglayer/message.h"
#include "msglayer/keydescr.h"
#include "parser/segment.h"



int AQFINTS_Session_VerifySegmentHbci(AQFINTS_SESSION *sess,
                                      AQFINTS_MESSAGE *message,
                                      const AQFINTS_KEYDESCR *keyDescr,
                                      AQFINTS_SEGMENT *segSigHead,
                                      AQFINTS_SEGMENT *segSigTail,
                                      AQFINTS_SEGMENT *segFirstSigned,
                                      AQFINTS_SEGMENT *segLastSigned);

#endif

