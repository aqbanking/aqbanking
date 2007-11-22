

#include <gwenhywfar/url.h>
#include <gwenhywfar/io_socket.h>
#include <gwenhywfar/io_tls.h>
#include <gwenhywfar/io_http.h>
#include <gwenhywfar/io_buffered.h>
#include <gwenhywfar/iomanager.h>


#define AO_PROVIDER_TIMEOUT_SEND    60000
#define AO_PROVIDER_TIMEOUT_RECV    60000
#define AO_PROVIDER_TIMEOUT_CONNECT 60000



int AO_Provider__SetAddress(GWEN_INETADDRESS *addr,
			    const char *bankAddr,
			    uint32_t guiid) {
  int err;

  err=GWEN_InetAddr_SetAddress(addr, bankAddr);
  if (err) {
    char dbgbuf[256];

    snprintf(dbgbuf, sizeof(dbgbuf)-1,
	     I18N("Resolving hostname \"%s\" ..."),
	     bankAddr);
    dbgbuf[sizeof(dbgbuf)-1]=0;
    GWEN_Gui_ProgressLog(guiid,
			 GWEN_LoggerLevel_Notice,
			 dbgbuf);
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Resolving hostname \"%s\"",
	     bankAddr);
    err=GWEN_InetAddr_SetName(addr, bankAddr);
    if (err) {
      snprintf(dbgbuf, sizeof(dbgbuf)-1,
	       I18N("Unknown hostname \"%s\""),
	       bankAddr);
      dbgbuf[sizeof(dbgbuf)-1]=0;
      GWEN_Gui_ProgressLog(guiid,
			   GWEN_LoggerLevel_Error,
			   dbgbuf);
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
		"Error resolving hostname \"%s\":",
		bankAddr);
      DBG_ERROR_ERR(AQOFXCONNECT_LOGDOMAIN, err);
      return err;
    }
    else {
      char addrBuf[256];
      int err;

      err=GWEN_InetAddr_GetAddress(addr, addrBuf, sizeof(addrBuf)-1);
      addrBuf[sizeof(addrBuf)-1]=0;
      if (err) {
	DBG_ERROR_ERR(AQOFXCONNECT_LOGDOMAIN, err);
      }
      else {
	snprintf(dbgbuf, sizeof(dbgbuf)-1,
		 I18N("IP address is %s"),
		 addrBuf);
	dbgbuf[sizeof(dbgbuf)-1]=0;
	GWEN_Gui_ProgressLog(guiid,
			     GWEN_LoggerLevel_Notice,
			     dbgbuf);
      }
    }
  }

  return 0;
}



int AO_Provider_CreateConnection(AB_PROVIDER *pro,
				 AB_USER *u,
				 GWEN_IO_LAYER **pIo,
				 uint32_t guiid) {
  int port;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_URL *url;
  GWEN_IO_LAYER *io;
  GWEN_IO_LAYER *ioBase;
  int rv;
  GWEN_DB_NODE *db;
  AO_USER_SERVERTYPE addrType;
  const char *s;

  /* take bank addr from user */
  s=AO_User_GetServerAddr(u);
  if (s==NULL) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Missing server address");
    GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Error,
                         I18N("Missing server address"));
    return GWEN_ERROR_INVALID;
  }
  url=GWEN_Url_fromString(s);
  if (!url) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Bad server address [%s]", s);
    GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Error,
			 I18N("Bad server address"));
    return GWEN_ERROR_INVALID;
  }

  /* prepare socket layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  io=GWEN_Io_LayerSocket_new(sk);

  addrType=AO_User_GetServerType(u);

  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  rv=AO_Provider__SetAddress(addr, GWEN_Url_GetServer(url), guiid);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_InetAddr_free(addr);
    GWEN_Url_free(url);
    return rv;
  }
  port=AO_User_GetServerPort(u);
  if (port<1) {
    /* set default port if none given */
    switch(addrType) {
    case AO_User_ServerTypeHTTP:  port=80; break;
    case AO_User_ServerTypeHTTPS: port=443; break;
    default:
      DBG_WARN(AQOFXCONNECT_LOGDOMAIN,
               "Unknown address type (%d), assuming HTTPS",
               addrType);
      port=443;
      break;
    } /* switch */
  }

  GWEN_InetAddr_SetPort(addr, port);
  GWEN_Io_LayerSocket_SetPeerAddr(io, addr);

  if (addrType==AO_User_ServerTypeHTTPS) {
    /* possibly prepare TLS layer */
    ioBase=io;
    io=GWEN_Io_LayerTls_new(ioBase);
    if (io==NULL) {
      GWEN_InetAddr_free(addr);
      GWEN_Io_Layer_free(ioBase);
      GWEN_Url_free(url);
      return GWEN_ERROR_GENERIC;
    }
    GWEN_Io_Layer_AddFlags(io,
			   GWEN_IO_LAYER_TLS_FLAGS_ALLOW_V1_CA_CRT|
			   GWEN_IO_LAYER_TLS_FLAGS_ADD_TRUSTED_CAS);

    if (AO_User_GetFlags(u) & AO_USER_FLAGS_FORCE_SSL3)
      GWEN_Io_Layer_AddFlags(io, GWEN_IO_LAYER_TLS_FLAGS_FORCE_SSL_V3);

    GWEN_Io_LayerTls_SetRemoteHostName(io, GWEN_Url_GetServer(url));
  }

  /* prepare buffered layer */
  ioBase=io;
  io=GWEN_Io_LayerBuffered_new(ioBase);
  if (io==NULL) {
    GWEN_InetAddr_free(addr);
    GWEN_Io_Layer_free(ioBase);
    GWEN_Url_free(url);
    return GWEN_ERROR_GENERIC;
  }

  /* prepare HTTP layer */
  ioBase=io;
  io=GWEN_Io_LayerHttp_new(ioBase);
  if (io==NULL) {
    GWEN_InetAddr_free(addr);
    GWEN_Io_Layer_free(ioBase);
    GWEN_Url_free(url);
    return GWEN_ERROR_GENERIC;
  }

  /* prepare HTTP command line */
  db=GWEN_Io_LayerHttp_GetDbCommandOut(io);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "command", "POST");
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
      GWEN_Url_free(url);
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
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Content-type",
		       "application/x-ofx");
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "Accept",
		       "*/*, application/x-ofx");
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Connection", "close");
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Content-length", 0);

  /* now register the layer */
  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Could not register io layer (%d)", rv);
    GWEN_InetAddr_free(addr);
    GWEN_Io_Layer_free(io);
    GWEN_Url_free(url);
    return 0;
  }

  *pIo=io;
  GWEN_InetAddr_free(addr);
  GWEN_Url_free(url);
  return 0;
}



int AO_Provider_SendPacket(AB_PROVIDER *pro,
                           GWEN_IO_LAYER *io,
			   const uint8_t *buf, int blen,
			   uint32_t guiid) {
  int rv;

  /* first connect to server */
  GWEN_Gui_ProgressLog(guiid,
		       GWEN_LoggerLevel_Notice,
		       I18N("Connecting to bank..."));
  rv=GWEN_Io_Layer_ConnectRecursively(io, NULL, 0, guiid,
				      AO_PROVIDER_TIMEOUT_CONNECT);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Could not connect to server (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					guiid, 2000);
    return rv;
  }
  else {
    GWEN_DB_NODE *db;

    GWEN_Gui_ProgressLog(guiid,
			 GWEN_LoggerLevel_Notice,
			 I18N("Connected."));

    db=GWEN_Io_LayerHttp_GetDbHeaderOut(io);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"Content-length", blen);

    /* send message */
    rv=GWEN_Io_Layer_WriteBytes(io,
                                buf, blen,
				GWEN_IO_REQUEST_FLAGS_PACKETBEGIN |
				GWEN_IO_REQUEST_FLAGS_PACKETEND |
				GWEN_IO_REQUEST_FLAGS_FLUSH,
				guiid, AO_PROVIDER_TIMEOUT_SEND);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Could not send message (%d)", rv);
      GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					  GWEN_IO_REQUEST_FLAGS_FORCE,
					  guiid, 2000);
      return rv;
    }

    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Message sent.");
    return 0;
  }
}



int AO_Provider_RecvPacket(AB_PROVIDER *pro,
                           GWEN_IO_LAYER *io,
			   GWEN_BUFFER *buf,
			   uint32_t guiid) {
  int rv;
  GWEN_DB_NODE *db;
  int code;

  /* recv packet (this reads the HTTP body) */
  rv=GWEN_Io_Layer_ReadPacketToBuffer(io, buf, 0, guiid,
				      AO_PROVIDER_TIMEOUT_RECV);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "No message received (%d)", rv);
    return rv;
  }

  /* check for status and log it */
  db=GWEN_Io_LayerHttp_GetDbStatusIn(io);
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
    DBG_DEBUG(AQOFXCONNECT_LOGDOMAIN, "Status: %d (%s)",
	      code, text);
    if (code<200 || code>299) {
      /* response is only ok for continuation (100) code */
      if (code!=100) {
	DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
		  "Got an error response (%d: %s)",
		  code, text);
	GWEN_Gui_ProgressLog(guiid,
			     GWEN_LoggerLevel_Error,
			     GWEN_Buffer_GetStart(lbuf));
        /* set error code */
        code=AB_ERROR_NETWORK;
      }
      else {
	GWEN_Gui_ProgressLog(guiid,
			     GWEN_LoggerLevel_Notice,
			     GWEN_Buffer_GetStart(lbuf));
      }
      GWEN_Buffer_free(lbuf);
    }
    else {
      GWEN_Gui_ProgressLog(guiid,
			   GWEN_LoggerLevel_Info,
			   GWEN_Buffer_GetStart(lbuf));
      GWEN_Buffer_free(lbuf);
    }
  }
  else {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "No HTTP status code received");
    GWEN_Gui_ProgressLog(guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("No HTTP status code received"));
    code=AB_ERROR_NETWORK;
  }

  return code;
}




int AO_Provider_SendAndReceive(AB_PROVIDER *pro,
			       AB_USER *u,
                               const uint8_t *p,
                               unsigned int plen,
			       GWEN_BUFFER **pRbuf,
			       uint32_t guiid) {
  AO_PROVIDER *dp;
  GWEN_IO_LAYER *io;
  GWEN_BUFFER *rbuf;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AO_PROVIDER, pro);
  assert(dp);

  if (getenv("AQOFX_LOG_COMM")) {
    FILE *f;

    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Saving response in \"/tmp/ofx.log\" ...");
    GWEN_Gui_ProgressLog(guiid,
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
  rv=AO_Provider_CreateConnection(pro, u, &io, guiid);
  if (rv) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Could not create connection");
    GWEN_Gui_ProgressLog(guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("Could not connect to bank server"));
    return rv;
  }

  /* send message */
  GWEN_Gui_ProgressLog(guiid,
		       GWEN_LoggerLevel_Info,
		       I18N("Sending request..."));
  rv=AO_Provider_SendPacket(pro, io, p, plen, guiid);
  if (rv) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Error %d", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					guiid, 2000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  /* wait for response */
  GWEN_Gui_ProgressLog(guiid,
		       GWEN_LoggerLevel_Info,
		       I18N("Waiting for response..."));
  rbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=AO_Provider_RecvPacket(pro, io, rbuf, guiid);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN,
             "Error receiving packet (%d)", rv);
    GWEN_Gui_ProgressLog(guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("Network error while waiting for response"));
    GWEN_Buffer_free(rbuf);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					guiid, 2000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  /* disconnect (ignore result) */
  rv=GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					 0,
					 guiid, 2000);
  if (rv) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Could not disconnect (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					guiid, 2000);
  }

  /* found a response, transform it */
  GWEN_Gui_ProgressLog(guiid,
		       GWEN_LoggerLevel_Info,
		       I18N("Parsing response..."));

  *pRbuf=rbuf;

  /* release connection ressources */
  GWEN_Io_Layer_free(io);

  if (getenv("AQOFX_LOG_COMM")) {
    FILE *f;

    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN,
	      "Saving response in \"/tmp/ofx.log\" ...");
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







