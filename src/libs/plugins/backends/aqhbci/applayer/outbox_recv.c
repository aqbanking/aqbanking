/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "aqhbci/applayer/outbox_recv.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gui.h>




AH_MSG *AH_Outbox__CBox_RecvMessage(AH_OUTBOX__CBOX *cbox, AH_DIALOG *dlg, GWEN_DB_NODE *dbRsp)
{
  AH_MSG *msg=NULL;
  int rv;

  assert(cbox);

  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Waiting for response"));

  rv=AH_Dialog_RecvMessage(dlg, &msg);
  if (rv>=200 && rv<300)
    rv=0;
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error receiving response (%d)", rv);
    GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Error, I18N("Error receiving response (%d)"), rv);
    return NULL;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Got a message");
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Response received"));

  /* try to dispatch the message */
  if (AH_Msg_DecodeMsg(msg, dbRsp, GWEN_MSGENGINE_READ_FLAGS_DEFAULT)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not decode this message:");
    AH_Msg_Dump(msg, 2);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Bad response (unable to decode)"));
    AH_Msg_free(msg);
    return NULL;
  }

  /* transform from ISO 8859-1 to UTF8 */
  AB_ImExporter_DbFromIso8859_1ToUtf8(dbRsp);

  /* check for message reference */
  if (AH_Msg_GetMsgRef(msg)==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unrequested message, deleting it");
    AH_Msg_Dump(msg, 2);
    GWEN_DB_Dump(dbRsp, 2);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Bad response (bad message reference)"));
    AH_Msg_free(msg);
    return NULL;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Message received and decoded");
  return msg;
}



int AH_Outbox__CBox_RecvQueue(AH_OUTBOX__CBOX *cbox,
                              AH_DIALOG *dlg,
                              AH_JOBQUEUE *jq)
{
  AH_MSG *msg;
  GWEN_DB_NODE *dbRsp;
  int rv;

  assert(cbox);

  dbRsp=GWEN_DB_Group_new("response");
  msg=AH_Outbox__CBox_RecvMessage(cbox, dlg, dbRsp);
  if (msg==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_DB_Group_free(dbRsp);
    return GWEN_ERROR_GENERIC;
  }

  rv=AH_JobQueue_DispatchMessage(jq, msg, dbRsp);
  if (rv) {
    if (rv==GWEN_ERROR_ABORTED) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Dialog aborted by server");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Dialog aborted by server"));
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not dispatch response");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Bad response (unable to dispatch)"));
    }
    GWEN_DB_Group_free(dbRsp);
    AH_Msg_free(msg);
    return rv;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Message dispatched");
  GWEN_DB_Group_free(dbRsp);
  AH_Msg_free(msg);
  return 0;
}





