/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* included by outbox.c */



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int _sendAndReceiveTanMessageProc2(AH_OUTBOX__CBOX *cbox,
					  AH_DIALOG *dlg,
					  AH_JOBQUEUE *qJob,
					  AH_JOB *job,
					  AH_JOB *jTan1);


static int _sendAndReceiveTanResponseProc2(AH_OUTBOX__CBOX *cbox,
					   AH_DIALOG *dlg,
					   AH_JOBQUEUE *qJob,
					   AH_JOB *jTan1);

static void _dispatchJobSegResultsToQueue(AH_JOB *job, AH_JOBQUEUE *qJob);
static void _dispatchJobMsgResultsToQueue(AH_JOB *job, AH_JOBQUEUE *qJob);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_Outbox__CBox_SendTanJobQueue_Proc2(AH_OUTBOX__CBOX *cbox,
					  AH_DIALOG *dlg,
					  AH_JOB *job)
{
  int rv;
  AH_JOBQUEUE *qJob;
  AH_JOB *jTan1;
  AB_USER *u;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Sending job with TAN (process variant 2)");

  assert(job);
  u=AH_Job_GetUser(job);
  assert(u);

  qJob=AH_JobQueue_new(cbox->user);

  /* add original job to queue */
  AH_Job_Attach(job);
  rv=AH_JobQueue_AddJob(qJob, job);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* prepare HKTAN (process type 4) */
  jTan1=AH_Job_Tan_new(cbox->provider, u, 4, AH_Dialog_GetTanJobVersion(dlg));
  if (!jTan1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    AH_JobQueue_free(qJob);
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_Tan_SetTanMediumId(jTan1, AH_User_GetTanMediumId(u));
  if (AH_Dialog_GetFlags(dlg) &  AH_DIALOG_FLAGS_SCA) {
    /* With tan process "4" only with strong verification */
    AH_Job_Tan_SetSegCode(jTan1, AH_Job_GetCode(job));
  }

  /* copy signers */
  if (AH_Job_GetFlags(job) & AH_JOB_FLAGS_SIGN) {
    rv=AH_Job_AddSigners(jTan1, AH_Job_GetSigners(job));
    if (rv<1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      AH_JobQueue_free(qJob);
      return GWEN_ERROR_INVALID;
    }
  }

  /* add job to queue */
  rv=AH_JobQueue_AddJob(qJob, jTan1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(jTan1);
    AH_JobQueue_free(qJob);
    return rv;
  }

  /* send original job with HKTAN, receive response and dispatch it to those jobs */
  rv=_sendAndReceiveTanMessageProc2(cbox, dlg, qJob, job, jTan1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(qJob);
    return rv;
  }

  if (AH_Job_HasResultWithCode(job, 3076) ||
      AH_Job_HasResultWithCode(jTan1, 3076)) { /* SCA not needed */
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No TAN needed");
  }
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job has no 3076 result, getting TAN");
    DBG_ERROR(0, "Original job:");
    AH_Job_Dump(job, stderr, 2);

    DBG_ERROR(0, "TAN job:");
    AH_Job_Dump(jTan1, stderr, 2);

    rv=_sendAndReceiveTanResponseProc2(cbox, dlg, qJob, jTan1);
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_JobQueue_free(qJob);
      return rv;
    }
  }

  AH_JobQueue_free(qJob);

  return 0;
}



int _sendAndReceiveTanMessageProc2(AH_OUTBOX__CBOX *cbox,
				   AH_DIALOG *dlg,
				   AH_JOBQUEUE *qJob,
				   AH_JOB *job,
				   AH_JOB *jTan1)
{
  AH_MSG *msg1;
  int rv;

  /* create message */
  msg1=AH_Msg_new(dlg);
  /* add original job */
  rv=AH_Outbox__CBox_JobToMessage(job, msg1, 1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    return rv;
  }

  /* add HKTAN message */
  rv=AH_Outbox__CBox_JobToMessage(jTan1, msg1, 0);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    return rv;
  }

  /* encode message */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding queue");
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Encoding queue"));
  AH_Msg_SetNeedTan(msg1, 0);
  rv=AH_Msg_EncodeMsg(msg1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    return rv;
  }

  /* update job status */
  if (AH_Job_GetStatus(job)==AH_JobStatusEncoded) {
    const char *s;

    AH_Job_SetMsgNum(job, AH_Msg_GetMsgNum(msg1));
    AH_Job_SetDialogId(job, AH_Dialog_GetDialogId(dlg));
    /* store expected signer and crypter (if any) */
    s=AH_Msg_GetExpectedSigner(msg1);
    if (s)
      AH_Job_SetExpectedSigner(job, s);
    s=AH_Msg_GetExpectedCrypter(msg1);
    if (s)
      AH_Job_SetExpectedCrypter(job, s);
  }

  if (AH_Job_GetStatus(jTan1)==AH_JobStatusEncoded) {
    const char *s;

    AH_Job_SetMsgNum(jTan1, AH_Msg_GetMsgNum(msg1));
    AH_Job_SetDialogId(jTan1, AH_Dialog_GetDialogId(dlg));
    /* store expected signer and crypter (if any) */
    s=AH_Msg_GetExpectedSigner(msg1);
    if (s)
      AH_Job_SetExpectedSigner(jTan1, s);
    s=AH_Msg_GetExpectedCrypter(msg1);
    if (s)
      AH_Job_SetExpectedCrypter(jTan1, s);
  }

  /* send message */
  rv=AH_Outbox__CBox_Itan_SendMsg(cbox, dlg, msg1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    return rv;
  }
  AH_Msg_free(msg1);

  AH_Job_SetStatus(job, AH_JobStatusSent);
  AH_Job_SetStatus(jTan1, AH_JobStatusSent);

  /* wait for response, dispatch it */
  rv=AH_Outbox__CBox_RecvQueue(cbox, dlg, qJob);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



int _sendAndReceiveTanResponseProc2(AH_OUTBOX__CBOX *cbox,
				    AH_DIALOG *dlg,
				    AH_JOBQUEUE *qJob,
				    AH_JOB *jTan1)
{
  int rv;
  AH_JOB *j;
  AB_USER *u;
  const char *challenge;
  const char *challengeHhd;
  AH_JOBQUEUE *qJob2=NULL;
  AH_MSG *msg2;
  AH_JOB *jTan2;

  assert(qJob);
  assert(jTan1);

  j=AH_JobQueue_GetFirstJob(qJob);
  assert(j);
  u=AH_Job_GetUser(j);
  assert(u);

  /* get challenge */
  rv=AH_Job_Process(jTan1, cbox->outbox->context);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  challengeHhd=AH_Job_Tan_GetHhdChallenge(jTan1);
  challenge=AH_Job_Tan_GetChallenge(jTan1);

  /* prepare second message (the one with the TAN) */
  qJob2=AH_JobQueue_fromQueue(qJob);
  msg2=AH_Msg_new(dlg);
  AH_Msg_SetNeedTan(msg2, 1);
  AH_Msg_SetItanMethod(msg2, 0);
  AH_Msg_SetItanHashMode(msg2, 0);

  /* ask for TAN */
  if (challenge || challengeHhd) {
    char tanBuffer[64];

    memset(tanBuffer, 0, sizeof(tanBuffer));
    rv=AH_Outbox__CBox_InputTanWithChallenge(cbox, dlg, challenge, challengeHhd, tanBuffer, 1, sizeof(tanBuffer)-1);
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_Msg_free(msg2);
      return rv;
    }

    /* set TAN in msg 2 */
    AH_Msg_SetTan(msg2, tanBuffer);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No challenge received");
    AH_Msg_free(msg2);
    return GWEN_ERROR_BAD_DATA;
  }

  /* prepare HKTAN (process type 2) */
  jTan2=AH_Job_Tan_new(cbox->provider, u, 2, AH_Dialog_GetTanJobVersion(dlg));
  if (!jTan2) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    AH_Job_free(jTan2);
    AH_Msg_free(msg2);
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_Tan_SetReference(jTan2, AH_Job_Tan_GetReference(jTan1));
  AH_Job_Tan_SetTanMediumId(jTan2, AH_User_GetTanMediumId(u));

  /* copy signers */
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_SIGN) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(AH_Job_GetSigners(j));
    if (!se) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      AH_Job_free(jTan2);
      AH_Msg_free(msg2);
      AH_JobQueue_free(qJob2);
      return GWEN_ERROR_INVALID;
    }
    while (se) {
      AH_Job_AddSigner(jTan2, GWEN_StringListEntry_Data(se));
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  rv=AH_JobQueue_AddJob(qJob2, jTan2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(jTan2);
    AH_Msg_free(msg2);
    AH_JobQueue_free(qJob2);
    return rv;
  }

  rv=AH_Outbox__CBox_JobToMessage(jTan2, msg2, 1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    AH_JobQueue_free(qJob2);
    return rv;
  }

  /* encode HKTAN message */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding queue");
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Encoding queue"));
  AH_Msg_SetNeedTan(msg2, 1);
  rv=AH_Msg_EncodeMsg(msg2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    AH_JobQueue_free(qJob2);
    return rv;
  }

  /* store used TAN in original job (if any) */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Storing TAN in job [%s]", AH_Job_GetName(j));
  AH_Job_SetUsedTan(j, AH_Msg_GetTan(msg2));

  if (AH_Job_GetStatus(jTan2)==AH_JobStatusEncoded) {
    const char *s;

    AH_Job_SetMsgNum(jTan2, AH_Msg_GetMsgNum(msg2));
    AH_Job_SetDialogId(jTan2, AH_Dialog_GetDialogId(dlg));
    /* store expected signer and crypter (if any) */
    s=AH_Msg_GetExpectedSigner(msg2);
    if (s)
      AH_Job_SetExpectedSigner(jTan2, s);
    s=AH_Msg_GetExpectedCrypter(msg2);
    if (s)
      AH_Job_SetExpectedCrypter(jTan2, s);

    /* store TAN in TAN job */
    AH_Job_SetUsedTan(jTan2, AH_Msg_GetTan(msg2));
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "jTAN2 not encoded? (%d)", AH_Job_GetStatus(jTan2));
  }

  /* send HKTAN message */
  rv=AH_Outbox__CBox_Itan_SendMsg(cbox, dlg, msg2); /* directly send message via AH_DIALOG */
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    AH_JobQueue_free(qJob2);
    return rv;
  }
  AH_Msg_free(msg2);
  AH_Job_SetStatus(jTan2, AH_JobStatusSent);

  /* wait for response, dispatch it */
  rv=AH_Outbox__CBox_RecvQueue(cbox, dlg, qJob2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_JobQueue_free(qJob2);
    return rv;
  }
  else {
    rv=AH_Job_Process(jTan2, cbox->outbox->context);
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_JobQueue_free(qJob2);
      return rv;
    }

    /* dispatch results from jTan2 to all members of the queue */
    _dispatchJobSegResultsToQueue(jTan2, qJob);
    _dispatchJobMsgResultsToQueue(jTan2, qJob);
  }
  AH_JobQueue_free(qJob2);

  return 0;
}



void _dispatchJobSegResultsToQueue(AH_JOB *job, AH_JOBQUEUE *qJob)
{
  const AH_JOB_LIST *qjl;

  /* dispatch results from job to all members of the queue */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Dispatching results for HKTAN to queue jobs");
  qjl=AH_JobQueue_GetJobList(qJob);
  if (qjl) {
    AH_RESULT_LIST *rl;
  
    /* segment results */
    rl=AH_Job_GetSegResults(job);
    if (rl) {
      AH_RESULT *origRes;
  
      origRes=AH_Result_List_First(rl);
      if (origRes==NULL) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "No segment result in job HKTAN");
      }
      while (origRes) {
	AH_JOB *qj;
  
	qj=AH_Job_List_First(qjl);
	while (qj) {
	  if (qj!=job) {
	    AH_RESULT *nr;
  
	    nr=AH_Result_dup(origRes);
	    DBG_ERROR(AQHBCI_LOGDOMAIN, "Adding result %d to job %s", AH_Result_GetCode(origRes), AH_Job_GetName(qj));
	    AH_Result_List_Add(nr, AH_Job_GetSegResults(qj));
	  }
	  else {
	    DBG_INFO(AQHBCI_LOGDOMAIN, "Not adding result to the same job");
	  }
	  qj=AH_Job_List_Next(qj);
	}
  
	origRes=AH_Result_List_Next(origRes);
      } /* while origRes */
    } /* if rl */
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No segment results in HKTAN");
    }
  } /* if jql */
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No job list");
  }
}



void _dispatchJobMsgResultsToQueue(AH_JOB *job, AH_JOBQUEUE *qJob)
{
  const AH_JOB_LIST *qjl;

  /* dispatch results from job to all members of the queue */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Dispatching results for HKTAN to queue jobs");
  qjl=AH_JobQueue_GetJobList(qJob);
  if (qjl) {
    AH_RESULT_LIST *rl;

    /* msg results */
    rl=AH_Job_GetMsgResults(job);
    if (rl) {
      AH_RESULT *origRes;
  
      origRes=AH_Result_List_First(rl);
      while (origRes) {
	AH_JOB *qj;
  
	qj=AH_Job_List_First(qjl);
	while (qj) {
	  AH_RESULT *nr;
  
	  nr=AH_Result_dup(origRes);
	  AH_Result_List_Add(nr, AH_Job_GetMsgResults(qj));
	  qj=AH_Job_List_Next(qj);
	}
  
	origRes=AH_Result_List_Next(origRes);
      } /* while origRes */
    } /* if rl */
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No message results in HKTAN");
    }
  } /* if qjl */
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No job list");
  }
}



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

  rv=AH_Outbox__CBox_SendTanJobQueue_Proc2(cbox, dlg, jDlgOpen);
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


