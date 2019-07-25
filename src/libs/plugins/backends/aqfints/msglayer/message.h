/***************************************************************************
 begin       : Fri Jul 19 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQFINTS_MESSAGE_H
#define AQFINTS_MESSAGE_H


#include "msglayer/parser/parser.h"
#include "msglayer/parser/segment.h"

#include <gwenhywfar/buffer.h>



typedef struct AQFINTS_MESSAGE AQFINTS_MESSAGE;



AQFINTS_MESSAGE *AQFINTS_Message_new(AQFINTS_PARSER *parser);
void AQFINTS_Message_free(AQFINTS_MESSAGE *msg);


int AQFINTS_Message_AddSegment(AQFINTS_MESSAGE *msg,
                               AQFINTS_SEGMENT *segment,
                               GWEN_DB_NODE *data);

GWEN_BUFFER *AQFINTS_Message_GetBuffer(AQFINTS_MESSAGE *msg);
GWEN_BUFFER *AQFINTS_Message_TakeBuffer(AQFINTS_MESSAGE *msg);





#endif

