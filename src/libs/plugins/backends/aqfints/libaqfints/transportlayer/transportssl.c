/***************************************************************************
 begin       : Wed Jul 31 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "transportssl_p.h"

#include "libaqfints/aqfints.h"

#include <gwenhywfar/base64.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/debug.h>

#define I18N




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static void GWENHYWFAR_CB freeData(void *bp, void *p);
static int createIoLayer(AQFINTS_TRANSPORT *trans);
static int transportConnect(AQFINTS_TRANSPORT *trans);
static int transportDisconnect(AQFINTS_TRANSPORT *trans);
static int transportSendMessage(AQFINTS_TRANSPORT *trans, const char *ptrBuffer, int lenBuffer);
static int transportReallySendMessage(AQFINTS_TRANSPORT *trans, const char *ptrBuffer, int lenBuffer);

static int transportReceiveMessage(AQFINTS_TRANSPORT *trans, GWEN_BUFFER *buffer);
static int transportReallyReceiveMessage(AQFINTS_TRANSPORT *trans, GWEN_BUFFER *buffer);
static int recvPacket(AQFINTS_TRANSPORT *trans, GWEN_BUFFER *tbuf);
static void trimBuffer(GWEN_BUFFER *tbuf);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



GWEN_INHERIT(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL)


AQFINTS_TRANSPORT *AQFINTS_TransportSsl_new(const char *url)
{
  AQFINTS_TRANSPORT *trans;
  AQFINTS_TRANSPORT_SSL *xtrans;

  trans=AQFINTS_Transport_new();
  GWEN_NEW_OBJECT(AQFINTS_TRANSPORT_SSL, xtrans);
  GWEN_INHERIT_SETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans, xtrans, freeData);

  AQFINTS_Transport_SetUrl(trans, url);

  /* set virtual functions */
  AQFINTS_Transport_SetConnectFn(trans, transportConnect);
  AQFINTS_Transport_SetDisconnectFn(trans, transportDisconnect);
  AQFINTS_Transport_SetSendMessageFn(trans, transportSendMessage);
  AQFINTS_Transport_SetReceiveMessageFn(trans, transportReceiveMessage);

  return trans;
}



void GWENHYWFAR_CB freeData(void *bp, void *p)
{
  AQFINTS_TRANSPORT_SSL *xtrans;

  xtrans=(AQFINTS_TRANSPORT_SSL *) p;
  free(xtrans->contentType);
  free(xtrans->userAgent);
  GWEN_HttpSession_free(xtrans->httpSession);
  GWEN_FREE_OBJECT(xtrans);
}



int AQFINTS_TransportSsl_TestConnection(AQFINTS_TRANSPORT *trans)
{
  AQFINTS_TRANSPORT_SSL *xtrans;
  int rv;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  GWEN_Gui_ProgressLog(0,
                       GWEN_LoggerLevel_Notice,
                       I18N("Preparing connection"));
  rv=transportConnect(trans);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Could not connect (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error preparing connection"));
    return rv;
  }

  rv=GWEN_HttpSession_ConnectionTest(xtrans->httpSession);
  GWEN_HttpSession_Fini(xtrans->httpSession);
  GWEN_HttpSession_free(xtrans->httpSession);
  xtrans->httpSession=NULL;
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



const char *AQFINTS_TransportSsl_GetContentType(const AQFINTS_TRANSPORT *trans)
{
  AQFINTS_TRANSPORT_SSL *xtrans;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  return xtrans->contentType;
}



void AQFINTS_TransportSsl_SetContentType(AQFINTS_TRANSPORT *trans, const char *s)
{
  AQFINTS_TRANSPORT_SSL *xtrans;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  if (xtrans->contentType)
    free(xtrans->contentType);

  if (s)
    xtrans->contentType=strdup(s);
  else
    xtrans->contentType=NULL;
}



const char *AQFINTS_TransportSsl_GetUserAgent(const AQFINTS_TRANSPORT *trans)
{
  AQFINTS_TRANSPORT_SSL *xtrans;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  return xtrans->userAgent;
}



void AQFINTS_TransportSsl_SetUserAgent(AQFINTS_TRANSPORT *trans, const char *s)
{
  AQFINTS_TRANSPORT_SSL *xtrans;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  if (xtrans->userAgent)
    free(xtrans->userAgent);

  if (s)
    xtrans->userAgent=strdup(s);
  else
    xtrans->userAgent=NULL;
}



int AQFINTS_TransportSsl_GetVersionMajor(const AQFINTS_TRANSPORT *trans)
{
  AQFINTS_TRANSPORT_SSL *xtrans;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  return xtrans->versionMajor;
}



void AQFINTS_TransportSsl_SetVersionMajor(AQFINTS_TRANSPORT *trans, int v)
{
  AQFINTS_TRANSPORT_SSL *xtrans;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  xtrans->versionMajor=v;
}



int AQFINTS_TransportSsl_GetVersionMinor(const AQFINTS_TRANSPORT *trans)
{
  AQFINTS_TRANSPORT_SSL *xtrans;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  return xtrans->versionMinor;
}



void AQFINTS_TransportSsl_SetVersionMinor(AQFINTS_TRANSPORT *trans, int v)
{
  AQFINTS_TRANSPORT_SSL *xtrans;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  xtrans->versionMinor=v;
}





int createIoLayer(AQFINTS_TRANSPORT *trans)
{
  AQFINTS_TRANSPORT_SSL *xtrans;
  GWEN_HTTP_SESSION *sess;
  int rv;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  sess=GWEN_HttpSession_new(AQFINTS_Transport_GetUrl(trans), "https", 443);
  assert(sess);

  GWEN_HttpSession_AddFlags(sess, GWEN_HTTP_SESSION_FLAGS_NO_CACHE);
  GWEN_HttpSession_AddFlags(sess, GWEN_HTTP_SESSION_FLAGS_TLS_IGN_PREMATURE_CLOSE);

  if (xtrans->contentType)
    GWEN_HttpSession_SetHttpContentType(sess, xtrans->contentType);

  if (xtrans->userAgent)
    GWEN_HttpSession_SetHttpUserAgent(sess, xtrans->userAgent);

  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  xtrans->httpSession=sess;
  return 0;
}



int transportConnect(AQFINTS_TRANSPORT *trans)
{
  AQFINTS_TRANSPORT_SSL *xtrans;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  if (xtrans->httpSession==NULL) {
    int rv;

    rv=createIoLayer(trans);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    AQFINTS_Transport_AddRuntimeFlags(trans, AQFINTS_TRANSPORT_RTFLAGS_CONNECTED);
  }

  return 0;
}



int transportDisconnect(AQFINTS_TRANSPORT *trans)
{
  AQFINTS_TRANSPORT_SSL *xtrans;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  if (xtrans->httpSession) {
    GWEN_HttpSession_Fini(xtrans->httpSession);
    GWEN_HttpSession_free(xtrans->httpSession);
    xtrans->httpSession=NULL;
    AQFINTS_Transport_SubRuntimeFlags(trans, AQFINTS_TRANSPORT_RTFLAGS_CONNECTED);
  }

  return 0;
}



int transportSendMessage(AQFINTS_TRANSPORT *trans, const char *ptrBuffer, int lenBuffer)
{
  AQFINTS_TRANSPORT_SSL *xtrans;
  int rv;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  rv=transportReallySendMessage(trans, ptrBuffer, lenBuffer);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Could not BASE64 encode data (%d)", rv);
    GWEN_HttpSession_Fini(xtrans->httpSession);
    GWEN_HttpSession_free(xtrans->httpSession);
    xtrans->httpSession=NULL;
    return rv;
  }

  return 0;
}



int transportReallySendMessage(AQFINTS_TRANSPORT *trans, const char *ptrBuffer, int lenBuffer)
{
  AQFINTS_TRANSPORT_SSL *xtrans;
  GWEN_BUFFER *tbuf;
  int rv;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  if (xtrans->httpSession==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No HTTP session");
    return GWEN_ERROR_INVALID;
  }

  /* base64 encode */
  tbuf=GWEN_Buffer_new(0, lenBuffer, 0, 1);
  rv=GWEN_Base64_Encode((const unsigned char *)ptrBuffer, lenBuffer, tbuf, 0);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Could not BASE64 encode data (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_Buffer_AppendString(tbuf, "\r\n");

  rv=GWEN_HttpSession_SendPacket(xtrans->httpSession, "POST",
                                 (const uint8_t *) GWEN_Buffer_GetStart(tbuf),
                                 GWEN_Buffer_GetUsedBytes(tbuf));
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_Buffer_free(tbuf);

  DBG_INFO(AQFINTS_LOGDOMAIN, "Message sent.");
  return 0;
}





int transportReceiveMessage(AQFINTS_TRANSPORT *trans, GWEN_BUFFER *buffer)
{
  AQFINTS_TRANSPORT_SSL *xtrans;
  int rv;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  rv=transportReallyReceiveMessage(trans, buffer);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_Fini(xtrans->httpSession);
    GWEN_HttpSession_free(xtrans->httpSession);
    xtrans->httpSession=NULL;
    return rv;
  }

  return 0;
}



int recvPacket(AQFINTS_TRANSPORT *trans, GWEN_BUFFER *tbuf)
{
  AQFINTS_TRANSPORT_SSL *xtrans;
  int rv;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_SSL, trans);
  assert(xtrans);

  /* read HBCI message */
  rv=GWEN_HttpSession_RecvPacket(xtrans->httpSession, tbuf);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  else if (rv==0) {
    /* not a HTTP code */
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return GWEN_ERROR_INTERNAL;
  }
  else if (!(rv>=200 && rv<=299)) {
    /* not a HTTP: ok code */
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



void trimBuffer(GWEN_BUFFER *tbuf)
{
  const char *p;
  int i;

  i=GWEN_Buffer_GetUsedBytes(tbuf);
  p=GWEN_Buffer_GetStart(tbuf);
  while (i>0) {
    if (p[i-1]!=0)
      break;
    i--;
  }
  GWEN_Buffer_Crop(tbuf, 0, i);
}



int transportReallyReceiveMessage(AQFINTS_TRANSPORT *trans, GWEN_BUFFER *buffer)
{
  GWEN_BUFFER *tbuf;
  int rv;
  int msgSize;

  tbuf=GWEN_Buffer_new(0, 1024, 0, 1);

  /* read HBCI message */
  rv=recvPacket(trans, tbuf);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* optionally decode BASE64 encoded message */
  if (strstr(GWEN_Buffer_GetStart(tbuf), "HNHBK:")==NULL) {
    GWEN_BUFFER *bbuf;

    bbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(tbuf), 0, 1);
    rv=GWEN_Base64_Decode((const unsigned char *) GWEN_Buffer_GetStart(tbuf),
                          GWEN_Buffer_GetUsedBytes(tbuf),
                          bbuf);
    if (rv) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "Could not decode BASE64 message (%d)", rv);
      /* for debugging purposes */
      GWEN_Buffer_Dump(tbuf, 2);
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("Could not BASE64-decode the message"));
      GWEN_Buffer_free(bbuf);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
    GWEN_Buffer_free(tbuf);
    tbuf=bbuf;
  }

  /* check message for HBCI message */
  if (strstr(GWEN_Buffer_GetStart(tbuf), "HNHBK:")==NULL) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "Received message is not HBCI");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Received message is not HBCI"));
    /* for debugging purposes */
    GWEN_Buffer_Dump(tbuf, 2);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* trim response */
  trimBuffer(tbuf);

  /* check message size */
  msgSize=AQFINTS_Transport_DetermineMessageSize(GWEN_Buffer_GetStart(tbuf));
  if (msgSize<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", msgSize);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Unparsable message received"));
    /* for debugging purposes */
    GWEN_Buffer_Dump(tbuf, 2);
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_BAD_DATA;
  }

  /* check message size */
  if (GWEN_Buffer_GetUsedBytes(tbuf)<msgSize) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Bad message size (%d<%d)",
              GWEN_Buffer_GetUsedBytes(tbuf), msgSize);
    /* for debugging purposes */
    GWEN_Buffer_Dump(tbuf, 2);
    GWEN_Buffer_free(tbuf);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Received message was truncated"));
    return GWEN_ERROR_BAD_DATA;
  }

  GWEN_Buffer_AppendBuffer(buffer, tbuf);
  GWEN_Buffer_free(tbuf);

  return 0;
}








