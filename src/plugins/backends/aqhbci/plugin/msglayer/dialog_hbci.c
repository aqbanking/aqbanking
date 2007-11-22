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

#include <gwenhywfar/io_socket.h>




int AH_Dialog_CreateIoLayer_Hbci(AH_DIALOG *dlg) {
  int port;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  const GWEN_URL *url;
  GWEN_IO_LAYER *io;
  int rv;

  assert(dlg);

  /* take bank addr from user */
  url=AH_User_GetServerUrl(dlg->dialogOwner);
  if (!url) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User has no valid address settings");
    return GWEN_ERROR_INVALID;
  }

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
    port=3000;

  GWEN_InetAddr_SetPort(addr, port);
  GWEN_Io_LayerSocket_SetPeerAddr(io, addr);

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Could not register io layer (%d)", rv);
    GWEN_InetAddr_free(addr);
    GWEN_Io_Layer_free(io);
    return 0;
  }

  dlg->ioLayer=io;
  GWEN_InetAddr_free(addr);
  return 0;
}



int AH_Dialog_SendPacket_Hbci(AH_DIALOG *dlg,
			      const char *buf, int blen,
			      int timeout) {
  int rv;

  rv=GWEN_Io_Layer_WriteBytes(dlg->ioLayer,
			      (const uint8_t*)buf,
			      blen,
			      GWEN_IO_REQUEST_FLAGS_WRITEALL |
			      GWEN_IO_REQUEST_FLAGS_FLUSH,
			      dlg->guiid,
			      timeout);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AH_Dialog_RecvMessage_Hbci(AH_DIALOG *dlg, AH_MSG **pMsg, int timeout) {
  AH_MSG *msg;
  GWEN_BUFFER *tbuf;
  int rv;
  char *p1;
  char *p2;
  char header[32];
  int msgSize;

  assert(dlg->ioLayer);

  /* receive header */
  rv=GWEN_Io_Layer_ReadBytes(dlg->ioLayer,
			     (uint8_t*)header,
			     sizeof(header)-1,
			     GWEN_IO_REQUEST_FLAGS_READALL,
			     dlg->guiid,
			     timeout);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error reading header (%d)", rv);
    return rv;
  }
  else if (rv!=sizeof(header)-1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "EOF met");
    return GWEN_ERROR_EOF;
  }
  header[rv]=0;

  /* check for beginning of HBCI message */
  if (strncmp(header, "HNHBK:", 6)!=0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Message does not start with HNHBK");
    return GWEN_ERROR_BAD_DATA;
  }

  /* seek to begin of size */
  p1=strchr(header, '+');
  if (p1==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad data (missing '+')");
    return GWEN_ERROR_BAD_DATA;
  }
  p1++;

  /* seek to end of size */
  p2=strchr(p1, '+');
  if (p2==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad data (missing '+')");
    GWEN_Text_DumpString(header, 21, stderr, 2);
    return GWEN_ERROR_BAD_DATA;
  }

  /* save received bytes */
  tbuf=GWEN_Buffer_new(0, 512, 0, 1);
  GWEN_Buffer_AppendBytes(tbuf, header, rv);

  /* read message size */
  *p2=0;
  if (1!=sscanf(p1, "%d", &msgSize)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad size field [%s]", p1);
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_BAD_DATA;
  }

  /* subtract bytes already received */
  msgSize-=rv;

  /* make room for the rest of the message */
  GWEN_Buffer_AllocRoom(tbuf, msgSize);

  /* receive rest of the message */
  rv=GWEN_Io_Layer_ReadBytes(dlg->ioLayer,
			     (uint8_t*)GWEN_Buffer_GetPosPointer(tbuf),
                             msgSize,
			     GWEN_IO_REQUEST_FLAGS_READALL,
			     dlg->guiid,
			     timeout);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error reading message (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  else if (rv!=msgSize) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "EOF met");
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_EOF;
  }

  /* advance pointer and counter in buffer */
  GWEN_Buffer_IncrementPos(tbuf, msgSize);
  GWEN_Buffer_AdjustUsedBytes(tbuf);

  /* create message and assign the buffer */
  msg=AH_Msg_new(dlg);
  AH_Msg_SetBuffer(msg, tbuf);

  /* done */
  *pMsg=msg;
  return 0;
}




