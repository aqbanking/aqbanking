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
#include "medium_l.h"

#include <gwenhywfar/md.h>




int AH_Outbox__CBox_Itan1(AH_OUTBOX__CBOX *cbox,
			  AH_DIALOG *dlg,
			  AH_JOBQUEUE *qJob,
			  int timeout){
  const AH_JOB_LIST *jl;
  AH_MSG *msg1;
  AH_MSG *msg2;
  int rv;
  AH_JOB *j;
  AH_JOB *jTan;
  AB_USER *u;
  AH_MEDIUM *medium;
  GWEN_DB_NODE *dbParams;
  GWEN_DB_NODE *dbMethod=0;
  GWEN_TYPE_UINT32 fid=0;
  GWEN_TYPE_UINT32 um=0;
  GWEN_BUFFER *bHash;
  AH_JOBQUEUE *jq;
  const char *challenge;

  jl=AH_JobQueue_GetJobList(qJob);
  assert(jl);
  assert(AH_Job_List_GetCount(jl)==1);

  j=AH_Job_List_First(jl);
  assert(j);

  u=AH_Job_GetUser(j);
  assert(u);

  medium=AH_User_GetMedium(u);
  assert(medium);

  um=AH_Dialog_GetItanMethod(dlg);
  assert(um);

  /* prepare HKTAN */
  jTan=AH_Job_Tan_new(u, 1);
  if (!jTan) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job HKTAN not available");
    return -1;
  }
  /* copy signers */
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_SIGN) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(AH_Job_GetSigners(j));
    if (!se) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      return AB_ERROR_INVALID;
    }
    while(se) {
      AH_Job_AddSigner(jTan, GWEN_StringListEntry_Data(se));
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* find DB_NODE for selected TanMethod */
  dbParams=AH_Job_GetParams(jTan);
  assert(dbParams);
  dbMethod=GWEN_DB_FindFirstGroup(dbParams, "tanMethod");
  while(dbMethod) {
    int fn;

    fid=0;
    fn=GWEN_DB_GetIntValue(dbMethod, "function", 0, 0);
    switch(fn) {
    case 999: fid=AH_USER_TANMETHOD_SINGLE_STEP; break;
    case 900: fid=AH_USER_TANMETHOD_TWO_STEP_00; break;
    case 990: fid=AH_USER_TANMETHOD_TWO_STEP_0; break;
    case 991: fid=AH_USER_TANMETHOD_TWO_STEP_1; break;
    case 992: fid=AH_USER_TANMETHOD_TWO_STEP_2; break;
    case 993: fid=AH_USER_TANMETHOD_TWO_STEP_3; break;
    case 994: fid=AH_USER_TANMETHOD_TWO_STEP_4; break;
    case 995: fid=AH_USER_TANMETHOD_TWO_STEP_5; break;
    case 996: fid=AH_USER_TANMETHOD_TWO_STEP_6; break;
    case 997: fid=AH_USER_TANMETHOD_TWO_STEP_7; break;
    default:
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid tan method %d", fn);
      fid=0;
    }

    if (fid) {
      if (um) {
	if (fid==um)
	  break;
      }
      else {
	if (AH_User_GetTanMethods(u) & fid)
	  break;
      }
    }

    dbMethod=GWEN_DB_FindNextGroup(dbMethod, "tanMethod");
  }

  if (!dbMethod || !fid) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "TAN mode %x not found", um);
    AH_Job_free(jTan);
    return -1;
  }

  /* calculate the job's hash */
  msg1=AH_Msg_new(dlg);
  AH_Msg_SetItanMethod(msg1, fid);
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
  AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			 0,
			 AB_Banking_LogLevelInfo,
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
  rv=AH_Outbox__CBox_Itan_SendMsg(cbox, dlg, msg2, timeout);
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
  rv=AH_Outbox__CBox_RecvQueue(cbox, timeout, dlg, jq);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    AH_JobQueue_free(jq);
    return rv;
  }


  /* get challenge */
  rv=AH_Job_Process(jTan, cbox->outbox->context);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    AH_JobQueue_free(jq);
    return rv;
  }
  challenge=AH_Job_Tan_GetChallenge(jTan);

  /* ask for TAN */
  if (challenge) {
    char tanBuffer[64];

    memset(tanBuffer, 0, sizeof(tanBuffer));
    rv=AH_Medium_InputTanWithChallenge(medium, challenge,
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
    return AB_ERROR_BAD_DATA;
  }

  AH_JobQueue_free(jq);

  /* now handle the real job */
  /* encode job message */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding queue");
  AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			 0,
			 AB_Banking_LogLevelInfo,
			 I18N("Encoding queue"));
  rv=AH_Msg_EncodeMsg(msg1);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    return rv;
  }

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

    /* store used TAN (if any) */
    s=AH_Msg_GetTan(msg1);
    if (s) AH_Job_SetUsedTan(j, s);
  }

  /* send job message */
  rv=AH_Outbox__CBox_Itan_SendMsg(cbox, dlg, msg1, timeout);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg1);
    return rv;
  }
  AH_Msg_free(msg1);
  AH_Job_SetStatus(j, AH_JobStatusSent);

  /* wait for response, dispatch it */
  rv=AH_Outbox__CBox_RecvQueue(cbox, timeout, dlg, qJob);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}






