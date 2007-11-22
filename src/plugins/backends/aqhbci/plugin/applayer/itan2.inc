/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: adminjobs.c 950 2006-02-22 19:39:02Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/* This file is included by outbox.c */


#include "message_l.h"
#include "user_l.h"

#include <gwenhywfar/mdigest.h>




int AH_Outbox__CBox_Itan2(AH_OUTBOX__CBOX *cbox,
                          AH_DIALOG *dlg,
                          AH_JOBQUEUE *qJob,
                          int timeout){
  const AH_JOB_LIST *jl;
  AH_MSG *msg1;
  AH_MSG *msg2;
  int rv;
  AH_JOB *j;
  AH_JOB *jTan1;
  AH_JOB *jTan2;
  AB_USER *u;
  const char *challenge;

  jl=AH_JobQueue_GetJobList(qJob);
  assert(jl);
  assert(AH_Job_List_GetCount(jl)==1);

  j=AH_Job_List_First(jl);
  assert(j);

  u=AH_Job_GetUser(j);
  assert(u);

  /* prepare HKTAN (process type 4) */
  jTan1=AH_Job_Tan_new(u, 4);
  if (!jTan1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    return -1;
  }

  /* copy signers */
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_SIGN) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(AH_Job_GetSigners(j));
    if (!se) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      return GWEN_ERROR_INVALID;
    }
    while(se) {
      AH_Job_AddSigner(jTan1, GWEN_StringListEntry_Data(se));
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* add job to queue */
  rv=AH_JobQueue_AddJob(qJob, jTan1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(jTan1);
    return rv;
  }

  /* create message */
  msg1=AH_Msg_new(dlg);
  /* add original job */
  rv=AH_Outbox__CBox_JobToMessage(j, msg1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    return rv;
  }

  /* add HKTAN message */
  rv=AH_Outbox__CBox_JobToMessage(jTan1, msg1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    return rv;
  }

  /* encode message */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding queue");
  GWEN_Gui_ProgressLog(
                         0,
                         GWEN_LoggerLevel_Info,
                         I18N("Encoding queue"));
  AH_Msg_SetNeedTan(msg1, 0);
  rv=AH_Msg_EncodeMsg(msg1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    return rv;
  }

  /* update job status */
  if (AH_Job_GetStatus(j)==AH_JobStatusEncoded) {
    const char *s;

    AH_Job_SetMsgNum(j, AH_Msg_GetMsgNum(msg1));
    AH_Job_SetDialogId(j, AH_Dialog_GetDialogId(dlg));
    /* store expected signer and crypter (if any) */
    s=AH_Msg_GetExpectedSigner(msg1);
    if (s) AH_Job_SetExpectedSigner(j, s);
    s=AH_Msg_GetExpectedCrypter(msg1);
    if (s) AH_Job_SetExpectedCrypter(j, s);
  }

  if (AH_Job_GetStatus(jTan1)==AH_JobStatusEncoded) {
    const char *s;

    AH_Job_SetMsgNum(jTan1, AH_Msg_GetMsgNum(msg1));
    AH_Job_SetDialogId(jTan1, AH_Dialog_GetDialogId(dlg));
    /* store expected signer and crypter (if any) */
    s=AH_Msg_GetExpectedSigner(msg1);
    if (s) AH_Job_SetExpectedSigner(jTan1, s);
    s=AH_Msg_GetExpectedCrypter(msg1);
    if (s) AH_Job_SetExpectedCrypter(jTan1, s);
  }

  /* send message */
  rv=AH_Outbox__CBox_Itan_SendMsg(cbox, dlg, msg1, timeout);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    return rv;
  }
  AH_Msg_free(msg1);

  AH_Job_SetStatus(j, AH_JobStatusSent);
  AH_Job_SetStatus(jTan1, AH_JobStatusSent);

  /* wait for response, dispatch it */
  rv=AH_Outbox__CBox_RecvQueue(cbox, timeout, dlg, qJob);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* get challenge */
  rv=AH_Job_Process(jTan1, cbox->outbox->context, cbox->guiid);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  challenge=AH_Job_Tan_GetChallenge(jTan1);

  /* prepare second message (the one with the TAN) */
  msg2=AH_Msg_new(dlg);
  AH_Msg_SetNeedTan(msg2, 1);
  AH_Msg_SetItanMethod(msg2, 0);
  AH_Msg_SetItanHashMode(msg2, 0);

  /* ask for TAN */
  if (challenge) {
    char tanBuffer[64];

    memset(tanBuffer, 0, sizeof(tanBuffer));
    rv=AH_User_InputTanWithChallenge(u,
				     challenge,
				     tanBuffer,
				     1,
				     sizeof(tanBuffer));
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
  jTan2=AH_Job_Tan_new(u, 2);
  if (!jTan2) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    AH_Job_free(jTan2);
    AH_Msg_free(msg2);
    return -1;
  }
  AH_Job_Tan_SetReference(jTan2, AH_Job_Tan_GetReference(jTan1));

  /* copy signers */
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_SIGN) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(AH_Job_GetSigners(j));
    if (!se) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      return GWEN_ERROR_INVALID;
    }
    while(se) {
      AH_Job_AddSigner(jTan2, GWEN_StringListEntry_Data(se));
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  rv=AH_JobQueue_AddJob(qJob, jTan2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(jTan2);
    AH_Msg_free(msg2);
    return rv;
  }

  rv=AH_Outbox__CBox_JobToMessage(jTan2, msg2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    return rv;
  }

  /* encode HKTAN message */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding queue");
  GWEN_Gui_ProgressLog(
			 0,
			 GWEN_LoggerLevel_Info,
			 I18N("Encoding queue"));
  AH_Msg_SetNeedTan(msg2, 1);
  rv=AH_Msg_EncodeMsg(msg2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    return rv;
  }

  if (AH_Job_GetStatus(jTan2)==AH_JobStatusEncoded) {
    const char *s;

    AH_Job_SetMsgNum(jTan2, AH_Msg_GetMsgNum(msg2));
    AH_Job_SetDialogId(jTan2, AH_Dialog_GetDialogId(dlg));
    /* store expected signer and crypter (if any) */
    s=AH_Msg_GetExpectedSigner(msg2);
    if (s) AH_Job_SetExpectedSigner(jTan2, s);
    s=AH_Msg_GetExpectedCrypter(msg2);
    if (s) AH_Job_SetExpectedCrypter(jTan2, s);
  }

  /* send HKTAN message */
  rv=AH_Outbox__CBox_Itan_SendMsg(cbox, dlg, msg2, timeout);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    return rv;
  }
  AH_Msg_free(msg2);
  AH_Job_SetStatus(jTan2, AH_JobStatusSent);

  /* wait for response, dispatch it */
  rv=AH_Outbox__CBox_RecvQueue(cbox, timeout, dlg, qJob);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}






