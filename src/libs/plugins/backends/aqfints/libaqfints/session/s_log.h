/***************************************************************************
 begin       : Sat Aug 03 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_SESSION_LOG_H
#define AQFINTS_SESSION_LOG_H


#include "session/session.h"



void AQFINTS_Session_LogMessage(AQFINTS_SESSION *sess,
                                const uint8_t *ptrLogData,
                                uint32_t lenLogData,
                                int rec,
                                int crypt);



#endif

