/***************************************************************************
 begin       : Thu Aug 01 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_SESSION_ENCODE_H
#define AQFINTS_SESSION_ENCODE_H


#include "libaqfints/aqfints.h"
#include "libaqfints/msg/message.h"
#include "libaqfints/msg/keydescr.h"
#include "libaqfints/session/session.h"

#include <gwenhywfar/types.h>
#include <gwenhywfar/db.h>



GWEN_BUFFER *AQFINTS_Session_EncodeMessage(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *message);




#endif

