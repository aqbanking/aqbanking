/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_PROVIDER_REQUEST_P_H
#define AG_PROVIDER_REQUEST_P_H

#include "provider_request.h"
#include "gwenhywfar/json.h"


typedef struct AG_HTTP_SESSION_HEADER AG_HTTP_SESSION_HEADER;
struct AG_HTTP_SESSION_HEADER {
  GWEN_DB_NODE *header;
  char *url;
};



#endif
