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


#include "aqhbci/applayer/cbox_hbci.h"

#include "aqhbci/applayer/cbox_queue.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gui.h>



int AH_OutboxCBox_OpenDialog_Hbci(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, uint32_t jqFlags)
{
  AB_PROVIDER *provider;
  AB_USER *user;
  AH_JOBQUEUE *jqDlgOpen;
  AH_JOB *jDlgOpen;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating dialog open request");

  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  AH_Dialog_SetItanProcessType(dlg, 0);

  if ((jqFlags & AH_JOBQUEUE_FLAGS_CRYPT) || (jqFlags & AH_JOBQUEUE_FLAGS_SIGN)) {
    /* sign and crypt, not anonymous */
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating non-anonymous dialog open request");
    jDlgOpen=AH_Job_new("JobDialogInit", provider, user, 0, 0);
    if (!jDlgOpen) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create job JobDialogInit");
      return GWEN_ERROR_GENERIC;
    }
    if (jqFlags & AH_JOBQUEUE_FLAGS_SIGN)
      AH_Job_AddSigner(jDlgOpen, AB_User_GetUserId(user));
    AH_Dialog_SubFlags(dlg, AH_DIALOG_FLAGS_ANONYMOUS);

    if (AH_User_GetCryptMode(user)==AH_CryptMode_Pintan) {
      if (AH_User_HasTanMethodOtherThan(user, 999) &&
          !(jqFlags & AH_JOBQUEUE_FLAGS_NOITAN)) {
        /* only use itan if any other mode than singleStep is available
         * and the job queue does not request non-ITAN mode
         */
        rv=AH_OutboxCBox_SelectItanMode(cbox, dlg);
        if (rv) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
          return rv;
        }
      }
    }
  }
  else {
    /* neither sign nor crypt, use anonymous dialog */
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating anonymous dialog open request");
    jDlgOpen=AH_Job_new("JobDialogInitAnon", provider, user, 0, 0);
    if (!jDlgOpen) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create job JobDialogInitAnon");
      return GWEN_ERROR_GENERIC;
    }
    AH_Dialog_AddFlags(dlg, AH_DIALOG_FLAGS_ANONYMOUS);
  }

  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice, I18N("Opening dialog"));
  jqDlgOpen=AH_JobQueue_new(user);
  AH_JobQueue_AddFlags(jqDlgOpen, AH_JOBQUEUE_FLAGS_OUTBOX);
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Enqueueing dialog open request");
  if (AH_JobQueue_AddJob(jqDlgOpen, jDlgOpen)!=AH_JobQueueAddResultOk) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add single job to queue");
    AH_Job_free(jDlgOpen);
    AH_JobQueue_free(jqDlgOpen);
    return GWEN_ERROR_GENERIC;
  }

  rv=AH_OutboxCBox_SendAndRecvQueue(cbox, dlg, jqDlgOpen);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not exchange message");
    AH_JobQueue_free(jqDlgOpen);
    return rv;
  }
  if (AH_Job_HasErrors(jDlgOpen)) {
    /* TODO: check for iTAN related error and try again */
    if (AH_Job_HasItanResult(jDlgOpen)) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Adjusting to iTAN modes of the server");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice, I18N("Adjusting to iTAN modes of the server"));
      /* do not call AH_Job_CommitSystemData() here, the iTAN modes have already
       * been caught by AH_JobQueue_DispatchMessage()
        AH_Job_CommitSystemData(jDlgOpen); */
      AH_JobQueue_free(jqDlgOpen);
      return 1;
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error opening dialog, aborting");
      AH_JobQueue_free(jqDlgOpen);
      return GWEN_ERROR_GENERIC;
    }
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Dialog open request done");
  rv=AH_Job_CommitSystemData(jDlgOpen, 0);
  AH_JobQueue_free(jqDlgOpen);
  return rv;
}



