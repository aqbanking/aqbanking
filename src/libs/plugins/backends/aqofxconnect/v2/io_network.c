/***************************************************************************
 begin       : Thu Jan 16 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


/* plugin headers */
#include "io_network.h"
#include "aqofxconnect/user.h"

/* aqbanking headers */
#include "aqbanking/i18n_l.h"
#include <aqbanking/backendsupport/httpsession.h>

/* gwenhywfar headers */
#include <gwenhywfar/gui.h>

/* system headers */
#include <stdio.h>
#include <errno.h>




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */



static int _createConnection(AB_PROVIDER *pro, AB_USER *u, GWEN_HTTP_SESSION **pSess);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AO_V2_SendAndReceive(AB_PROVIDER *pro, AB_USER *u, const uint8_t *p, unsigned int plen, GWEN_BUFFER **pRbuf)
{
  GWEN_HTTP_SESSION *sess=NULL;
  GWEN_BUFFER *rbuf;
  int rv;

  if (getenv("AQOFX_LOG_COMM")) {
    FILE *f;

    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
              "Saving response in \"/tmp/ofx.log\" ...");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Warning,
                         I18N("Saving communication log to /tmp/ofx.log"));

    f=fopen("/tmp/ofx.log", "a+");
    if (!f) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fopen: %s", strerror(errno));
    }
    else {
      fprintf(f, "\n\nSending:\n");
      fprintf(f, "-------------------------------------\n");
      if (fwrite(p,
                 plen,
                 1,
                 f)!=1) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fwrite: %s", strerror(errno));
      }
      if (fclose(f)) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fclose: %s", strerror(errno));
      }
    }
  }

  /* setup connection */
  rv=_createConnection(pro, u, &sess);
  if (rv) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Could not create connection");
    GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Error, I18N("Could not create connection (%d)"), rv);
    return rv;
  }

  /* send request */
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Sending request..."));
  rv=GWEN_HttpSession_SendPacket(sess, "POST", p, plen);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_Fini(sess);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* wait for response */
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Waiting for response..."));
  rbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=GWEN_HttpSession_RecvPacket(sess, rbuf);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Error receiving packet (%d)", rv);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Network error while waiting for response"));
    GWEN_Buffer_free(rbuf);
    GWEN_HttpSession_Fini(sess);
    GWEN_HttpSession_free(sess);
    return rv;
  }
  else if (!(rv>=200 && rv<=299)) {
    /* not a HTTP: ok code */
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(rbuf);
    GWEN_HttpSession_Fini(sess);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* disconnect (ignore result) */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  /* found a response, transform it */
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Parsing response..."));

  *pRbuf=rbuf;

  if (getenv("AQOFX_LOG_COMM")) {
    FILE *f;

    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Saving response in \"/tmp/ofx.log\" ...");
    f=fopen("/tmp/ofx.log", "a+");
    if (!f) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fopen: %s", strerror(errno));
    }
    else {
      fprintf(f, "\n\nReceived:\n");
      fprintf(f, "-------------------------------------\n");
      if (fwrite(GWEN_Buffer_GetStart(rbuf),
                 GWEN_Buffer_GetUsedBytes(rbuf),
                 1,
                 f)!=1) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fwrite: %s", strerror(errno));
      }
      if (fclose(f)) {
        DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "fclose: %s", strerror(errno));
      }
    }
  }

  return 0;
}



int _createConnection(AB_PROVIDER *pro, AB_USER *u, GWEN_HTTP_SESSION **pSess)
{
  int rv;
  GWEN_HTTP_SESSION *sess;
  const char *addr;
  const char *s;

  /* take bank addr from user */
  addr=AO_User_GetServerAddr(u);
  if (!(addr && *addr)) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "User has no valid address settings");
    return GWEN_ERROR_INVALID;
  }

  sess=AB_HttpSession_new(pro, u, addr, "https", 443);

  /* setup session */
  GWEN_HttpSession_AddFlags(sess, GWEN_HTTP_SESSION_FLAGS_NO_CACHE);

  GWEN_HttpSession_SetHttpContentType(sess, "application/x-ofx");

  GWEN_HttpSession_SetHttpVMajor(sess, AO_User_GetHttpVMajor(u));
  GWEN_HttpSession_SetHttpVMinor(sess, AO_User_GetHttpVMinor(u));

  s=AO_User_GetHttpUserAgent(u);
  GWEN_HttpSession_SetHttpUserAgent(sess, (s && *s)?s:"AqBanking");

  /* init session */
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  *pSess=sess;
  return 0;
}




