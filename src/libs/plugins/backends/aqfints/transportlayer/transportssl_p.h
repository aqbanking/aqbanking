/***************************************************************************
 begin       : Wed Jul 31 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_TRANSPORTSSL_P_H
#define AQFINTS_TRANSPORTSSL_P_H


#include "transportssl.h"

#include <gwenhywfar/httpsession.h>


typedef struct AQFINTS_TRANSPORT_SSL AQFINTS_TRANSPORT_SSL;
struct AQFINTS_TRANSPORT_SSL {
  char *contentType;
  char *userAgent;

  int versionMajor;
  int versionMinor;

  GWEN_HTTP_SESSION *httpSession;

};



#endif

