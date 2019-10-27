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

  int hbciVersion;
  char *secProfileCode;
  int secProfileVersion;
  AQFINTS_TANMETHOD *tanMethod;

  char *dialogId;

  int isServer;

  AQFINTS_PARSER *parser;
  AQFINTS_TRANSPORT *transport;

  AQFINTS_USERDATA *userData;
  AQFINTS_BPD *bpd;

  int allowedTanMethods[AQFINTS_SESSION_MAX_ALLOWED_TANMETHODS];

  AQFINTS_SESSION_EXCHANGEMESSAGES_FN exchangeMessagesFn;

  AQFINTS_SESSION_FILLOUT_KEYNAME_FN filloutKeynameFn;
  AQFINTS_SESSION_SIGN_FN signFn;
  AQFINTS_SESSION_VERIFY_FN verifyFn;
  AQFINTS_SESSION_ENCRYPT_FN encryptFn;
  AQFINTS_SESSION_DECRYPT_FN decryptFn;

};


#endif

