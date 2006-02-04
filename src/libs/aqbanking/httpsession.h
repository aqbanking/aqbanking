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


#ifndef AB_HTTPSESS_H
#define AB_HTTPSESS_H

#include <aqbanking/banking.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/nl_http.h>


#define AB_HTTPSESSION_FLAGS_ALLOW_REDIRECT 0x00000001
#define AB_HTTPSESSION_FLAGS_REUSE          0x00000002

#define AB_HTTPSESSION_DEFAULT_CONNECT_TIMEOUT  30
#define AB_HTTPSESSION_DEFAULT_TRANSFER_TIMEOUT 60



typedef struct AB_HTTPSESSION AB_HTTPSESSION;
GWEN_INHERIT_FUNCTION_LIB_DEFS(AB_HTTPSESSION, AQBANKING_API)
GWEN_LIST_FUNCTION_LIB_DEFS(AB_HTTPSESSION, AB_HttpSession, AQBANKING_API)


AQBANKING_API 
AB_HTTPSESSION *AB_HttpSession_new(AB_PROVIDER *pro, AB_USER *u);

AQBANKING_API 
void AB_HttpSession_free(AB_HTTPSESSION *hc);

AQBANKING_API 
AB_USER *AB_HttpSession_GetUser(const AB_HTTPSESSION *hc);

AQBANKING_API 
AB_PROVIDER *AB_HttpSession_GetProvider(const AB_HTTPSESSION *hc);

AQBANKING_API 
GWEN_TYPE_UINT32 AB_HttpSession_GetSessionId(const AB_HTTPSESSION *hc);


AQBANKING_API GWEN_NETLAYER_HTTP_VERSION
AB_HttpSession_GetHttpVersion(const AB_HTTPSESSION *hc);

AQBANKING_API 
void AB_HttpSession_SetHttpVersion(AB_HTTPSESSION *hc,
                                   GWEN_NETLAYER_HTTP_VERSION v);

AQBANKING_API 
GWEN_TYPE_UINT32 AB_HttpSession_GetFlags(const AB_HTTPSESSION *hc);

AQBANKING_API 
void AB_HttpSession_SetFlags(AB_HTTPSESSION *hc, GWEN_TYPE_UINT32 f);

AQBANKING_API 
void AB_HttpSession_AddFlags(AB_HTTPSESSION *hc, GWEN_TYPE_UINT32 f);

AQBANKING_API 
void AB_HttpSession_SubFlags(AB_HTTPSESSION *hc, GWEN_TYPE_UINT32 f);

AQBANKING_API 
int AB_HttpSession_GetConnectTimeout(const AB_HTTPSESSION *hc);

AQBANKING_API 
void AB_HttpSession_SetConnectTimeout(AB_HTTPSESSION *hc, int i);

AQBANKING_API 
int AB_HttpSession_GetTransferTimeout(const AB_HTTPSESSION *hc);

AQBANKING_API 
void AB_HttpSession_SetTransferTimeout(AB_HTTPSESSION *hc, int i);


AQBANKING_API 
int AB_HttpSession_Open(AB_HTTPSESSION *hc);

AQBANKING_API 
void AB_HttpSession_Close(AB_HTTPSESSION *hc);

AQBANKING_API 
int AB_HttpSession_SendRequest(AB_HTTPSESSION *hc,
                               const char *command,
                               const GWEN_URL *url,
                               GWEN_DB_NODE *dbSendHeader,
                               const char *pSendBody,
                               int lSendBody,
                               GWEN_BUFFER *recvBuf);





#endif

