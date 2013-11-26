/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dialog_p.h"
#include "user_l.h"

#include <aqbanking/httpsession.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>


GWEN_INHERIT(GWEN_HTTP_SESSION, EBC_DIALOG)



GWEN_HTTP_SESSION *EBC_Dialog_new(AB_PROVIDER *pro, AB_USER *u) {
  GWEN_HTTP_SESSION *sess;
  EBC_DIALOG *xsess;
  const char *url;
  uint32_t flags;
  const char *s;

  url=EBC_User_GetServerUrl(u);
  if (url==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No URL for user [%s]",
	      AB_User_GetUserId(u));
    return NULL;
  }

  /* create session */
  sess=AB_HttpSession_new(pro, u, url, "https", 443);
  assert(sess);

  /* extend session */
  GWEN_NEW_OBJECT(EBC_DIALOG, xsess);
  GWEN_INHERIT_SETDATA(GWEN_HTTP_SESSION, EBC_DIALOG, sess, xsess,
		       EBC_Dialog_FreeData);

  /* set flags according to user settings */
  flags=EBC_User_GetFlags(u);
  if (flags & EBC_USER_FLAGS_FORCE_SSLV3)
    GWEN_HttpSession_AddFlags(sess, GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3);

  /* set HTTP config according to user settings */
  GWEN_HttpSession_SetHttpUserAgent(sess, EBC_User_GetHttpUserAgent(u));
  s=EBC_User_GetHttpContentType(u);
  if (s==NULL || *s==0)
    s="text/xml; charset=UTF-8";
  GWEN_HttpSession_SetHttpContentType(sess, s);
  GWEN_HttpSession_SetHttpVMajor(sess, EBC_User_GetHttpVMajor(u));
  GWEN_HttpSession_SetHttpVMinor(sess, EBC_User_GetHttpVMinor(u));

  return sess;
}



void GWENHYWFAR_CB EBC_Dialog_FreeData(GWEN_UNUSED void *bp, void *p) {
  EBC_DIALOG *dsess;

  dsess=(EBC_DIALOG*) p;

  GWEN_FREE_OBJECT(dsess);
}




int EBC_Dialog_ExchangeMessages(GWEN_HTTP_SESSION *sess,
                                EB_MSG *msg,
                                EB_MSG **pResponse) {
  AB_USER *u;
  int rv;
  GWEN_BUFFER *sendBuf;
  GWEN_BUFFER *recvBuf;
  EB_MSG *mResponse;

  /* preparations */
  u=AB_HttpSession_GetUser(sess);
  assert(u);
  sendBuf=GWEN_Buffer_new(0, 1024, 0, 1);
  recvBuf=GWEN_Buffer_new(0, 1024, 0, 1);

  /* convert message to buffer for sending */
  EB_Msg_toBuffer(msg, sendBuf);

#if 0
  if (GWEN_Logger_GetLevel(AQEBICS_LOGDOMAIN)>=GWEN_LoggerLevel_Debug) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Sending this:");
    fprintf(stderr, "====================================\n");
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(sendBuf));
    fprintf(stderr, "====================================\n");
  }
#endif

  /* send request */
  rv=GWEN_HttpSession_SendPacket(sess,
				 "POST",
				 (const uint8_t*)GWEN_Buffer_GetStart(sendBuf),
				 GWEN_Buffer_GetUsedBytes(sendBuf));
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error sending request (%d)", rv);
    GWEN_Buffer_free(recvBuf);
    GWEN_Buffer_free(sendBuf);
    return rv;
  }
  GWEN_Buffer_free(sendBuf);

  /* receive response */
  rv=GWEN_HttpSession_RecvPacket(sess, recvBuf);
  if (rv<0 || rv>=300) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error sending request (%d)", rv);
    GWEN_Buffer_free(recvBuf);
    return rv;
  }

#if 0
  if (GWEN_Logger_GetLevel(AQEBICS_LOGDOMAIN)>=GWEN_LoggerLevel_Debug) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Received this:");
    fprintf(stderr, "====================================\n");
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(recvBuf));
    fprintf(stderr, "====================================\n");
  }
#endif

  /* convert buffer to EBICS message */
  mResponse=EB_Msg_fromBuffer(GWEN_Buffer_GetStart(recvBuf),
                              GWEN_Buffer_GetUsedBytes(recvBuf));
  GWEN_Buffer_free(recvBuf);
  if (!mResponse) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Invalid response (no EBICS message)");
    return GWEN_ERROR_BAD_DATA;
  }

  *pResponse=mResponse;
  return 0;
}





