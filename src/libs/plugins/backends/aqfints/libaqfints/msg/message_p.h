/***************************************************************************
 begin       : Fri Jul 19 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQFINTS_MESSAGE_P_H
#define AQFINTS_MESSAGE_P_H


#include "libaqfints/msg/message.h"


struct AQFINTS_MESSAGE {
  int messageNumber;
  int refMessageNumber;

  AQFINTS_KEYDESCR_LIST *signerList;
  AQFINTS_KEYDESCR *crypter;

  AQFINTS_SEGMENT_LIST *segmentList;

  int hbciVersion;
  char *dialogId;

  char *tanJobCode;

  uint32_t flags;
};






#endif

