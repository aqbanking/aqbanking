/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* included by outbox.c */



int AH_Outbox__CBox_OpenDialogPsd2_Proc2(AH_OUTBOX__CBOX *cbox, AH_DIALOG *dlg)
{
  AH_JOB *jDlgOpen;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating dialog open request");
  AH_Dialog_SetItanProcessType(dlg, 0);

  /* use strong authentication */
  AH_Dialog_AddFlags(dlg, AH_DIALOG_FLAGS_SCA);

  if (AH_User_HasTanMethodOtherThan(cbox->user, 999)) {
    /* only use itan if any other mode than singleStep is available
     * and the job queue does not request non-ITAN mode
     */
    rv=AH_Outbox__CBox_SelectItanMode(cbox, dlg);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "We have no list of allowed two-step TAN methods, maybe you should request TAN methods.");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Warning,
                         I18N("We have no list of allowed two-step TAN methods, maybe you should request TAN methods."));
    AH_Dialog_SetItanMethod(dlg, 999);
    AH_Dialog_SetItanProcessType(dlg, 1);
    AH_Dialog_SetTanJobVersion(dlg, 0);
  }


  /* dialog open job */
  jDlgOpen=AH_Job_new("JobDialogInit", cbox->provider, cbox->user, 0, 0);
  if (!jDlgOpen) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create job JobDialogInit");
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_SetCode(jDlgOpen, "HKIDN"); /* needed for HKTAN6 in SCA mode */
  AH_Job_AddSigner(jDlgOpen, AB_User_GetUserId(cbox->user));

  /* need signature in any case */
  AH_Job_AddFlags(jDlgOpen, AH_JOB_FLAGS_SIGN);

  rv=AH_Outbox__CBox_SendAndReceiveJobWithTan2(cbox, dlg, jDlgOpen);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(jDlgOpen);
    return rv;
  }


  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Dialog open request done.");
  rv=AH_Job_CommitSystemData(jDlgOpen, 0);
  AH_Job_free(jDlgOpen);
  return rv;
}


