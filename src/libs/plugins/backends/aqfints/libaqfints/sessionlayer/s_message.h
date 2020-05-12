/***************************************************************************
 begin       : Sat Aug 03 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_SESSION_MESSAGE_H
#define AQFINTS_SESSION_MESSAGE_H


#include "sessionlayer/session.h"
#include "parser/segment.h"



int AQFINTS_Session_WrapMessageHeadAndTail(AQFINTS_SESSION *sess,
                                           AQFINTS_SEGMENT_LIST *segmentList,
                                           const char *dialogId,
                                           int msgNum, int refMsgNum, int lastSegNum);



#endif

