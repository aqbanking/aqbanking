/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/* this file is included from dialog.c" */



int AH_Dialog_CreateIoLayer_Https(AH_DIALOG *dlg) {
  const GWEN_URL *url;
  int rv;
  GWEN_HTTP_SESSION *sess;
  GWEN_BUFFER *tbuf;
  uint32_t flags;
  const char *s;

  assert(dlg);

  /* take bank addr from user */
  url=AH_User_GetServerUrl(dlg->dialogOwner);
  if (!url) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User has no valid address settings");
    return GWEN_ERROR_INVALID;
  }

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Url_toString(url, tbuf);

  sess=AB_HttpSession_new(AB_User_GetProvider(dlg->dialogOwner),
			  dlg->dialogOwner,
			  GWEN_Buffer_GetStart(tbuf),
			  "https",
			  443);
  GWEN_Buffer_free(tbuf);

  /* setup session */
  flags=AH_User_GetFlags(dlg->dialogOwner);
  if (flags & AH_USER_FLAGS_FORCE_SSL3)
    GWEN_HttpSession_AddFlags(sess, GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3);
  if (flags & AH_USER_FLAGS_TLS_ONLY_SAFE_CIPHERS)
      GWEN_HttpSession_AddFlags(sess, GWEN_HTTP_SESSION_FLAGS_TLS_ONLY_SAFE_CIPHERS);

  GWEN_HttpSession_AddFlags(sess, GWEN_HTTP_SESSION_FLAGS_NO_CACHE);

  s=AH_User_GetHttpContentType(dlg->dialogOwner);
  if (s && *s)
    GWEN_HttpSession_SetHttpContentType(sess, s);

  s=AH_User_GetHttpUserAgent(dlg->dialogOwner);
  if (s && *s)
    GWEN_HttpSession_SetHttpUserAgent(sess, s);

  GWEN_HttpSession_SetHttpVMajor(sess, AH_User_GetHttpVMajor(dlg->dialogOwner));
  GWEN_HttpSession_SetHttpVMinor(sess, AH_User_GetHttpVMinor(dlg->dialogOwner));

  /* init session */
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  dlg->httpSession=sess;
  return 0;
}



int AH_Dialog_Connect_Https(AH_DIALOG *dlg) {
  if (dlg->httpSession==NULL) {
    int rv;

    rv=AH_Dialog_CreateIoLayer_Https(dlg);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int AH_Dialog_Disconnect_Https(AH_DIALOG *dlg) {
  if (dlg->httpSession) {
    GWEN_HttpSession_Fini(dlg->httpSession);
    GWEN_HttpSession_free(dlg->httpSession);
    dlg->httpSession=NULL;
  }

  return 0;
}




int AH_Dialog_SendPacket_Https(AH_DIALOG *dlg, const char *buf, int blen) {
  int rv;
  GWEN_BUFFER *tbuf;
  uint32_t fl;

  fl=AH_User_GetFlags(dlg->dialogOwner);

  /* possibly base64 encode message */
  tbuf=GWEN_Buffer_new(0, blen, 0, 1);
  if (fl & AH_USER_FLAGS_NO_BASE64) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Not encoding message using BASE64");
    GWEN_Buffer_AppendBytes(tbuf, buf, blen);
  }
  else {
    rv=GWEN_Base64_Encode((const unsigned char*)buf, blen, tbuf, 0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not BASE64 encode data (%d)", rv);
      GWEN_Buffer_free(tbuf);
      GWEN_HttpSession_Fini(dlg->httpSession);
      GWEN_HttpSession_free(dlg->httpSession);
      dlg->httpSession=NULL;
      return rv;
    }
    GWEN_Buffer_AppendString(tbuf, "\r\n");
  }

  rv=GWEN_HttpSession_SendPacket(dlg->httpSession, "POST",
				 (const uint8_t*) GWEN_Buffer_GetStart(tbuf),
				 GWEN_Buffer_GetUsedBytes(tbuf));
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_Fini(dlg->httpSession);
    GWEN_HttpSession_free(dlg->httpSession);
    dlg->httpSession=NULL;
    return rv;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Message sent.");
  return 0;
}



int AH_Dialog_RecvMessage_Https(AH_DIALOG *dlg, AH_MSG **pMsg) {
  GWEN_BUFFER *tbuf;
  AH_MSG *msg;
  int rv;
  const char *p;
  unsigned int i;
  char c;
  char *p1;
  char *p2;
  int msgSize;

  tbuf=GWEN_Buffer_new(0, 1024, 0, 1);

  /* read HBCI message */
  rv=GWEN_HttpSession_RecvPacket(dlg->httpSession, tbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_Fini(dlg->httpSession);
    GWEN_HttpSession_free(dlg->httpSession);
    dlg->httpSession=NULL;
    return rv;
  }
  else if (rv==0) {
    /* not a HTTP code */
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_Fini(dlg->httpSession);
    GWEN_HttpSession_free(dlg->httpSession);
    dlg->httpSession=NULL;
    return GWEN_ERROR_INTERNAL;
  }
  else if (!(rv>=200 && rv<=299)) {
    /* not a HTTP: ok code */
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_Fini(dlg->httpSession);
    GWEN_HttpSession_free(dlg->httpSession);
    dlg->httpSession=NULL;
    return rv;
  }

  /* optionally decode BASE64 encoded message */
  if (strstr(GWEN_Buffer_GetStart(tbuf), "HNHBK:")==NULL) {
    GWEN_BUFFER *bbuf;

    bbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(tbuf), 0, 1);
    rv=GWEN_Base64_Decode((const unsigned char*) GWEN_Buffer_GetStart(tbuf),
			  GWEN_Buffer_GetUsedBytes(tbuf),
			  bbuf);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not decode BASE64 message (%d)", rv);
      /* for debugging purposes */
      GWEN_Buffer_Dump(tbuf, 2);
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
                           I18N("Could not BASE64-decode the message"));
      GWEN_Buffer_free(bbuf);
      GWEN_Buffer_free(tbuf);
      GWEN_HttpSession_Fini(dlg->httpSession);
      GWEN_HttpSession_free(dlg->httpSession);
      dlg->httpSession=NULL;
      return rv;
    }
    GWEN_Buffer_free(tbuf);
    tbuf=bbuf;
    if (strstr(GWEN_Buffer_GetStart(tbuf), "HNHBK:")==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Received message is not HBCI");
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
                           I18N("Received message is not HBCI"));
      /* for debugging purposes */
      GWEN_Buffer_Dump(tbuf, 2);
      GWEN_Buffer_free(tbuf);
      GWEN_HttpSession_Fini(dlg->httpSession);
      GWEN_HttpSession_free(dlg->httpSession);
      dlg->httpSession=NULL;
      return rv;
    }
  }

  /* trim response */
  i=GWEN_Buffer_GetUsedBytes(tbuf);
  p=GWEN_Buffer_GetStart(tbuf);
  while(i>0) {
    if (p[i-1]!=0)
      break;
    i--;
  }
  GWEN_Buffer_Crop(tbuf, 0, i);

  /* seek to begin of size */
  p1=strchr(GWEN_Buffer_GetStart(tbuf), '+');
  if (p1==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad data (missing '+')");
    /* for debugging purposes */
    GWEN_Buffer_Dump(tbuf, 2);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_Fini(dlg->httpSession);
    GWEN_HttpSession_free(dlg->httpSession);
    dlg->httpSession=NULL;
    return GWEN_ERROR_BAD_DATA;
  }
  p1++;

  /* seek to end of size */
  p2=strchr(p1, '+');
  if (p2==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad data (missing '+')");
    /* for debugging purposes */
    GWEN_Buffer_Dump(tbuf, 2);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_Fini(dlg->httpSession);
    GWEN_HttpSession_free(dlg->httpSession);
    dlg->httpSession=NULL;
    return GWEN_ERROR_BAD_DATA;
  }

  /* read message size */
  c=*p2;
  *p2=0;
  if (1!=sscanf(p1, "%d", &msgSize)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad size field [%s]", p1);
    /* for debugging purposes */
    GWEN_Buffer_Dump(tbuf, 2);
    GWEN_Buffer_free(tbuf);
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Unparsable message received"));
    GWEN_HttpSession_Fini(dlg->httpSession);
    GWEN_HttpSession_free(dlg->httpSession);
    dlg->httpSession=NULL;
    return GWEN_ERROR_BAD_DATA;
  }
  *p2=c;

  /* check message size */
  if (GWEN_Buffer_GetUsedBytes(tbuf)<msgSize) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad message size (%d<%d)",
              GWEN_Buffer_GetUsedBytes(tbuf), msgSize);
    /* for debugging purposes */
    GWEN_Buffer_Dump(tbuf, 2);
    GWEN_Buffer_free(tbuf);
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Received message was truncated"));
    GWEN_HttpSession_Fini(dlg->httpSession);
    GWEN_HttpSession_free(dlg->httpSession);
    dlg->httpSession=NULL;
    return GWEN_ERROR_BAD_DATA;
  }

  msg=AH_Msg_new(dlg);
  AH_Msg_SetBuffer(msg, tbuf);

  *pMsg=msg;

  return 0;
}



int AH_Dialog_TestServer_Https(AH_DIALOG *dlg) {
  int rv;

  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Notice,
		       I18N("Preparing connection"));
  rv=AH_Dialog_CreateIoLayer_Https(dlg);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create io layer (%d)", rv);
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Error preparing connection"));
    return rv;
  }

  rv=GWEN_HttpSession_ConnectionTest(dlg->httpSession);
  GWEN_HttpSession_Fini(dlg->httpSession);
  GWEN_HttpSession_free(dlg->httpSession);
  dlg->httpSession=NULL;
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



