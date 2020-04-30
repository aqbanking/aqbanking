/***************************************************************************
 begin       : Thu Aug 01 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_SESSION_DECODE_H
#define AQFINTS_SESSION_DECODE_H


#include "libaqfints/aqfints.h"
#include "msglayer/message.h"
#include "msglayer/keydescr.h"
#include "sessionlayer/session.h"

#include <gwenhywfar/types.h>
#include <gwenhywfar/db.h>



AQFINTS_MESSAGE *AQFINTS_Session_DecodeMessage(AQFINTS_SESSION *sess, const uint8_t *ptrBuffer, uint32_t lenBuffer);

AQFINTS_KEYDESCR *AQFINTS_Session_ReadKeyDescrFromDbHead(GWEN_DB_NODE *dbHead);


#endif

