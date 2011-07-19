/***************************************************************************
 begin       : Wed Jan 09 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



int AO_Provider__AddHeaders(AB_PROVIDER *pro,
			    AB_USER *u,
			    GWEN_BUFFER *buf) {
  GWEN_TIME *ti;
  const char *s;

  ti=GWEN_CurrentTime();
  assert(ti);

  GWEN_Buffer_AppendString(buf,
			   "OFXHEADER:100\r\n"
			   "DATA:OFXSGML\r\n");
  GWEN_Buffer_AppendString(buf, "VERSION:");
  s=AO_User_GetHeaderVer(u);
  if (!s || !*s)
    s="102";
  GWEN_Buffer_AppendString(buf, s);
  GWEN_Buffer_AppendString(buf, "\r\n");

  s=AO_User_GetSecurityType(u);
  if (!s || !*s)
    s="NONE";
  GWEN_Buffer_AppendString(buf, "SECURITY:");
  GWEN_Buffer_AppendString(buf, s);
  GWEN_Buffer_AppendString(buf, "\r\n");

  GWEN_Buffer_AppendString(buf,
                           "ENCODING:USASCII\r\n"
                           "CHARSET:1252\r\n"
                           "COMPRESSION:NONE\r\n"
                           "OLDFILEUID:NONE\r\n");
  GWEN_Buffer_AppendString(buf, "NEWFILEUID:");
  GWEN_Time_toString(ti, "YYYYMMDDhhmmss.000", buf);
  GWEN_Buffer_AppendString(buf, "\r\n");

  /* header finished */
  GWEN_Buffer_AppendString(buf, "\r\n");

  /* cleanup */
  GWEN_Time_free(ti);

  /* done */
  return 0;
}



int AO_Provider__AddSignOn(AB_PROVIDER *pro,
			   AB_USER *u,
			   GWEN_BUFFER *buf) {
  GWEN_TIME *ti;
  const char *s;
  char userpass[64];

  ti=GWEN_CurrentTime();
  assert(ti);

  GWEN_Buffer_AppendString(buf, "<SIGNONMSGSRQV1>");
  GWEN_Buffer_AppendString(buf, "<SONRQ>");
  GWEN_Buffer_AppendString(buf, "<DTCLIENT>");
  if (AO_User_GetFlags(u) & AO_USER_FLAGS_SEND_SHORT_DATE)
    GWEN_Time_toString(ti, "YYYYMMDDhhmmss", buf);
  else
    GWEN_Time_toString(ti, "YYYYMMDDhhmmss.000", buf);

  s=AB_User_GetUserId(u);
  if (s) {
    GWEN_Buffer_AppendString(buf, "<USERID>");
    GWEN_Buffer_AppendString(buf, s);
    GWEN_Buffer_AppendString(buf, "\r\n");
  }
  else {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Missing user id, should not happen");
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Missing user id, should not happen"));
    return GWEN_ERROR_INTERNAL;
  }

  /* get password */
  userpass[0]=0;
  while (strlen(userpass)<4) {
    GWEN_BUFFER *nbuf;
    int rv;
    char msg[]=I18S("Please enter the password for user %s"
                    "<html>"
                    "Please enter the password for user <b>%s</b>"
                    "</html>");
    char msgbuf[512];

    nbuf=GWEN_Buffer_new(0, 64, 0, 1);
    GWEN_Buffer_AppendString(nbuf, "OFX::userpass::");
    GWEN_Buffer_AppendString(nbuf, s);
    snprintf(msgbuf, sizeof(msgbuf), I18N(msg), s, s);
    rv=GWEN_Gui_GetPassword(0,
			    GWEN_Buffer_GetStart(nbuf),
			    I18N("Enter Password"),
			    msgbuf,
			    userpass,
			    4,
			    sizeof(userpass),
			    0);
    GWEN_Buffer_free(nbuf);
    if (rv<0) {
      memset(userpass, 0, sizeof(userpass));
      GWEN_Time_free(ti);
      return rv;
    }
  } /* while */

  GWEN_Buffer_AppendString(buf, "<USERPASS>");
  GWEN_Buffer_AppendString(buf, userpass);
  GWEN_Buffer_AppendString(buf, "\r\n");
  memset(userpass, 0, sizeof(userpass));

  GWEN_Buffer_AppendString(buf, "<LANGUAGE>ENG");

  /* possibly add FI */
  if (!(AO_User_GetFlags(u) & AO_USER_FLAGS_EMPTY_FID)) {
    if (AO_User_GetFid(u)) {
      GWEN_Buffer_AppendString(buf, "<FI>");
      s=AO_User_GetOrg(u);
      if (s) {
	GWEN_Buffer_AppendString(buf, "<ORG>");
	GWEN_Buffer_AppendString(buf, s);
      }
      s=AO_User_GetFid(u);
      if (s) {
	GWEN_Buffer_AppendString(buf, "<FID>");
	GWEN_Buffer_AppendString(buf, s);
      }
      GWEN_Buffer_AppendString(buf, "</FI>");
    }
  }

  /* add APPID */
  s=AO_User_GetAppId(u);
  if (s==NULL || *s==0)
    s="QWIN";
  GWEN_Buffer_AppendString(buf, "<APPID>");
  GWEN_Buffer_AppendString(buf, s);

  /* add APPVER */
  s=AO_User_GetAppVer(u);
  if (s==NULL || *s==0)
    s="1200";
  GWEN_Buffer_AppendString(buf, "<APPVER>");
  GWEN_Buffer_AppendString(buf, s);

  /* add CLIENTUID, if known */
  s=AO_User_GetClientUid(u);
  if (s && *s) {
    GWEN_Buffer_AppendString(buf, "<CLIENTUID>");
    GWEN_Buffer_AppendString(buf, s);
  }

  /* close elements */
  GWEN_Buffer_AppendString(buf, "</SONRQ>");
  GWEN_Buffer_AppendString(buf, "</SIGNONMSGSRQV1>");

  GWEN_Time_free(ti);

  return 0;
}




int AO_Provider__WrapRequest(AB_PROVIDER *pro,
                             AB_USER *u,
			     const char *mTypeName,
			     const char *tTypeName,
			     GWEN_BUFFER *buf) {
  GWEN_BUFFER *tbuf;
  GWEN_TIME *ti;

  tbuf=GWEN_Buffer_new(0, 512, 0, 1);

  /* begin: msg wrapper */
  GWEN_Buffer_AppendString(tbuf, "<");
  GWEN_Buffer_AppendString(tbuf, mTypeName);
  GWEN_Buffer_AppendString(tbuf, "MSGSRQV1>");

  /* begin: transaction wrapper */
  GWEN_Buffer_AppendString(tbuf, "<");
  GWEN_Buffer_AppendString(tbuf, tTypeName);
  GWEN_Buffer_AppendString(tbuf, "TRNRQ>");
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Buffer_AppendString(tbuf, "<TRNUID>");
  if (AO_User_GetFlags(u) & AO_USER_FLAGS_SEND_SHORT_DATE)
    GWEN_Time_toString(ti, "YYYYMMDDhhmmss", tbuf);
  else
    GWEN_Time_toString(ti, "YYYYMMDDhhmmss.000", tbuf);
  GWEN_Buffer_AppendString(tbuf, "<CLTCOOKIE>1");

  /* append ends of elements to original buffer */
  GWEN_Buffer_AppendString(buf, "</");
  GWEN_Buffer_AppendString(buf, tTypeName);
  GWEN_Buffer_AppendString(buf, "TRNRQ>");

  GWEN_Buffer_AppendString(buf, "</");
  GWEN_Buffer_AppendString(buf, mTypeName);
  GWEN_Buffer_AppendString(buf, "MSGSRQV1>");

  /* go to start of buffer and insert leading elements there */
  GWEN_Buffer_SetPos(buf, 0);
  GWEN_Buffer_InsertBuffer(buf, tbuf);

  /* point to end of buffer */
  GWEN_Buffer_SetPos(buf, GWEN_Buffer_GetUsedBytes(buf));

  /* cleanup */
  GWEN_Time_free(ti);
  GWEN_Buffer_free(tbuf);

  return 0;
}



int AO_Provider__WrapMessage(AB_PROVIDER *pro,
                             AB_USER *u,
			     GWEN_BUFFER *buf) {
  GWEN_BUFFER *tbuf;
  int rv;

  tbuf=GWEN_Buffer_new(0, 1024, 0, 1);

  /* add headers and "<OFX>" */
  rv=AO_Provider__AddHeaders(pro, u, tbuf);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Error adding headers (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_Buffer_AppendString(tbuf, "<OFX>");

  rv=AO_Provider__AddSignOn(pro, u, tbuf);
  if (rv<0) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Error adding signon element (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  /* append end of OFX element to original buffer */
  GWEN_Buffer_AppendString(buf, "</OFX>");

  /* go to start of buffer and insert leading elements there */
  GWEN_Buffer_SetPos(buf, 0);
  GWEN_Buffer_InsertBuffer(buf, tbuf);

  /* point to end of buffer */
  GWEN_Buffer_SetPos(buf, GWEN_Buffer_GetUsedBytes(buf));

  /* cleanup */
  GWEN_Buffer_free(tbuf);

  return 0;
}








#include "r_statements.c"
#include "r_accountinfo.c"


