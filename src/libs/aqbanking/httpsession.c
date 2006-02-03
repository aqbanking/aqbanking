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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "httpsession_p.h"
#include "i18n_l.h"
#include "banking_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/nl_socket.h>
#include <gwenhywfar/nl_ssl.h>
#include <gwenhywfar/nl_http.h>
#include <gwenhywfar/nl_file.h>
#include <gwenhywfar/nl_log.h>
#include <gwenhywfar/net2.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>

#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif


#define ENABLE_LOG
//#define USE_FILE
#define DIALOG_LOGFILE_BASE "dialoglog"


GWEN_INHERIT_FUNCTIONS(AB_HTTPSESSION)
GWEN_LIST_FUNCTIONS(AB_HTTPSESSION, AB_HttpSession)



AB_HTTPSESSION *AB_HttpSession_new(AB_PROVIDER *pro, AB_USER *u) {
  AB_HTTPSESSION *hc;

  assert(pro);
  assert(u);

  GWEN_NEW_OBJECT(AB_HTTPSESSION, hc);
  GWEN_INHERIT_INIT(AB_HTTPSESSION, hc);
  GWEN_LIST_INIT(AB_HTTPSESSION, hc);

  hc->provider=pro;
  hc->user=u;

  hc->connectTimeout=AB_HTTPSESSION_DEFAULT_CONNECT_TIMEOUT;
  hc->transferTimeout=AB_HTTPSESSION_DEFAULT_TRANSFER_TIMEOUT;
  hc->httpVersion=GWEN_NetLayerHttpVersion_1_1;

  return hc;
}



void AB_HttpSession_free(AB_HTTPSESSION *hc) {
  if (hc) {
    GWEN_LIST_FINI(AB_HTTPSESSION, hc);
    GWEN_INHERIT_FINI(AB_HTTPSESSION, hc);
    free(hc->logFolder);
    GWEN_NetLayer_free(hc->netLayer);
    GWEN_FREE_OBJECT(hc);
  }
}



AB_PROVIDER *AB_HttpSession_GetProvider(const AB_HTTPSESSION *hc) {
  assert(hc);
  return hc->provider;
}



GWEN_TYPE_UINT32 AB_HttpSession_GetSessionId(const AB_HTTPSESSION *hc) {
  assert(hc);
  return hc->sessionId;
}



GWEN_NETLAYER_HTTP_VERSION
AB_HttpSession_GetHttpVersion(const AB_HTTPSESSION *hc) {
  assert(hc);
  return hc->httpVersion;
}



void AB_HttpSession_SetHttpVersion(AB_HTTPSESSION *hc,
                                   GWEN_NETLAYER_HTTP_VERSION v) {
  assert(hc);
  hc->httpVersion=v;
}



int AB_HttpSession_GetConnectTimeout(const AB_HTTPSESSION *hc) {
  assert(hc);
  return hc->connectTimeout;
}



void AB_HttpSession_SetConnectTimeout(AB_HTTPSESSION *hc, int i) {
  assert(hc);
  hc->connectTimeout=i;
}



int AB_HttpSession_GetTransferTimeout(const AB_HTTPSESSION *hc) {
  assert(hc);
  return hc->transferTimeout;
}



void AB_HttpSession_SetTransferTimeout(AB_HTTPSESSION *hc, int i) {
  assert(hc);
  hc->transferTimeout=i;
}



GWEN_TYPE_UINT32 AB_HttpSession_GetFlags(const AB_HTTPSESSION *hc) {
  assert(hc);
  return hc->flags;
}



void AB_HttpSession_SetFlags(AB_HTTPSESSION *hc, GWEN_TYPE_UINT32 f) {
  assert(hc);
  hc->flags=f;
}



void AB_HttpSession_AddFlags(AB_HTTPSESSION *hc, GWEN_TYPE_UINT32 f) {
  assert(hc);
  hc->flags|=f;
}



void AB_HttpSession_SubFlags(AB_HTTPSESSION *hc, GWEN_TYPE_UINT32 f) {
  assert(hc);
  hc->flags&=~f;
}



AB_USER *AB_HttpSession_GetUser(const AB_HTTPSESSION *hc) {
  assert(hc);
  return hc->user;
}



void AB_HttpSession__AddPeerCertFolder(AB_HTTPSESSION *hc, GWEN_BUFFER *nbuf){
  const char *s;

  AB_Provider_GetUserDataDir(hc->provider, nbuf);
  GWEN_Buffer_AppendString(nbuf, "/banks/");
  s=AB_User_GetCountry(hc->user);
  if (!s || !*s)
    s="ch";
  GWEN_Buffer_AppendString(nbuf, s);
  GWEN_Buffer_AppendByte(nbuf, '/');
  s=AB_User_GetBankCode(hc->user);
  if (!s || !*s)
    s="none";
  GWEN_Buffer_AppendString(nbuf, s);
  GWEN_Buffer_AppendByte(nbuf, '/');
  GWEN_Buffer_AppendString(nbuf, "/certs");
}



int AB_HttpSession__EnsureConnection(AB_HTTPSESSION *hc, const GWEN_URL *url) {
  int rv;
  GWEN_NETLAYER_STATUS st;

  assert(hc);
  /* check whether there is a connection */
  if (hc->netLayer) {
    if (GWEN_NetLayer_GetStatus(hc->netLayer)==
        GWEN_NetLayerStatus_Connected) {
      if (hc->flags & AB_HTTPSESSION_FLAGS_REUSE) {
        const char *s1, *s2;
        int match=0;

        assert(hc->lastUrl);
        s1=GWEN_Url_GetServer(url);
        s2=GWEN_Url_GetServer(hc->lastUrl);
        if (s1 && s2)
          match=(strcasecmp(s1, s2)==0);

        if (!match) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "Different server");
          GWEN_NetLayer_Disconnect_Wait(hc->netLayer, 2);
          GWEN_NetLayer_free(hc->netLayer);
        hc->netLayer=0;
        }
      }
      else {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Not reusing current connection");
        GWEN_NetLayer_Disconnect_Wait(hc->netLayer, 2);
        GWEN_NetLayer_free(hc->netLayer);
        hc->netLayer=0;
      }
    }
    else {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Not connected");
      GWEN_NetLayer_Disconnect_Wait(hc->netLayer, 2);
      GWEN_NetLayer_free(hc->netLayer);
      hc->netLayer=0;
    }
  }

  if (!hc->netLayer) {
    GWEN_NETLAYER *baseLayer, *nl;
#ifdef USE_FILE
    int fdRead;
    int fdWrite;
#else
    GWEN_SOCKET *sk;
    GWEN_INETADDRESS *addr;
    const char *s;
    GWEN_ERRORCODE err;
    GWEN_BUFFER *pbuf;
#endif

    /* need to create and connect a new netLayer */
    DBG_INFO(AQBANKING_LOGDOMAIN, "Creating connection");
    /* lowest layer */
#ifdef USE_FILE
    fdRead=open(DIALOG_LOGFILE_BASE"-1.read", O_RDONLY);
    fdWrite=open(DIALOG_LOGFILE_BASE".out", O_WRONLY | O_CREAT);
    baseLayer=GWEN_NetLayerFile_new(fdRead, fdWrite, 1);
#else
    sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
    baseLayer=GWEN_NetLayerSocket_new(sk, 1);
    addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
    err=GWEN_InetAddr_SetAddress(addr, GWEN_Url_GetServer(url));
    if (!GWEN_Error_IsOk(err)) {
      char dbgbuf[256];

      snprintf(dbgbuf, sizeof(dbgbuf)-1,
               I18N("Resolving hostname \"%s\" ..."),
               GWEN_Url_GetServer(url));
      dbgbuf[sizeof(dbgbuf)-1]=0;
      AB_Banking_ProgressLog(AB_Provider_GetBanking(hc->provider), 0,
                             AB_Banking_LogLevelNotice,
                             dbgbuf);
      DBG_INFO(AQBANKING_LOGDOMAIN, "Resolving hostname \"%s\"",
               GWEN_Url_GetServer(url));

      err=GWEN_InetAddr_SetName(addr, GWEN_Url_GetServer(url));
      if (!GWEN_Error_IsOk(err)) {
        snprintf(dbgbuf, sizeof(dbgbuf)-1,
                 I18N("Unknown hostname \"%s\""),
                 GWEN_Url_GetServer(url));
        dbgbuf[sizeof(dbgbuf)-1]=0;
        AB_Banking_ProgressLog(AB_Provider_GetBanking(hc->provider), 0,
                               AB_Banking_LogLevelError,
                               dbgbuf);
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Error resolving hostname \"%s\":",
                  GWEN_Url_GetServer(url));
        DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
        GWEN_NetLayer_free(hc->netLayer);
        hc->netLayer=0;
        return AB_ERROR_NETWORK;
      }
      else {
        char addrBuf[256];
        GWEN_ERRORCODE err;
  
        err=GWEN_InetAddr_GetAddress(addr, addrBuf, sizeof(addrBuf)-1);
        addrBuf[sizeof(addrBuf)-1]=0;
        if (!GWEN_Error_IsOk(err)) {
          DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
        }
        else {
          snprintf(dbgbuf, sizeof(dbgbuf)-1,
                   I18N("IP address is %s"),
                   addrBuf);
          dbgbuf[sizeof(dbgbuf)-1]=0;
          AB_Banking_ProgressLog(AB_Provider_GetBanking(hc->provider), 0,
                                 AB_Banking_LogLevelNotice,
                                 dbgbuf);
        }

      }
    }
    GWEN_InetAddr_SetPort(addr, GWEN_Url_GetPort(url));
    GWEN_NetLayer_SetPeerAddr(baseLayer, addr);
    GWEN_InetAddr_free(addr);

    /* eventually create SSL layer above it */
    s=GWEN_Url_GetProtocol(url);
    if (s && strcasecmp(s, "https")==0) {
      GWEN_NETLAYER *nlssl;
      GWEN_BUFFER *nbuf;

      nbuf=GWEN_Buffer_new(0, 64, 0, 1);
      AB_HttpSession__AddPeerCertFolder(hc, nbuf);

      nlssl=GWEN_NetLayerSsl_new(baseLayer,
                                 GWEN_Buffer_GetStart(nbuf),
                                 GWEN_Buffer_GetStart(nbuf),
                                 0, 0, 1);
      GWEN_Buffer_free(nbuf);
      GWEN_NetLayer_free(baseLayer);
      //GWEN_NetLayerSsl_SetAskAddCertFn(nlssl, _nlAskAddCert, 0);
      GWEN_NetLayerSsl_SetAskAddCertFn(nlssl,
                                       AB_Banking_AskAddCert,
                                       AB_Provider_GetBanking(hc->provider));
      baseLayer=nlssl;
    }
#endif

#ifdef ENABLE_LOG
    DBG_NOTICE(AQBANKING_LOGDOMAIN, "Creating loglayer");
    pbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(pbuf, hc->logFolder);
    GWEN_Buffer_AppendString(pbuf, DIRSEP "netlog");
    nl=GWEN_NetLayerLog_new(baseLayer, GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);
    GWEN_NetLayer_free(baseLayer);
    baseLayer=nl;
#endif

    nl=GWEN_NetLayerHttp_new(baseLayer);
    GWEN_NetLayer_free(baseLayer);

    GWEN_NetLayerHttp_SetHttpVersion(nl, hc->httpVersion);

    GWEN_Net_AddConnectionToPool(nl);
    hc->netLayer=nl;
    GWEN_Url_free(hc->lastUrl);
    hc->lastUrl=GWEN_Url_dup(url);
  }

  /* eventually connect */
  st=GWEN_NetLayer_GetStatus(hc->netLayer);
  DBG_INFO(AQBANKING_LOGDOMAIN,
	   "NetLayer status: %s",
	   GWEN_NetLayerStatus_toString(st));
  if (st==GWEN_NetLayerStatus_Disconnected) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "Changing status from disconnected to unconnected");
    st=GWEN_NetLayerStatus_Unconnected;
    GWEN_NetLayer_SetStatus(hc->netLayer, st);
  }
  if (st==GWEN_NetLayerStatus_Unconnected) {
    int timeout;

    AB_Banking_ProgressLog(AB_Provider_GetBanking(hc->provider), 0,
                           AB_Banking_LogLevelNotice,
                           I18N("Connecting"));

    DBG_INFO(AQBANKING_LOGDOMAIN,
             "Not connected, connecting now");

    timeout=hc->connectTimeout;
    rv=GWEN_NetLayer_Connect_Wait(hc->netLayer, timeout);
    if (rv) {
      fprintf(stderr, "ERROR: Could not connect (%d)\n", rv);
      return AB_ERROR_NETWORK;
    }
    DBG_INFO(AQBANKING_LOGDOMAIN, "Connected");
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Already connected");
  }

  return 0;
}



int AB_HttpSession___SendRequest(AB_HTTPSESSION *hc,
                                 const char *command,
                                 const GWEN_URL *url,
                                 GWEN_DB_NODE *dbHeader,
                                 const char *pBody,
                                 int lBody,
                                 GWEN_BUFFER *buf) {
  GWEN_BUFFEREDIO *bio;
  int rv;
  GWEN_ERRORCODE err;

  bio=GWEN_BufferedIO_Buffer2_new(buf, 0);
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);

  rv=AB_HttpSession__EnsureConnection(hc, url);
  if (rv) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return rv;
  }

  rv=GWEN_NetLayerHttp_Request(hc->netLayer,
                               command, url, dbHeader,
			       pBody, lBody, bio);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return AB_ERROR_NETWORK;
  }

  err=GWEN_BufferedIO_Flush(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return AB_ERROR_NETWORK;
  }

  GWEN_BufferedIO_free(bio);
  return rv;
}



int AB_HttpSession_SendRequest(AB_HTTPSESSION *hc,
                            const char *command,
                            const GWEN_URL *url,
                            GWEN_DB_NODE *dbHeader,
                            const char *pBody,
                            int lBody,
                            GWEN_BUFFER *buf) {
  int rv;

  AB_Banking_ProgressLog(AB_Provider_GetBanking(hc->provider), 0,
                         AB_Banking_LogLevelInfo,
                         I18N("Sending request"));

  rv=AB_HttpSession___SendRequest(hc, command, url, dbHeader, pBody, lBody, buf);
  if (rv==0 || (rv>=200 && rv<300))
    return rv;

  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (rv!=301 && rv!=302 && rv!=303 && rv!=307) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Bad HTTP response (code: %d)", rv);
    return AB_ERROR_NETWORK;
  }
  else {
    if (hc->flags & AB_HTTPSESSION_FLAGS_ALLOW_REDIRECT) {
      char txtBuffer[1024];
      GWEN_BUFFER *url1;
      GWEN_BUFFER *url2;
      GWEN_NETLAYER *nl;
      GWEN_DB_NODE *dbInHeader;
      const char *s;
      GWEN_URL *newUrl;
      const char *qRedir=
        I18N_NOOP("The server wants to redirect us from\n"
                  "\"%s\"\n"
                  "to\n"
                  "\"%s\".\n"
                  "Do you accept this redirection?"
                  "<html>"
                  "<p>"
                  "The server wants to <b>redirect</b> us from "
                  "<font color=\"red\">%s</font>"
                  "to "
                  "<font color=\"red\">%s</font>"
                  "</p>"
                  "<p>"
                  "Do you accept this redirection?"
                  "</p>"
                  "</html>");
  
      nl=hc->netLayer;
      assert(nl);
      if (strcasecmp(GWEN_NetLayer_GetTypeName(nl), GWEN_NL_HTTP_NAME)!=0)
        nl=GWEN_NetLayer_FindBaseLayer(nl, GWEN_NL_HTTP_NAME);
      assert(nl);
      dbInHeader=GWEN_NetLayerHttp_GetInHeader(nl);
      assert(dbInHeader);
      s=GWEN_DB_GetCharValue(dbInHeader, "Location", 0, 0);
      if (!s) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Bad HTTP response (no Location header)");
        return AB_ERROR_NETWORK;
      }
    
      newUrl=GWEN_Url_fromString(s);
      if (!newUrl) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Bad HTTP response (bad Location header \"%s\")", s);
        return AB_ERROR_NETWORK;
      }
  
      url1=GWEN_Buffer_new(0, 256, 0, 1);
      url2=GWEN_Buffer_new(0, 256, 0, 1);
      GWEN_Url_toString(url, url1);
      GWEN_Url_toString(newUrl, url2);
  
      snprintf(txtBuffer, sizeof(txtBuffer)-1,
               I18N(qRedir),
               GWEN_Buffer_GetStart(url1),
               GWEN_Buffer_GetStart(url2),
               GWEN_Buffer_GetStart(url1),
               GWEN_Buffer_GetStart(url2));
      txtBuffer[sizeof(txtBuffer)-1]=0;
      GWEN_Buffer_free(url2);
      GWEN_Buffer_free(url1);
  
      rv=AB_Banking_MessageBox(AB_Provider_GetBanking(hc->provider),
                               AB_BANKING_MSG_FLAGS_TYPE_WARN |
                               AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS,
                               I18N("Redirection Requested"),
                               txtBuffer,
                               I18N("Yes"), I18N("Abort"), 0);
      if (rv!=1) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "User does not follow redirection");
        GWEN_Url_free(newUrl);
        return AB_ERROR_NETWORK;
      }
  
      /* resend request, now using the new URL */
      GWEN_Buffer_Reset(buf);
      rv=AB_HttpSession___SendRequest(hc, command, newUrl, dbHeader,
                                   pBody, lBody, buf);
      GWEN_Url_free(newUrl);
      return rv;
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Redirection requested but not allowed");
      return AB_ERROR_SECURITY;
    }
  }
}



int AB_HttpSession_Open(AB_HTTPSESSION *hc) {
  int rv;
  GWEN_BUFFER *pbuf;
  GWEN_TYPE_UINT32 bpos;
  char pwbuf[16];

  /* reset */
  hc->sessionId=AB_User_GetLastSessionId(hc->user)+1;
  AB_User_SetLastSessionId(hc->user, hc->sessionId);

  AB_Banking_ProgressLog(AB_Provider_GetBanking(hc->provider), 0,
                         AB_Banking_LogLevelNotice,
                         I18N("Opening session"));

  /* create path */
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AB_Provider_GetUserDataDir(hc->provider, pbuf);
  GWEN_Buffer_AppendString(pbuf, DIRSEP "users" DIRSEP);
  GWEN_Text_EscapeToBufferTolerant(AB_User_GetUserId(hc->user), pbuf);
  snprintf(pwbuf, sizeof(pwbuf), "session-%d", hc->sessionId);
  GWEN_Buffer_AppendString(pbuf, DIRSEP);
  GWEN_Buffer_AppendString(pbuf, pwbuf);
  GWEN_Buffer_AppendString(pbuf, DIRSEP);
  bpos=GWEN_Buffer_GetPos(pbuf);

  /* set logname */
  GWEN_Buffer_AppendString(pbuf, "logs");
  DBG_INFO(AQBANKING_LOGDOMAIN, "LogFolder: %s",
           GWEN_Buffer_GetStart(pbuf));
  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(pbuf),
                            GWEN_PATH_FLAGS_CHECKROOT |
                            GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not create path \"%s\"",
	      GWEN_Buffer_GetStart(pbuf));
    GWEN_Buffer_free(pbuf);
    return AB_ERROR_INVALID;
  }
  free(hc->logFolder);
  hc->logFolder=strdup(GWEN_Buffer_GetStart(pbuf));
  GWEN_Buffer_free(pbuf);

  DBG_NOTICE(AQBANKING_LOGDOMAIN, "Session started");
  return 0;
}



void AB_HttpSession_Close(AB_HTTPSESSION *hc) {
  assert(hc);
  AB_Banking_ProgressLog(AB_Provider_GetBanking(hc->provider), 0,
                         AB_Banking_LogLevelNotice,
                         I18N("Closing session"));
  if (hc->netLayer) {
    GWEN_NetLayer_Disconnect(hc->netLayer);
    GWEN_NetLayer_free(hc->netLayer);
    hc->netLayer=0;
    GWEN_Url_free(hc->lastUrl);
    hc->lastUrl=0;
  }
}





