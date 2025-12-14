/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "aqpaypal/provider_l.h"
#include "aqpaypal/user_l.h"

#include <aqbanking/backendsupport/httpsession.h>

#include <gwenhywfar/debug.h>

#include <ctype.h>
#include <errno.h>


/* #define AQPAYPAL_ENABLE_LOGTOFILE */



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static GWEN_BUFFER *_sendAndReceive(GWEN_HTTP_SESSION *sess, const char *requestString, const char *jobName);
static GWEN_HTTP_SESSION *_setupHttpSession(AB_PROVIDER *pro, AB_USER *u);
static GWEN_DB_NODE *_parseAndCheckResponse(AB_PROVIDER *pro, const char *recvdData);
static int _parseResponse(AB_PROVIDER *pro, const char *s, GWEN_DB_NODE *db);

#ifdef AQPAYPAL_ENABLE_LOGTOFILE
  static void _logToFile(const char *fileName, const char *direction, const char *jobName, const char *ptr, uint32_t len);
#endif



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

GWEN_DB_NODE *APY_Provider_SendRequestParseResponse(AB_PROVIDER *pro, AB_USER *u, const char *requestString, const char *jobName)
{
  GWEN_HTTP_SESSION *sess;
  GWEN_BUFFER *tbuf;
  int rv;
  GWEN_DB_NODE *dbResponse;

  sess=_setupHttpSession(pro, u);
  if (sess==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not create http session for user [%s]", AB_User_GetUserId(u));
    return NULL;
  }

  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_free(sess);
    return NULL;
  }

  tbuf=_sendAndReceive(sess, requestString, jobName);
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);
  if (tbuf==NULL) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here");
    return NULL;
  }

  /* parse response */
  dbResponse=_parseAndCheckResponse(pro, GWEN_Buffer_GetStart(tbuf));
  if (dbResponse==NULL) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return NULL;
  }

  return dbResponse;
}



int APY_Provider_SetupUrlString(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *tbuf)
{
  const char *s;

  s=APY_User_GetApiUserId(u);
  if (!(s && *s)) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing user id");
    return GWEN_ERROR_INVALID;
  }
  GWEN_Buffer_AppendString(tbuf, "user=");
  GWEN_Text_EscapeToBuffer(s, tbuf);

  s=APY_User_GetApiPassword(u);
  if (!(s && *s)) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing API password");
    return GWEN_ERROR_INVALID;
  }
  GWEN_Buffer_AppendString(tbuf, "&pwd=");
  GWEN_Text_EscapeToBuffer(s, tbuf);

  s=APY_User_GetApiSignature(u);
  if (!(s && *s)) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing API signature");
    return GWEN_ERROR_INVALID;
  }
  GWEN_Buffer_AppendString(tbuf, "&signature=");
  GWEN_Text_EscapeToBuffer(s, tbuf);

  GWEN_Buffer_AppendString(tbuf, "&version=");
  GWEN_Text_EscapeToBuffer(AQPAYPAL_API_VER, tbuf);

  return 0;
}




GWEN_HTTP_SESSION *_setupHttpSession(AB_PROVIDER *pro, AB_USER *u)
{
  GWEN_HTTP_SESSION *sess;
  int vmajor;
  int vminor;

  sess=AB_HttpSession_new(pro, u, APY_User_GetServerUrl(u), "https", 443);
  if (sess==NULL) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Could not create http session for user [%s]", AB_User_GetUserId(u));
    return NULL;
  }

  vmajor=APY_User_GetHttpVMajor(u);
  vminor=APY_User_GetHttpVMinor(u);
  if (vmajor==0 && vminor==0) {
    vmajor=1;
    vminor=0;
  }
  GWEN_HttpSession_SetHttpVMajor(sess, vmajor);
  GWEN_HttpSession_SetHttpVMinor(sess, vminor);
  GWEN_HttpSession_SetHttpContentType(sess, "application/x-www-form-urlencoded");

  return sess;
}



GWEN_BUFFER *_sendAndReceive(GWEN_HTTP_SESSION *sess, const char *requestString, const char *jobName)
{
  GWEN_BUFFER *tbuf;
  int rv;

#ifdef AQPAYPAL_ENABLE_LOGTOFILE
  if (getenv("AQPAYPAL_LOG_COMM"))
    _logToFile("paypal.log", "Sending", jobName, requestString, strlen(requestString));
#endif

  /* send request */
  rv=GWEN_HttpSession_SendPacket(sess, "POST", (const uint8_t *) requestString, strlen(requestString));
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    return NULL;
  }

  /* get response */
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_HttpSession_RecvPacket(sess, tbuf);
  if (rv<0 || rv<200 || rv>299) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return NULL;
  }

#ifdef AQPAYPAL_ENABLE_LOGTOFILE
  if (getenv("AQPAYPAL_LOG_COMM"))
    _logToFile("paypal.log", "Received", jobName, GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(tbuf));
#endif

  if (GWEN_Buffer_GetUsedBytes(tbuf))
    return tbuf;
  GWEN_Buffer_free(tbuf);
  return NULL;
}



#ifdef AQPAYPAL_ENABLE_LOGTOFILE
void _logToFile(const char *fileName, const char *direction, const char *jobName, const char *ptr, uint32_t len)
{
  FILE *f;

  f=fopen(fileName, "a+");
  if (f) {
    fprintf(f, "\n============================================\n");
    fprintf(f, "%s (%s)\n", direction?direction:"<no direction>", jobName?jobName:"<no info>");
    if (ptr && len>0) {
      if (1!=fwrite(ptr, len, 1, f)) {
        DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
        fclose(f);
      }
      else {
        if (fclose(f)) {
          DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
        }
      }
    }
    else {
      fprintf(f, "Empty data.\n");
      if (fclose(f)) {
        DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d: %s)", errno, strerror(errno));
      }
    }
  }
}
#endif



GWEN_DB_NODE *_parseAndCheckResponse(AB_PROVIDER *pro, const char *recvdData)
{
  GWEN_DB_NODE *dbResponse;
  int rv;
  const char *s;

  dbResponse=GWEN_DB_Group_new("response");
  rv=_parseResponse(pro, recvdData, dbResponse);

#ifdef AQPAYPAL_ENABLE_LOGTOFILE
  if (getenv("AQPAYPAL_LOG_COMM")) {
    static int debugCounter=0;
    char namebuf[64];

    snprintf(namebuf, sizeof(namebuf)-1, "paypal-%02x.db", debugCounter++);
    GWEN_DB_WriteFile(dbResponse, namebuf, GWEN_DB_FLAGS_DEFAULT);
  }
#endif

  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbResponse);
    return NULL;
  }

  /* check result */
  s=GWEN_DB_GetCharValue(dbResponse, "ACK", 0, NULL);
  if (s && *s) {
    if (strcasecmp(s, "Success")==0 ||
        strcasecmp(s, "SuccessWithWarning")==0) {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "Success");
    }
    else {
      DBG_INFO(AQPAYPAL_LOGDOMAIN, "No positive response from server");
      GWEN_DB_Group_free(dbResponse);
      return NULL;
    }
  }
  else {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "No ACK response from server");
    GWEN_DB_Group_free(dbResponse);
    return NULL;
  }

  return dbResponse;
}



int _parseResponse(AB_PROVIDER *pro, const char *s, GWEN_DB_NODE *db)
{
  /* read vars */
  while (*s) {
    GWEN_BUFFER *bName;
    GWEN_BUFFER *bValue;
    const char *p;
    GWEN_DB_NODE *dbT;

    bName=GWEN_Buffer_new(0, 256, 0, 1);
    bValue=GWEN_Buffer_new(0, 256, 0, 1);
    p=s;
    while (*p && *p!='&' && *p!='=')
      p++;
    if (p!=s)
      GWEN_Buffer_AppendBytes(bName, s, (p-s));
    s=p;
    if (*p=='=') {
      s++;
      p=s;
      while (*p && *p!='&')
        p++;
      if (p!=s)
        GWEN_Buffer_AppendBytes(bValue, s, (p-s));
      s=p;
    }

    dbT=db;
    if (strncasecmp(GWEN_Buffer_GetStart(bName), "L_ERRORCODE", 11)!=0 &&
        strncasecmp(GWEN_Buffer_GetStart(bName), "L_SHORTMESSAGE", 14)!=0 &&
        strncasecmp(GWEN_Buffer_GetStart(bName), "L_LONGMESSAGE", 13)!=0 &&
        strncasecmp(GWEN_Buffer_GetStart(bName), "L_SEVERITYCODE", 14)!=0 &&
        strncasecmp(GWEN_Buffer_GetStart(bName), "SHIPTOSTREET2", 13)!=0) {
      int i;

      i=GWEN_Buffer_GetUsedBytes(bName)-1;
      if (i>0) {
        char *t;

        t=GWEN_Buffer_GetStart(bName)+i;
        while (i && isdigit(*t)) {
          i--;
          t--;
        }
        if (i>0) {
          t++;
          if (*t) {
            dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, t);
            *t=0;
          }
        }
      }
    }

    /* store variable/value pair */
    if (strlen(GWEN_Buffer_GetStart(bName))) {
      GWEN_BUFFER *xbuf;

      xbuf=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Text_UnescapeToBufferTolerant(GWEN_Buffer_GetStart(bValue), xbuf);
      GWEN_DB_SetCharValue(dbT,
                           GWEN_DB_FLAGS_DEFAULT,
                           GWEN_Buffer_GetStart(bName),
                           GWEN_Buffer_GetStart(xbuf));
      GWEN_Buffer_free(xbuf);
    }

    GWEN_Buffer_free(bValue);
    GWEN_Buffer_free(bName);
    if (*s!='&')
      break;
    s++;
  }

  return 0;
}

