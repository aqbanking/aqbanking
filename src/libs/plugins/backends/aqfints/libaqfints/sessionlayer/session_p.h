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


#define AQFINTS_SESSION_MAX_ALLOWED_TANMETHODS 16



struct AQFINTS_SESSION {
  GWEN_INHERIT_ELEMENT(AQFINTS_SESSION)
  int _refCount;
  int lastMessageNumSent;
  int lastMessageNumReceived;

  char *logFile;

  int hbciVersion;

  char *appRegKey;
  char *appVersion;

  char *dialogId;

  int isServer;

  AQFINTS_PARSER *parser;
  AQFINTS_TRANSPORT *transport;


  AQFINTS_SESSION_EXCHANGEMESSAGES_FN exchangeMessagesFn;

  AQFINTS_SESSION_DECRYPT_SKEY_FN decryptSessionKeyFn;
  AQFINTS_SESSION_VERIFYPIN_FN verifyPinFn;
  AQFINTS_SESSION_FILLOUT_KEYDESCR_FN filloutKeynameFn;

};


#endif

