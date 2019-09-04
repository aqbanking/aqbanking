/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* included by outbox.c */



int AH_Outbox__CBox_OpenDialog(AH_OUTBOX__CBOX *cbox,
                               AH_DIALOG *dlg,
                               uint32_t jqFlags)
{

  assert(cbox);
  assert(dlg);

  if (AH_User_GetCryptMode(cbox->user)==AH_CryptMode_Pintan) {
    /* TODO: check for PSD2, use appropriate function */
  }

  /* hardcoded for now, will be selected in the next step */
  return AH_Outbox__CBox_OpenDialog_Hbci(cbox, dlg, jqFlags);
}




int AH_Outbox__CBox_CloseDialog(AH_OUTBOX__CBOX *cbox,
                                AH_DIALOG *dlg,
                                uint32_t jqFlags)
{
  AH_JOBQUEUE *jqDlgClose;
  AH_JOB *jDlgClose;
  GWEN_DB_NODE *db;
  uint32_t dlgFlags;
  int rv;

  GWEN_Gui_ProgressLog(0,
                       GWEN_LoggerLevel_Notice,
                       I18N("Closing dialog"));
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Sending dialog close request (flags=%08x)",
             jqFlags);
  dlgFlags=AH_Dialog_GetFlags(dlg);
  jDlgClose=AH_Job_new("JobDialogEnd", cbox->provider, cbox->user, 0, 0);
  if (!jDlgClose) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create job JobDialogEnd");
    return GWEN_ERROR_GENERIC;
  }

  /* set some parameters */
  db=AH_Job_GetArguments(jDlgClose);
  assert(db);
  GWEN_DB_SetCharValue(db,
                       GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "dialogId",
                       AH_Dialog_GetDialogId(dlg));

  if (!(dlgFlags & AH_DIALOG_FLAGS_ANONYMOUS)) {
    /* sign and encrypt job */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Will encrypt and sign dialog close request");
    AH_Job_AddSigner(jDlgClose, AB_User_GetUserId(cbox->user));
    AH_Job_AddFlags(jDlgClose,
                    AH_JOB_FLAGS_CRYPT |
                    AH_JOB_FLAGS_SIGN);
  }
  jqDlgClose=AH_JobQueue_new(cbox->user);

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Adding dialog close request to queue");
  if (AH_JobQueue_AddJob(jqDlgClose, jDlgClose)!=
      AH_JobQueueAddResultOk) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add single job to queue");
    AH_JobQueue_free(jqDlgClose);
    return GWEN_ERROR_GENERIC;
  }

  rv=AH_Outbox__CBox_SendAndRecvQueue(cbox, dlg, jqDlgClose);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not exchange message");
    AH_JobQueue_free(jqDlgClose);
    return rv;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Dialog closed");
  rv=AH_Job_CommitSystemData(jDlgClose, 0);
  AH_JobQueue_free(jqDlgClose);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not commit system data");
    return rv;
  }
  return 0;
}







