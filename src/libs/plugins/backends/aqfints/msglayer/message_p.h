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


#include "msglayer/message.h"


struct AQFINTS_MESSAGE {
  AQFINTS_PARSER *parser;

  AQFINTS_KEYNAME_LIST *signerList;
  AQFINTS_KEYNAME *crypter;

  AQFINTS_SEGMENT_LIST *segmentList;
};






#endif

