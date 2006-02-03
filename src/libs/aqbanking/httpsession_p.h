/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AB_HTTPSESS_P_H
#define AB_HTTPSESS_P_H

#include "httpsession.h"

#include <aqbanking/user.h>
#include <gwenhywfar/netlayer.h>
#include <gwenhywfar/url.h>



struct AB_HTTPSESSION {
  GWEN_INHERIT_ELEMENT(AB_HTTPSESSION)
  GWEN_LIST_ELEMENT(AB_HTTPSESSION)
  AB_PROVIDER *provider;
  AB_USER *user;
  GWEN_NETLAYER *netLayer;
  GWEN_URL *lastUrl;
  char *logFolder;
  GWEN_TYPE_UINT32 sessionId;

  GWEN_TYPE_UINT32 flags;
  int connectTimeout;
  int transferTimeout;
  GWEN_NETLAYER_HTTP_VERSION httpVersion;
};


static int AB_HttpSession__EnsureConnection(AB_HTTPSESSION *dlg,
                                            const GWEN_URL *url);
static void AB_HttpSession__AddPeerCertFolder(AB_HTTPSESSION *hc,
                                              GWEN_BUFFER *nbuf);




#endif

