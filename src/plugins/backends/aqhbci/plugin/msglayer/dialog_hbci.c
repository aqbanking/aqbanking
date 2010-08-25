/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/* this file is included from dialog.c" */




int AH_Dialog_CreateIoLayer_Hbci(AH_DIALOG *dlg) {
  const GWEN_URL *url;
  GWEN_SYNCIO *sio;
  int rv;
  GWEN_BUFFER *tbuf;

  assert(dlg);

  /* take bank addr from user */
  url=AH_User_GetServerUrl(dlg->dialogOwner);
  if (!url) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User has no valid address settings");
    return GWEN_ERROR_INVALID;
  }

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Url_toString(url, tbuf);
  rv=GWEN_Gui_GetSyncIo(GWEN_Buffer_GetStart(tbuf), "hbci", 3000, &sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(tbuf);

  dlg->ioLayer=sio;
  return 0;
}



int AH_Dialog_Connect_Hbci(AH_DIALOG *dlg) {
  if (dlg->ioLayer==NULL) {
    int rv;

    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Notice,
			 I18N("Connecting to bank..."));

    rv=AH_Dialog_CreateIoLayer_Hbci(dlg);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    do {
      rv=GWEN_SyncIo_Connect(dlg->ioLayer);
    } while (rv==GWEN_ERROR_INTERRUPTED);

    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"Could not connect to bank (%d)", rv);
      GWEN_SyncIo_free(dlg->ioLayer);
      dlg->ioLayer=NULL;
      GWEN_Gui_ProgressLog2(0,
			    GWEN_LoggerLevel_Error,
			    I18N("Could not connect (%d)"),
			    rv);
      return rv;
    }
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Notice,
			 I18N("Connected."));
  }
  return 0;
}



int AH_Dialog_Disconnect_Hbci(AH_DIALOG *dlg) {
  if (dlg->ioLayer) {
    int rv;

    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Notice,
			 I18N("Disconnecting from bank..."));

    do {
      rv=GWEN_SyncIo_Disconnect(dlg->ioLayer);
    } while (rv==GWEN_ERROR_INTERRUPTED);

    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Notice,
			 I18N("Disconnected."));
    GWEN_SyncIo_free(dlg->ioLayer);
    dlg->ioLayer=NULL;
  }
  return 0;
}



int AH_Dialog_SendPacket_Hbci(AH_DIALOG *dlg, const char *buf, int blen) {
  int rv;

  rv=GWEN_SyncIo_WriteForced(dlg->ioLayer, (const uint8_t*)buf, blen);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AH_Dialog_RecvMessage_Hbci(AH_DIALOG *dlg, AH_MSG **pMsg) {
  AH_MSG *msg;
  GWEN_BUFFER *tbuf;
  int rv;
  char *p1;
  char *p2;
  char header[32];
  int msgSize;

  assert(dlg->ioLayer);

  /* receive header */
  rv=GWEN_SyncIo_ReadForced(dlg->ioLayer, (uint8_t*)header, sizeof(header)-1);
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
    GWEN_Text_DumpString(header, 21, 2);
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
  rv=GWEN_SyncIo_ReadForced(dlg->ioLayer,
			    (uint8_t*)GWEN_Buffer_GetPosPointer(tbuf),
			    msgSize);
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




