/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: dialog.c 1109 2007-01-10 14:30:14Z martin $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/* this file is included from dialog.c" */

#include <gwenhywfar/io_tls.h>
#include <gwenhywfar/io_http.h>
#include <gwenhywfar/io_buffered.h>



int AH_Dialog_CreateIoLayer_Https(AH_DIALOG *dlg) {
  int port;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  const GWEN_URL *url;
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER *ioBase;
  int rv;
  GWEN_DB_NODE *db;
  const char *s;

  assert(dlg);

  /* take bank addr from user */
  url=AH_User_GetServerUrl(dlg->dialogOwner);
  if (!url) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User has no valid address settings");
    return GWEN_ERROR_INVALID;
  }

  /* prepare socket layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  io=GWEN_Io_LayerSocket_new(sk);

  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  rv=AH_Dialog__SetAddress(dlg, addr, GWEN_Url_GetServer(url));
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_InetAddr_free(addr);
    return rv;
  }
  port=GWEN_Url_GetPort(url);
  if (port==0)
    port=443;
  GWEN_InetAddr_SetPort(addr, port);
  GWEN_Io_LayerSocket_SetPeerAddr(io, addr);

  /* prepare TLS layer */
  ioBase=io;
  io=GWEN_Io_LayerTls_new(ioBase);
  if (io==NULL) {
    GWEN_InetAddr_free(addr);
    GWEN_Io_Layer_free(ioBase);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Io_Layer_AddFlags(io,
			 GWEN_IO_LAYER_TLS_FLAGS_ALLOW_V1_CA_CRT|
			 GWEN_IO_LAYER_TLS_FLAGS_ADD_TRUSTED_CAS);

  if (AH_User_GetFlags(dlg->dialogOwner) & AH_USER_FLAGS_FORCE_SSL3)
    GWEN_Io_Layer_AddFlags(io, GWEN_IO_LAYER_TLS_FLAGS_FORCE_SSL_V3);

  GWEN_Io_LayerTls_SetRemoteHostName(io, GWEN_Url_GetServer(url));

  /* prepare buffered layer */
  ioBase=io;
  io=GWEN_Io_LayerBuffered_new(ioBase);
  if (io==NULL) {
    GWEN_InetAddr_free(addr);
    GWEN_Io_Layer_free(ioBase);
    return GWEN_ERROR_GENERIC;
  }

  /* prepare HTTP layer */
  ioBase=io;
  io=GWEN_Io_LayerHttp_new(ioBase);
  if (io==NULL) {
    GWEN_InetAddr_free(addr);
    GWEN_Io_Layer_free(ioBase);
    return GWEN_ERROR_GENERIC;
  }

  /* prepare HTTP command line */
  db=GWEN_Io_LayerHttp_GetDbCommandOut(io);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "command", "POST");
  if (AH_User_GetHttpVMajor(dlg->dialogOwner)) {
    char numbuf[32];

    snprintf(numbuf, sizeof(numbuf)-1, "HTTP/%d.%d",
	     AH_User_GetHttpVMajor(dlg->dialogOwner),
	     AH_User_GetHttpVMinor(dlg->dialogOwner));
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "protocol", numbuf);
  }
  else
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "protocol", "HTTP/1.0");

  if (1) {
    GWEN_BUFFER *pbuf;

    pbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Url_toCommandString(url, pbuf);
    if (rv) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(pbuf);
      GWEN_InetAddr_free(addr);
      GWEN_Io_Layer_free(ioBase);
      return rv;
    }
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "url",
			 GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);
  }

  /* prepare HTTP out header */
  db=GWEN_Io_LayerHttp_GetDbHeaderOut(io);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Host", GWEN_Url_GetServer(url));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Pragma", "no-cache");
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Cache-control", "no cache");
  s=AH_User_GetHttpContentType(dlg->dialogOwner);
  if (s)
    /* previous value was: "application/x-www-form-urlencoded" */
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "Content-type", s);

  s=AH_User_GetHttpUserAgent(dlg->dialogOwner);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "User-Agent", s);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Connection", "close");
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Content-length", 0);

  /* now register the layer */
  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not register io layer (%d)", rv);
    GWEN_InetAddr_free(addr);
    GWEN_Io_Layer_free(io);
    return 0;
  }

  dlg->ioLayer=io;
  GWEN_InetAddr_free(addr);
  return 0;
}



int AH_Dialog_SendPacket_Https(AH_DIALOG *dlg,
			       const char *buf, int blen,
			       int timeout) {
  int rv;
  uint32_t fl;

  fl=AH_User_GetFlags(dlg->dialogOwner);

  /* first connect to server */
  GWEN_Gui_ProgressLog(dlg->guiid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Connecting to bank..."));
  rv=GWEN_Io_Layer_ConnectRecursively(dlg->ioLayer, NULL, 0, dlg->guiid, 30000);
  if (rv==GWEN_ERROR_SSL) {
    GWEN_IO_LAYER *ioTls;

    /* try again with alternated SSLv3 flag */
    DBG_NOTICE(AQHBCI_LOGDOMAIN,
	       "Error connecting (%d), retrying", rv);
    GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					dlg->guiid, 2000);
    ioTls=GWEN_Io_Layer_FindBaseLayerByType(dlg->ioLayer, GWEN_IO_LAYER_TLS_TYPE);
    assert(ioTls);

    if (fl & AH_USER_FLAGS_FORCE_SSL3) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Retrying to connect (non-SSLv3)");
      GWEN_Gui_ProgressLog(dlg->guiid,
			   GWEN_LoggerLevel_Notice,
			   I18N("Retrying to connect (non-SSLv3)"));
      GWEN_Io_Layer_SubFlags(ioTls, GWEN_IO_LAYER_TLS_FLAGS_FORCE_SSL_V3);
      rv=GWEN_Io_Layer_ConnectRecursively(dlg->ioLayer, NULL, 0,
					  dlg->guiid, 30000);
      if (rv==0) {
	AH_User_SubFlags(dlg->dialogOwner, AH_USER_FLAGS_FORCE_SSL3);
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Retrying to connect (SSLv3)");
      GWEN_Gui_ProgressLog(dlg->guiid,
			   GWEN_LoggerLevel_Notice,
			   I18N("Retrying to connect (SSLv3)"));
      GWEN_Io_Layer_AddFlags(ioTls, GWEN_IO_LAYER_TLS_FLAGS_FORCE_SSL_V3);
      rv=GWEN_Io_Layer_ConnectRecursively(dlg->ioLayer, NULL, 0,
					  dlg->guiid, 30000);
      if (rv==0) {
	AH_User_AddFlags(dlg->dialogOwner, AH_USER_FLAGS_FORCE_SSL3);
      }
    }
  }

  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not connect to server (%d)", rv);
    GWEN_Gui_ProgressLog(dlg->guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("Could not connect to the bank"));
    GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					dlg->guiid, 2000);
    return rv;
  }
  else {
    GWEN_BUFFER *tbuf;
    GWEN_DB_NODE *db;

    GWEN_Gui_ProgressLog(dlg->guiid,
			 GWEN_LoggerLevel_Notice,
			 I18N("Connected."));
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
	GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer, NULL,
					    GWEN_IO_REQUEST_FLAGS_FORCE,
					    dlg->guiid, 2000);
	return rv;
      }
      GWEN_Buffer_AppendString(tbuf, "\r\n");
    }

    db=GWEN_Io_LayerHttp_GetDbHeaderOut(dlg->ioLayer);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"Content-length", GWEN_Buffer_GetUsedBytes(tbuf));

    /* send BASE64 encoded message */
    rv=GWEN_Io_Layer_WriteBytes(dlg->ioLayer,
				(uint8_t*) GWEN_Buffer_GetStart(tbuf),
				GWEN_Buffer_GetUsedBytes(tbuf),
				GWEN_IO_REQUEST_FLAGS_PACKETBEGIN |
				GWEN_IO_REQUEST_FLAGS_PACKETEND |
				GWEN_IO_REQUEST_FLAGS_FLUSH,
				dlg->guiid, timeout);
    GWEN_Buffer_free(tbuf);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not send message (%d)", rv);
      GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer, NULL,
					  GWEN_IO_REQUEST_FLAGS_FORCE,
					  dlg->guiid, 2000);
      return rv;
    }

    DBG_INFO(AQHBCI_LOGDOMAIN, "Message sent.");
    return 0;
  }
}



int AH_Dialog_RecvMessage_Https_read(AH_DIALOG *dlg, GWEN_BUFFER *buf, int timeout) {
  int rv;
  GWEN_DB_NODE *db;
  int code;

  /* recv packet (this reads the HTTP body) */
  rv=GWEN_Io_Layer_ReadPacketToBuffer(dlg->ioLayer, buf, 0, dlg->guiid, timeout);
  if (rv<0) {
    if (GWEN_Buffer_GetUsedBytes(buf)) {
      /* data received, check for common error codes */
      if (rv==GWEN_ERROR_EOF || (rv==GWEN_ERROR_IO)) {
	DBG_INFO(AQHBCI_LOGDOMAIN,
		 "We received an error, but we still got data, "
		 "so we ignore the error here");
      }
      else {
	DBG_INFO(AQHBCI_LOGDOMAIN, "No message received (%d)", rv);
	return rv;
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No message received (%d)", rv);
      return rv;
    }
  }

  /* check for status and log it */
  db=GWEN_Io_LayerHttp_GetDbStatusIn(dlg->ioLayer);
  code=GWEN_DB_GetIntValue(db, "code", 0, 0);
  if (code) {
    GWEN_BUFFER *lbuf;
    char sbuf[32];
    const char *text;

    lbuf=GWEN_Buffer_new(0, 64, 0, 1);
    GWEN_Buffer_AppendString(lbuf, "HTTP-Status: ");
    snprintf(sbuf, sizeof(sbuf)-1, "%d", code);
    sbuf[sizeof(sbuf)-1]=0;
    GWEN_Buffer_AppendString(lbuf, sbuf);
    text=GWEN_DB_GetCharValue(db, "text", 0, NULL);
    if (text) {
      GWEN_Buffer_AppendString(lbuf, " (");
      GWEN_Buffer_AppendString(lbuf, text);
      GWEN_Buffer_AppendString(lbuf, ")");
    }
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Status: %d (%s)",
	      code, text);
    if (code<200 || code>299) {
      /* response is only ok for continuation (100) code */
      if (code!=100) {
	GWEN_DB_NODE *dbHeaderIn;

        dbHeaderIn=GWEN_Io_LayerHttp_GetDbHeaderIn(dlg->ioLayer);
	DBG_ERROR(AQHBCI_LOGDOMAIN,
		  "Got an error response (%d: %s)",
		  code, text);
	GWEN_Gui_ProgressLog(dlg->guiid,
			     GWEN_LoggerLevel_Error,
			     GWEN_Buffer_GetStart(lbuf));
	GWEN_Buffer_Reset(lbuf);

	if (code==301 || code==303 || code==305 || code==307) {
	  /* moved */
	  if (dbHeaderIn) {
	    const char *s;

	    s=GWEN_DB_GetCharValue(dbHeaderIn, "Location", 0, 0);
	    if (s) {
	      switch(code) {
	      case 301:
	      case 303:
		GWEN_Buffer_AppendString(lbuf,
					 I18N("HTTP: Moved permanently"));
		break;
	      case 305:
		GWEN_Buffer_AppendString(lbuf,
					 I18N("HTTP: Use proxy"));
		break;
	      case 307:
		GWEN_Buffer_AppendString(lbuf,
					 I18N("HTTP: Moved temporarily"));
		break;
	      default:
		GWEN_Buffer_AppendString(lbuf,
					 I18N("HTTP: Moved"));
	      } /* switch */
	      GWEN_Buffer_AppendString(lbuf, " (");
	      GWEN_Buffer_AppendString(lbuf, s);
	      GWEN_Buffer_AppendString(lbuf, ")");

	      GWEN_Gui_ProgressLog(dlg->guiid,
				   GWEN_LoggerLevel_Warning,
                                   GWEN_Buffer_GetStart(lbuf));
	    }
	  }
	}
      }
      else {
	GWEN_Gui_ProgressLog(dlg->guiid,
			     GWEN_LoggerLevel_Notice,
			     GWEN_Buffer_GetStart(lbuf));
      }
    }
    else {
      GWEN_Gui_ProgressLog(dlg->guiid,
			   GWEN_LoggerLevel_Info,
			   GWEN_Buffer_GetStart(lbuf));
    }
    GWEN_Buffer_free(lbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No HTTP status code received");
    GWEN_Gui_ProgressLog(dlg->guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("No HTTP status code received"));
    code=AB_ERROR_NETWORK;
  }

  return code;
}



int AH_Dialog_RecvMessage_Https(AH_DIALOG *dlg, AH_MSG **pMsg, int timeout) {
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
  for (;;) {
    rv=AH_Dialog_RecvMessage_Https_read(dlg, tbuf, timeout);
    if (rv<0 || rv<200 || rv>299) {
      GWEN_Buffer_free(tbuf);
      GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer, NULL,
					  GWEN_IO_REQUEST_FLAGS_FORCE,
					  dlg->guiid, 2000);
      return (rv>0)?AB_ERROR_NETWORK:rv;
    }
    if (rv!=100)
      break;
    GWEN_Buffer_Reset(tbuf);
  }

  /* disconnect */
  GWEN_Gui_ProgressLog(dlg->guiid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Disconnecting from bank..."));
  GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer, NULL,
				      GWEN_IO_REQUEST_FLAGS_FORCE,
				      dlg->guiid, 2000);
  GWEN_Gui_ProgressLog(dlg->guiid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Disconnected."));

  /* optionally decode BASE64 encoded message */
  if (strstr(GWEN_Buffer_GetStart(tbuf), "HNHBK:")==NULL) {
    GWEN_BUFFER *bbuf;

    bbuf=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(tbuf), 0, 1);
    rv=GWEN_Base64_Decode((const unsigned char*) GWEN_Buffer_GetStart(tbuf),
			  GWEN_Buffer_GetUsedBytes(tbuf),
			  bbuf);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not decode BASE64 message (%d)", rv);
      GWEN_Gui_ProgressLog(dlg->guiid,
			   GWEN_LoggerLevel_Error,
                           I18N("Could not BASE64-decode the message"));
      GWEN_Buffer_free(bbuf);
      GWEN_Buffer_free(tbuf);
      GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer, NULL,
					  GWEN_IO_REQUEST_FLAGS_FORCE,
					  dlg->guiid, 2000);
      return rv;
    }
    GWEN_Buffer_free(tbuf);
    tbuf=bbuf;
    if (strstr(GWEN_Buffer_GetStart(tbuf), "HNHBK:")==NULL) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Received message is not HBCI");
      GWEN_Gui_ProgressLog(dlg->guiid,
			   GWEN_LoggerLevel_Error,
                           I18N("Received message is not HBCI"));
      GWEN_Buffer_free(tbuf);
      GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer, NULL,
					  GWEN_IO_REQUEST_FLAGS_FORCE,
					  dlg->guiid, 2000);
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
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_BAD_DATA;
  }
  p1++;

  /* seek to end of size */
  p2=strchr(p1, '+');
  if (p2==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad data (missing '+')");
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_BAD_DATA;
  }

  /* read message size */
  c=*p2;
  *p2=0;
  if (1!=sscanf(p1, "%d", &msgSize)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad size field [%s]", p1);
    GWEN_Buffer_free(tbuf);
    GWEN_Gui_ProgressLog(dlg->guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("Unparsable message received"));
    return GWEN_ERROR_BAD_DATA;
  }
  *p2=c;

  /* check message size */
  if (GWEN_Buffer_GetUsedBytes(tbuf)<msgSize) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad message size (%d<%d)",
	      GWEN_Buffer_GetUsedBytes(tbuf), msgSize);
    GWEN_Buffer_free(tbuf);
    GWEN_Gui_ProgressLog(dlg->guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("Received message was truncated"));
    return GWEN_ERROR_BAD_DATA;
  }

  msg=AH_Msg_new(dlg);
  AH_Msg_SetBuffer(msg, tbuf);

  *pMsg=msg;

  return 0;
}



int AH_Dialog_TestServer_Https(AH_DIALOG *dlg, int timeout) {
  int rv;

  GWEN_Gui_ProgressLog(dlg->guiid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Preparing connection"));
  rv=AH_Dialog_CreateIoLayer_Https(dlg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create io layer (%d)", rv);
    GWEN_Gui_ProgressLog(dlg->guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("Error preparing connection"));
    GWEN_Io_Layer_free(dlg->ioLayer);
    dlg->ioLayer=NULL;
    return AB_ERROR_NETWORK;
  }

  GWEN_Gui_ProgressLog(dlg->guiid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Connecting to bank..."));
  rv=GWEN_Io_Layer_ConnectRecursively(dlg->ioLayer, NULL, 0, dlg->guiid, timeout);
  if (rv<0) {
    uint32_t fl;
    GWEN_IO_LAYER *ioTls;

    /* try again with alternated SSLv3 flag */
    DBG_NOTICE(AQHBCI_LOGDOMAIN,
	       "Error connecting (%d), retrying", rv);
    GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					dlg->guiid, 2000);
    ioTls=GWEN_Io_Layer_FindBaseLayerByType(dlg->ioLayer, GWEN_IO_LAYER_TLS_TYPE);
    assert(ioTls);
    fl=AH_User_GetFlags(dlg->dialogOwner);
    if (fl & AH_USER_FLAGS_FORCE_SSL3) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Retrying to connect (non-SSLv3)");
      GWEN_Gui_ProgressLog(dlg->guiid,
			   GWEN_LoggerLevel_Notice,
			   I18N("Retrying to connect (non-SSLv3)"));
      GWEN_Io_Layer_SubFlags(ioTls, GWEN_IO_LAYER_TLS_FLAGS_FORCE_SSL_V3);
      rv=GWEN_Io_Layer_ConnectRecursively(dlg->ioLayer, NULL, 0,
					  dlg->guiid, 30000);
      if (rv==0) {
	AH_User_SubFlags(dlg->dialogOwner, AH_USER_FLAGS_FORCE_SSL3);
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Retrying to connect (SSLv3)");
      GWEN_Gui_ProgressLog(dlg->guiid,
			   GWEN_LoggerLevel_Notice,
			   I18N("Retrying to connect (SSLv3)"));
      GWEN_Io_Layer_AddFlags(ioTls, GWEN_IO_LAYER_TLS_FLAGS_FORCE_SSL_V3);
      rv=GWEN_Io_Layer_ConnectRecursively(dlg->ioLayer, NULL, 0,
					  dlg->guiid, 30000);
      if (rv==0) {
	AH_User_AddFlags(dlg->dialogOwner, AH_USER_FLAGS_FORCE_SSL3);
      }
    }
  }

  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not connect to server (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					dlg->guiid, 2000);
    GWEN_Io_Layer_free(dlg->ioLayer);
    dlg->ioLayer=NULL;
    return rv;
  }

  rv=GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer, NULL,
                                         0,
					 dlg->guiid, 2000);
  if (rv) {
    GWEN_Io_Layer_DisconnectRecursively(dlg->ioLayer, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					dlg->guiid, 2000);
  }

  GWEN_Io_Layer_free(dlg->ioLayer);
  dlg->ioLayer=NULL;

  return 0;
}



