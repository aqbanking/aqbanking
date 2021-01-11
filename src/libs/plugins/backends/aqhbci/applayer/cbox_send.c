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

#include "aqhbci/applayer/cbox_send.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gui.h>




int AH_OutboxCBox_SendMessage(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_MSG *msg)
{
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Sending message");
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Sending message"));
  rv=AH_Dialog_SendMessage(dlg, msg);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not send message");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Unable to send (network error)"));
    return rv;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Message sent");
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Message sent"));

  return 0;
}



int AH_OutboxCBox_SendQueue(AH_OUTBOX_CBOX *cbox,
                              AH_DIALOG *dlg,
                              AH_JOBQUEUE *jq)
{
  AH_MSG *msg;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding queue");
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Encoding queue"));
  msg=AH_JobQueue_ToMessage(jq, dlg);
  if (!msg) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not encode queue");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Unable to encode"));
    return GWEN_ERROR_GENERIC;
  }

  rv=AH_OutboxCBox_SendMessage(cbox, dlg, msg);
  AH_Msg_free(msg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Queue sent");
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Queue sent"));
  return 0;
}



