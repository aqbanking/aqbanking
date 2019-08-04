/***************************************************************************
 begin       : Thu Aug 01 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_SESSION_P_H
#define AQFINTS_SESSION_P_H

#include "session.h"


struct AQFINTS_SESSION {
  GWEN_INHERIT_ELEMENT(AQFINTS_SESSION)
  int _refCount;
  int lastMessageNumSent;
  int lastMessageNumReceived;

  int hbciVersion;

  char *dialogId;

  AQFINTS_PARSER *parser;
  AQFINTS_TRANSPORT *transport;

  AQFINTS_USERDATA_LIST *userDataList;
  AQFINTS_BPD *bpd;

  AQFINTS_SESSION_EXCHANGEMESSAGES_FN exchangeMessagesFn;
};


#endif

