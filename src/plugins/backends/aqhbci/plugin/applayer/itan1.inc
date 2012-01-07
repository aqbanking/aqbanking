/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/* This file is included by outbox.c */


#include "message_l.h"
#include "user_l.h"

#include <gwenhywfar/mdigest.h>




int AH_Outbox__CBox_Itan1(AH_OUTBOX__CBOX *cbox,
			  AH_DIALOG *dlg,
			  AH_JOBQUEUE *qJob){
  const AH_JOB_LIST *jl;
  AH_MSG *msg1;
  AH_MSG *msg2;
  int rv;
  AH_JOB *j;
  AH_JOB *jTan;
  AB_USER *u;
  GWEN_DB_NODE *dbParams;
  uint32_t um=0;
  GWEN_BUFFER *bHash;
  AH_JOBQUEUE *jq;
  const char *challenge;
  const char *challengeHhd;
  //GWEN_STRINGLIST *sl;
  AB_ACCOUNT *acc=NULL;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Handling iTAN process type 1");

  jl=AH_JobQueue_GetJobList(qJob);
  assert(jl);
  assert(AH_Job_List_GetCount(jl)==1);

  j=AH_Job_List_First(jl);
  assert(j);

  u=AH_Job_GetUser(j);
  assert(u);

  um=AH_Dialog_GetItanMethod(dlg);
  assert(um);

  /* get account for HKTAN5 */
  if (j) {
    AH_JOB *aj;

    aj=j;
    while(aj) {
      if (AH_AccountJob_IsAccountJob(aj))
        break;
      aj=AH_Job_List_Next(aj);
    }
    if (aj)
      acc=AH_AccountJob_GetAccount(aj);
  }

  /* prepare HKTAN */
  jTan=AH_Job_Tan_new(u, 1, AH_Dialog_GetTanJobVersion(dlg));
  if (!jTan) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    return -1;
  }

  AH_Job_Tan_SetTanMethod(jTan, um);
  AH_Job_Tan_SetTanMediumId(jTan, AH_User_GetTanMediumId(u));

  DBG_INFO(AQHBCI_LOGDOMAIN, "Setting up HKTAN from job [%s]",
           AH_Job_GetName(j));

  if (acc) {
    const char *baBankCode;
    const char *baAccountId;
    const char *baAccountSubId;

    baBankCode=AB_Account_GetBankCode(acc);
    baAccountId=AB_Account_GetAccountNumber(acc);
    baAccountSubId=AB_Account_GetSubAccountId(acc);

    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Setting local and sms bank account for HKTAN to %s/%s/%s",
             baBankCode?baBankCode:"(none)",
             baAccountId?baAccountId:"(none)",
             baAccountSubId?baAccountSubId:"(none)");
    AH_Job_Tan_SetLocalAccountInfo(jTan, baBankCode, baAccountId, baAccountSubId);
    if (!(AH_User_GetFlags(u) & AH_USER_FLAGS_TAN_OMIT_SMS_ACCOUNT))
      /* only set SMS account if allowed */
      AH_Job_Tan_SetSmsAccountInfo(jTan, baBankCode, baAccountId, baAccountSubId);
  }

#if 0
  /* copy challenge params */
  sl=AH_Job_GetChallengeParams(j);
  if (sl) {
    GWEN_STRINGLISTENTRY *e;

    e=GWEN_StringList_FirstEntry(sl);
    while(e) {
      AH_Job_AddChallengeParam(jTan, GWEN_StringListEntry_Data(e));
      e=GWEN_StringListEntry_Next(e);
    }
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No challenge params");
  }

  /* copy challenge amount */
  AH_Job_SetChallengeValue(jTan, AH_Job_GetChallengeValue(j));
#endif

  /* copy challenge class */
  AH_Job_SetChallengeClass(jTan, AH_Job_GetChallengeClass(j));

  /* copy signers */
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_SIGN) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(AH_Job_GetSigners(j));
    if (!se) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      return GWEN_ERROR_INVALID;
    }
    while(se) {
      AH_Job_AddSigner(jTan, GWEN_StringListEntry_Data(se));
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* find DB_NODE for selected TanMethod */
  dbParams=AH_Job_GetParams(jTan);
  assert(dbParams);

  /* calculate the job's hash */
  msg1=AH_Msg_new(dlg);
  AH_Msg_SetItanMethod(msg1, um);
  AH_Msg_SetItanHashMode(msg1,
			 GWEN_DB_GetIntValue(dbParams, "hashMethod", 0, 0));
  rv=AH_Outbox__CBox_JobToMessage(j, msg1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    AH_Job_free(jTan);
    return rv;
  }

  /* get the job's hash */
  bHash=AH_Msg_GetItanHashBuffer(msg1);
  assert(bHash);

  /* create second message: This will be sent first */
  msg2=AH_Msg_new(dlg);
  AH_Msg_SetItanMethod(msg2, 0);
  AH_Msg_SetItanHashMode(msg2, 0);
  AH_Job_Tan_SetHash(jTan,
		     (const unsigned char*)GWEN_Buffer_GetStart(bHash),
		     GWEN_Buffer_GetUsedBytes(bHash));
  AH_Job_Tan_SetSegCode(jTan, AH_Job_GetCode(j));

  rv=AH_Job_Tan_FinishSetup(jTan, j);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    AH_Job_free(jTan);
    return rv;
  }

  jq=AH_JobQueue_new(u);
  rv=AH_JobQueue_AddJob(jq, jTan);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    AH_Msg_free(msg1);
    AH_JobQueue_free(jq);
    return rv;
  }

  rv=AH_Outbox__CBox_JobToMessage(jTan, msg2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    AH_Msg_free(msg1);
    AH_JobQueue_free(jq);
    return rv;
  }

  /* encode HKTAN message */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding queue");
  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Info,
		       I18N("Encoding queue"));
  AH_Msg_SetNeedTan(msg2, 0);
  rv=AH_Msg_EncodeMsg(msg2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    AH_Msg_free(msg1);
    AH_JobQueue_free(jq);
    return rv;
  }

  if (AH_Job_GetStatus(jTan)==AH_JobStatusEncoded) {
    const char *s;

    AH_Job_SetMsgNum(jTan, AH_Msg_GetMsgNum(msg2));
    AH_Job_SetDialogId(jTan, AH_Dialog_GetDialogId(dlg));
    /* store expected signer and crypter (if any) */
    s=AH_Msg_GetExpectedSigner(msg2);
    if (s) AH_Job_SetExpectedSigner(jTan, s);
    s=AH_Msg_GetExpectedCrypter(msg2);
    if (s) AH_Job_SetExpectedCrypter(jTan, s);
  }

  /* send HKTAN message */
  rv=AH_Outbox__CBox_Itan_SendMsg(cbox, dlg, msg2);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg2);
    AH_Msg_free(msg1);
    AH_JobQueue_free(jq);
    return rv;
  }
  AH_Msg_free(msg2);
  AH_Job_SetStatus(jTan, AH_JobStatusSent);

  /* wait for response, dispatch it */
  rv=AH_Outbox__CBox_RecvQueue(cbox, dlg, jq);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Outbox__CBox_CopyJobResultsToJobList(jTan, jl);
    AH_Msg_free(msg1);
    AH_JobQueue_free(jq);
    return rv;
  }
  AH_Outbox__CBox_CopyJobResultsToJobList(jTan, jl);

  /* get challenge */
  rv=AH_Job_Process(jTan, cbox->outbox->context);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    AH_JobQueue_free(jq);
    return rv;
  }
  challengeHhd=AH_Job_Tan_GetHhdChallenge(jTan);
  challenge=AH_Job_Tan_GetChallenge(jTan);

  /* ask for TAN */
  if (challenge || challengeHhd) {
    char tanBuffer[64];

    memset(tanBuffer, 0, sizeof(tanBuffer));
    rv=AH_User_InputTanWithChallenge2(u,
                                      challenge,
                                      challengeHhd,
                                      tanBuffer,
                                      1,
                                      sizeof(tanBuffer));
    if (rv) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AH_Msg_free(msg1);
      AH_JobQueue_free(jq);
      return rv;
    }

    /* set TAN in msg 1 */
    AH_Msg_SetTan(msg1, tanBuffer);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No challenge received");
    AH_Msg_free(msg1);
    AH_JobQueue_free(jq);
    return GWEN_ERROR_BAD_DATA;
  }

  AH_JobQueue_free(jq);
  jq=NULL;

  /* now handle the real job */
  /* encode job message */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding queue");
  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Info,
		       I18N("Encoding queue"));
  rv=AH_Msg_EncodeMsg(msg1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    return rv;
  }

  /* store used TAN in original job (if any) */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Storing TAN in job [%s]",
	   AH_Job_GetName(j));
  AH_Job_SetUsedTan(j, AH_Msg_GetTan(msg1));

  if (AH_Job_GetStatus(j)==AH_JobStatusEncoded) {
    const char *s;

    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job encoded");
    AH_Job_SetMsgNum(j, AH_Msg_GetMsgNum(msg1));
    AH_Job_SetDialogId(j, AH_Dialog_GetDialogId(dlg));
    /* store expected signer and crypter (if any) */
    s=AH_Msg_GetExpectedSigner(msg1);
    if (s) AH_Job_SetExpectedSigner(j, s);
    s=AH_Msg_GetExpectedCrypter(msg1);
    if (s) AH_Job_SetExpectedCrypter(j, s);
  }

  /* send job message */
  rv=AH_Outbox__CBox_Itan_SendMsg(cbox, dlg, msg1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    return rv;
  }
  AH_Msg_free(msg1);
  AH_Job_SetStatus(j, AH_JobStatusSent);

  /* wait for response, dispatch it */
  rv=AH_Outbox__CBox_RecvQueue(cbox, dlg, qJob);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}






