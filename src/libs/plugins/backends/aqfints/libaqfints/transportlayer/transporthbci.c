/***************************************************************************
 begin       : Wed Jul 31 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "transporthbci_p.h"


#include "libaqfints/aqfints.h"

#include <gwenhywfar/gui.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/debug.h>

#define I18N



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static void GWENHYWFAR_CB freeData(void *bp, void *p);
static int transportConnect(AQFINTS_TRANSPORT *trans);
static int transportDisconnect(AQFINTS_TRANSPORT *trans);
static int transportSendMessage(AQFINTS_TRANSPORT *trans, const char *ptrBuffer, int lenBuffer);
static int transportReceiveMessage(AQFINTS_TRANSPORT *trans, GWEN_BUFFER *buffer);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



GWEN_INHERIT(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_HBCI)


AQFINTS_TRANSPORT *AQFINTS_TransportHbci_new(const char *url)
{
  AQFINTS_TRANSPORT *trans;
  AQFINTS_TRANSPORT_HBCI *xtrans;

  trans=AQFINTS_Transport_new();
  GWEN_NEW_OBJECT(AQFINTS_TRANSPORT_HBCI, xtrans);
  GWEN_INHERIT_SETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_HBCI, trans, xtrans, freeData);

  AQFINTS_Transport_SetUrl(trans, url);

  /* set virtual functions */
  AQFINTS_Transport_SetConnectFn(trans, transportConnect);
  AQFINTS_Transport_SetDisconnectFn(trans, transportDisconnect);
  AQFINTS_Transport_SetSendMessageFn(trans, transportSendMessage);
  AQFINTS_Transport_SetReceiveMessageFn(trans, transportReceiveMessage);

  return trans;
}



AQFINTS_TRANSPORT *AQFINTS_TransportHbci_fromSyncIo(GWEN_SYNCIO* sio)
{
  AQFINTS_TRANSPORT *trans;
  AQFINTS_TRANSPORT_HBCI *xtrans;

  trans=AQFINTS_Transport_new();
  GWEN_NEW_OBJECT(AQFINTS_TRANSPORT_HBCI, xtrans);
  GWEN_INHERIT_SETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_HBCI, trans, xtrans, freeData);

  /* set virtual functions */
  AQFINTS_Transport_SetConnectFn(trans, transportConnect);
  AQFINTS_Transport_SetDisconnectFn(trans, transportDisconnect);
  AQFINTS_Transport_SetSendMessageFn(trans, transportSendMessage);
  AQFINTS_Transport_SetReceiveMessageFn(trans, transportReceiveMessage);

  xtrans->ioLayer=sio;

  return trans;
}



void GWENHYWFAR_CB freeData(void *bp, void *p)
{
  AQFINTS_TRANSPORT_HBCI *xtrans;

  xtrans=(AQFINTS_TRANSPORT_HBCI *) p;
  GWEN_SyncIo_free(xtrans->ioLayer);
  GWEN_FREE_OBJECT(xtrans);
}



int createIoLayer(AQFINTS_TRANSPORT *trans)
{
  AQFINTS_TRANSPORT_HBCI *xtrans;
  GWEN_SYNCIO *sio;
  int rv;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_HBCI, trans);
  assert(xtrans);

  rv=GWEN_Gui_GetSyncIo(AQFINTS_Transport_GetUrl(trans), "hbci", 3000, &sio);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return GWEN_ERROR_GENERIC;
  }

  xtrans->ioLayer=sio;
  return 0;
}



int transportConnect(AQFINTS_TRANSPORT *trans)
{
  AQFINTS_TRANSPORT_HBCI *xtrans;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_HBCI, trans);
  assert(xtrans);

  if (xtrans->ioLayer==NULL) {
    int rv;

    rv=createIoLayer(trans);
    if (rv<0) {
      DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    do {
      rv=GWEN_SyncIo_Connect(xtrans->ioLayer);
    }
    while (rv==GWEN_ERROR_INTERRUPTED);
    if (rv<0) {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "Could not connect to bank (%d)", rv);
      GWEN_SyncIo_free(xtrans->ioLayer);
      xtrans->ioLayer=NULL;
      GWEN_Gui_ProgressLog2(0,
                            GWEN_LoggerLevel_Error,
                            I18N("Could not connect (%d)"),
                            rv);
      return rv;
    }
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Notice,
                         I18N("Connected."));
    AQFINTS_Transport_AddRuntimeFlags(trans, AQFINTS_TRANSPORT_RTFLAGS_CONNECTED);
  }
  return 0;
}



int transportDisconnect(AQFINTS_TRANSPORT *trans)
{
  AQFINTS_TRANSPORT_HBCI *xtrans;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_HBCI, trans);
  assert(xtrans);

  if (xtrans->ioLayer) {
    int rv;

    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Notice,
                         I18N("Disconnecting from bank..."));

    do {
      rv=GWEN_SyncIo_Disconnect(xtrans->ioLayer);
    }
    while (rv==GWEN_ERROR_INTERRUPTED);

    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Notice,
                         I18N("Disconnected."));
    GWEN_SyncIo_free(xtrans->ioLayer);
    xtrans->ioLayer=NULL;
    AQFINTS_Transport_SubRuntimeFlags(trans, AQFINTS_TRANSPORT_RTFLAGS_CONNECTED);
  }
  return 0;
}



int transportSendMessage(AQFINTS_TRANSPORT *trans, const char *ptrBuffer, int lenBuffer)
{
  AQFINTS_TRANSPORT_HBCI *xtrans;
  int rv;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_HBCI, trans);
  assert(xtrans);

  if (xtrans->ioLayer==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Not connected");
    return GWEN_ERROR_INVALID;
  }

  rv=GWEN_SyncIo_WriteForced(xtrans->ioLayer, (const uint8_t *)ptrBuffer, lenBuffer);
  if (rv<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int transportReceiveMessage(AQFINTS_TRANSPORT *trans, GWEN_BUFFER *buffer)
{
  AQFINTS_TRANSPORT_HBCI *xtrans;
  int rv;
  char header[32];
  int msgSize;

  assert(trans);
  xtrans=GWEN_INHERIT_GETDATA(AQFINTS_TRANSPORT, AQFINTS_TRANSPORT_HBCI, trans);
  assert(xtrans);

  if (xtrans->ioLayer==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Not connected");
    return GWEN_ERROR_INVALID;
  }

  /* receive HNHBK */
  rv=GWEN_SyncIo_ReadForced(xtrans->ioLayer, (uint8_t *)header, sizeof(header)-1);
  if (rv<0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Error reading header (%d)", rv);
    return rv;
  }
  else if (rv!=sizeof(header)-1) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "EOF met");
    return GWEN_ERROR_EOF;
  }
  header[rv]=0;

  /* check for beginning of HBCI message */
  if (strncmp(header, "HNHBK:", 6)!=0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Message does not start with HNHBK");
    return GWEN_ERROR_BAD_DATA;
  }

  msgSize=AQFINTS_Transport_DetermineMessageSize(header);
  if (msgSize<0) {
    DBG_INFO(AQFINTS_LOGDOMAIN, "here (%d)", msgSize);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Unparsable message received"));
    return GWEN_ERROR_BAD_DATA;
  }

  /* subtract bytes already received */
  msgSize-=rv;

  /* make room for the rest of the message */
  GWEN_Buffer_AllocRoom(buffer, msgSize);

  /* receive rest of the message */
  rv=GWEN_SyncIo_ReadForced(xtrans->ioLayer,
                            (uint8_t *)GWEN_Buffer_GetPosPointer(buffer),
                            msgSize);
  if (rv<0) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "Error reading message (%d)", rv);
    return rv;
  }
  else if (rv!=msgSize) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "EOF met");
    return GWEN_ERROR_EOF;
  }

  /* advance pointer and counter in buffer */
  GWEN_Buffer_IncrementPos(buffer, msgSize);
  GWEN_Buffer_AdjustUsedBytes(buffer);

  return 0;
}








