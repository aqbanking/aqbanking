/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobqueue_p.h"
#include "aqhbci_l.h"
#include "job_l.h"
#include "user_l.h"
#include "message_l.h"
#include "hbci_l.h"
#include "medium_l.h"
#include "dialog_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/waitcallback.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


GWEN_LIST_FUNCTIONS(AH_JOBQUEUE, AH_JobQueue);



AH_JOBQUEUE *AH_JobQueue_new(AB_USER *u){
  AH_JOBQUEUE *jq;

  assert(u);

  GWEN_NEW_OBJECT(AH_JOBQUEUE, jq);
  GWEN_LIST_INIT(AH_JOBQUEUE, jq);

  jq->user=u;
  jq->signers=GWEN_StringList_new();
  jq->jobs=AH_Job_List_new();
  jq->usage=1;
  return jq;
}



void AH_JobQueue_free(AH_JOBQUEUE *jq){
  if (jq) {
    assert(jq->usage);
    if (--(jq->usage)==0) {
      GWEN_StringList_free(jq->signers);
      AH_Job_List_free(jq->jobs);
      free(jq->usedTan);
      free(jq->usedPin);

      GWEN_LIST_FINI(AH_JOBQUEUE, jq);
      GWEN_FREE_OBJECT(jq);
    }
  }
}



void AH_JobQueue_Attach(AH_JOBQUEUE *jq){
  assert(jq);
  jq->usage++;
}



void AH_JobQueue_SetUsedTan(AH_JOBQUEUE *jq, const char *s){
  assert(jq);
  assert(jq->usage);
  free(jq->usedTan);
  if (s) jq->usedTan=strdup(s);
  else jq->usedTan=0;
}



void AH_JobQueue_SetUsedPin(AH_JOBQUEUE *jq, const char *s){
  assert(jq);
  assert(jq->usage);
  free(jq->usedPin);
  if (s) jq->usedPin=strdup(s);
  else jq->usedPin=0;
}



AH_JOBQUEUE_ADDRESULT AH_JobQueue_AddJob(AH_JOBQUEUE *jq, AH_JOB *j){
  int jobsPerMsg;
  int maxJobTypes;
  int jobCount;
  int jobTypeCount;
  int thisJobTypeCount;
  int hasSingle;
  int crypt;
  int needTAN;
  int noSysId;
  int noItan;
  GWEN_STRINGLIST *jobTypes;
  AH_JOB *cj;
  AH_BPD *bpd;

  assert(jq);
  assert(jq->usage);

  /* job owner must equal queue owner */
  if (AH_Job_GetUser(j)!=jq->user) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Owner of the job doesn't match");
    return AH_JobQueueAddResultJobLimit;
  }

  /* sample some variables */
  bpd=AH_User_GetBpd(jq->user);
  jobsPerMsg=AH_Job_GetJobsPerMsg(j);
  maxJobTypes=AH_Bpd_GetJobTypesPerMsg(bpd);

  jobCount=0;
  jobTypeCount=0;
  thisJobTypeCount=0;
  hasSingle=0;
  crypt=0;
  needTAN=0;
  noSysId=0;
  noItan=0;
  jobTypes=GWEN_StringList_new();
  cj=AH_Job_List_First(jq->jobs);
  while(cj) {
    jobCount++;
    GWEN_StringList_AppendString(jobTypes, AH_Job_GetName(cj), 0, 1);
    if (strcasecmp(AH_Job_GetName(cj), AH_Job_GetName(j))==0)
      thisJobTypeCount++;
    hasSingle|=((AH_Job_GetFlags(cj) & AH_JOB_FLAGS_SINGLE) ||
                (AH_Job_GetFlags(cj) & AH_JOB_FLAGS_DLGJOB));
    crypt|=(AH_Job_GetFlags(cj) & AH_JOB_FLAGS_CRYPT);
    needTAN|=(AH_Job_GetFlags(cj) & AH_JOB_FLAGS_NEEDTAN);
    noSysId|=(AH_Job_GetFlags(cj) & AH_JOB_FLAGS_NOSYSID);
    noItan|=(AH_Job_GetFlags(cj) & AH_JOB_FLAGS_NOITAN);
    cj=AH_Job_List_Next(cj);
  } /* while */
  jobTypeCount=GWEN_StringList_Count(jobTypes);
  GWEN_StringList_free(jobTypes);

  if (strcasecmp(AH_Job_GetName(j), "JobTan")!=0) {
    if (jobCount &&
        (
         (crypt!=(AH_Job_GetFlags(j) & AH_JOB_FLAGS_CRYPT)) ||
         (needTAN!=(AH_Job_GetFlags(j) & AH_JOB_FLAGS_NEEDTAN)) ||
         (noSysId!=(AH_Job_GetFlags(j) & AH_JOB_FLAGS_NOSYSID)) ||
         (noItan!=(AH_Job_GetFlags(j) & AH_JOB_FLAGS_NOITAN))
        )
       ) {

      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Encryption/TAN/SysId flags for queue and this job differ");
      return AH_JobQueueAddResultJobLimit;
    }

    /* check for single jobs */
    if (hasSingle) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Queue already contains a job which wants to be left alone");
      return AH_JobQueueAddResultQueueFull;
    }

    /* check if this job is single and there already are jobs in the queue */
    if (((AH_Job_GetFlags(j) & AH_JOB_FLAGS_SINGLE) ||
         (AH_Job_GetFlags(j) & AH_JOB_FLAGS_DLGJOB)) && jobCount) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Queue already contains jobs and this one has the SINGLE flag");
      return AH_JobQueueAddResultJobLimit;
    }

    /* check if adding this job would exceed the limit of jobs of this kind */
    if (jobsPerMsg && thisJobTypeCount>jobsPerMsg) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Too many jobs of this kind (limit is %d)", jobsPerMsg);
      return AH_JobQueueAddResultJobLimit;
    }

    /* check for maximum of different job types per message */
    if (maxJobTypes && jobTypeCount>maxJobTypes) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Too many different job types (limit is %d)", maxJobTypes);
      return AH_JobQueueAddResultJobLimit;
    }

    /* check for signers */
    if (!jobCount && !GWEN_StringList_Count(jq->signers)) {
      const GWEN_STRINGLIST *sl;

      /* no jobs in queue and no signers,
       * so simply copy the signers of this job */
      sl=AH_Job_GetSigners(j);
      if (sl) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Copying signers from job to queue");
        GWEN_StringList_free(jq->signers);
        jq->signers=GWEN_StringList_dup(sl);
      }
    }
    else {
      const GWEN_STRINGLIST *sl;
      GWEN_STRINGLISTENTRY *se;

      sl=AH_Job_GetSigners(j);
      if (GWEN_StringList_Count(sl)!=GWEN_StringList_Count(jq->signers)) {
        DBG_INFO(AQHBCI_LOGDOMAIN,
                 "Number of signers of the job differs from that of the queue");
        return AH_JobQueueAddResultJobLimit;
      }
      se=GWEN_StringList_FirstEntry(sl);
      while(se) {
        if (!GWEN_StringList_HasString(jq->signers,
                                       GWEN_StringListEntry_Data(se))) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Signers of the job differ from those of the queue");
          return AH_JobQueueAddResultJobLimit;
        }
        se=GWEN_StringListEntry_Next(se);
      } /* while se */
    }

    /* adjust queue flags according to current job */
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_CRYPT)
      jq->flags|=AH_JOBQUEUE_FLAGS_CRYPT;
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_SIGN)
      jq->flags|=AH_JOBQUEUE_FLAGS_SIGN;
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_NEEDTAN)
      jq->flags|=AH_JOBQUEUE_FLAGS_NEEDTAN;
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_NOSYSID)
      jq->flags|=AH_JOBQUEUE_FLAGS_NOSYSID;
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_NOITAN)
      jq->flags|=AH_JOBQUEUE_FLAGS_NOITAN;
  }

  /* actually add job to queue */
  AH_Job_List_Add(j, jq->jobs);
  AH_Job_SetStatus(j, AH_JobStatusEnqueued);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Job added to the queue");
  return AH_JobQueueAddResultOk;
}



const AH_JOB_LIST *AH_JobQueue_GetJobList(const AH_JOBQUEUE *jq){
  assert(jq);
  assert(jq->usage);
  return jq->jobs;
}



AH_JOB_LIST *AH_JobQueue_TakeJobList(AH_JOBQUEUE *jq){
  AH_JOB_LIST *jl;

  assert(jq);
  assert(jq->usage);
  jl=jq->jobs;
  jq->jobs=AH_Job_List_new();
  return jl;
}



GWEN_TYPE_UINT32 AH_JobQueue_GetMsgNum(const AH_JOBQUEUE *jq){
  assert(jq);
  assert(jq->usage);
  return jq->msgNum;
}



AH_MSG *AH_JobQueue_ToMessage(AH_JOBQUEUE *jq, AH_DIALOG *dlg){
  AH_MSG *msg;
  AH_JOB *j;
  unsigned int encodedJobs;
  GWEN_STRINGLISTENTRY *se;
  int rv;

  assert(jq);
  assert(jq->usage);
  assert(dlg);

  if (!AH_Job_List_GetCount(jq->jobs)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Empty queue");
    return 0;
  }
  msg=AH_Msg_new(dlg);
  AH_Msg_SetHbciVersion(msg, AH_User_GetHbciVersion(jq->user));

  if (AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_NEEDTAN) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Queue needs a TAN");
  }
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Queue doesn't need a TAN");
  }
  AH_Msg_SetNeedTan(msg,
                    (AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_NEEDTAN));

  AH_Msg_SetNoSysId(msg,
                    (AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_NOSYSID));

  /* copy signers */

  if (AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_SIGN) {
    se=GWEN_StringList_FirstEntry(jq->signers);
    if (!se) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      AH_Msg_free(msg);
      return 0;
    }
    while(se) {
      GWEN_KEYSPEC *ks;

      ks=GWEN_KeySpec_new();
      GWEN_KeySpec_SetKeyType(ks, "RSA");
      GWEN_KeySpec_SetOwner(ks, GWEN_StringListEntry_Data(se));
      GWEN_KeySpec_SetKeyName(ks, "S");
      GWEN_KeySpec_SetNumber(ks, 999);
      GWEN_KeySpec_SetVersion(ks, 999);
      AH_Msg_AddSigner(msg, ks);
      GWEN_KeySpec_free(ks);
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* copy crypter */
  if (jq->flags & AH_JOBQUEUE_FLAGS_CRYPT) {
    GWEN_KEYSPEC *ks;

    ks=GWEN_KeySpec_new();
    GWEN_KeySpec_SetKeyType(ks, "RSA");
    /* The name doesn't matter here, since jobs are only used by clients
     * and the client code for getMedium always uses the name of the dialog
     * owner instead of the name from the keyspec when retrieving the medium
     * for encryption.
     */
    GWEN_KeySpec_SetOwner(ks, "");
    GWEN_KeySpec_SetKeyName(ks, "V");
    GWEN_KeySpec_SetNumber(ks, 999);
    GWEN_KeySpec_SetVersion(ks, 999);
    AH_Msg_SetCrypter(msg, ks);
    GWEN_KeySpec_free(ks);
  }

  encodedJobs=0;
  j=AH_Job_List_First(jq->jobs);
  while(j) {
    AH_JOB_STATUS st;

    st=AH_Job_GetStatus(j);
    /* only encode jobs which have not already been sent or which have
     * have no errors
     */
    if (st==AH_JobStatusEnqueued) {
      unsigned int firstSeg;
      unsigned int lastSeg;
      GWEN_DB_NODE *jargs;
      GWEN_XMLNODE *jnode;
      GWEN_BUFFER *msgBuf;

      DBG_INFO(AQHBCI_LOGDOMAIN, "Encoding job \"%s\"", AH_Job_GetName(j));
      jargs=AH_Job_GetArguments(j);
      jnode=AH_Job_GetXmlNode(j);
      if (strcasecmp(GWEN_XMLNode_GetData(jnode), "message")==0) {
	const char *s;

	s=GWEN_XMLNode_GetProperty(jnode, "name", 0);
	if (s) {
          DBG_NOTICE(AQHBCI_LOGDOMAIN,
                     "Getting for message specific data (%s)", s);
	  jargs=GWEN_DB_GetGroup(jargs, GWEN_PATH_FLAGS_NAMEMUSTEXIST, s);
	  if (!jargs) {
	    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No message specific data");
	    jargs=AH_Job_GetArguments(j);
	  }
	}
      }

      firstSeg=AH_Msg_GetCurrentSegmentNumber(msg);
      msgBuf=AH_Msg_GetBuffer(msg);
      assert(msgBuf);
      lastSeg=AH_Msg_AddNode(msg, jnode, jargs);
      if (!lastSeg) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Could not encode job \"%s\"",
                 AH_Job_GetName(j));
        AH_Job_SetStatus(j, AH_JobStatusError);
      }
      else {
        AH_Job_SetFirstSegment(j, firstSeg);
        AH_Job_SetLastSegment(j, lastSeg);

        if (AH_Job_GetStatus(j)!=AH_JobStatusError) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" encoded",
                   AH_Job_GetName(j));
          AH_Job_SetStatus(j, AH_JobStatusEncoded);
          encodedJobs++;
        }
      }
    } /* if status matches */
    j=AH_Job_List_Next(j);
  } /* while */

  if (!encodedJobs) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No job encoded");
    AH_Msg_free(msg);
    return 0;
  }
  rv=AH_Msg_EncodeMsg(msg);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not encode message (%d)", rv);

    j=AH_Job_List_First(jq->jobs);
    while(j) {
      if (AH_Job_GetStatus(j)==AH_JobStatusEncoded)
        AH_Job_SetStatus(j, AH_JobStatusError);
      j=AH_Job_List_Next(j);
    } /* while */
    return 0;
  }

  /*
   * inform all jobs that they have been encoded
   * this is needed for multi-message jobs so that they can prepare
   * themselves for the next message
   */
  j=AH_Job_List_First(jq->jobs);
  AH_JobQueue_SetUsedTan(jq, AH_Msg_GetTan(msg));
  AH_JobQueue_SetUsedPin(jq, AH_Msg_GetPin(msg));
  while(j) {
    const char *s;

    if (AH_Job_GetStatus(j)==AH_JobStatusEncoded) {
      /* store some information about the message in the job */
      AH_Job_SetMsgNum(j, AH_Msg_GetMsgNum(msg));
      AH_Job_SetDialogId(j, AH_Dialog_GetDialogId(dlg));
      /* store expected signer and crypter (if any) */
      s=AH_Msg_GetExpectedSigner(msg);
      if (s) AH_Job_SetExpectedSigner(j, s);
      s=AH_Msg_GetExpectedCrypter(msg);
      if (s) AH_Job_SetExpectedCrypter(j, s);

      /* store used TAN (if any) */
      s=AH_Msg_GetTan(msg);
      if (s) AH_Job_SetUsedTan(j, s);
    }
    j=AH_Job_List_Next(j);
  } /* while */


  jq->msgNum=AH_Msg_GetMsgNum(msg);
  DBG_INFO(AQHBCI_LOGDOMAIN,
           "Job queue encoded and ready to be sent (msgNum=%d)",
           jq->msgNum);
  return msg;
}



int AH_JobQueue__CheckTans(AH_JOBQUEUE *jq){
  AH_JOB *j;

  assert(jq);
  j=AH_Job_List_First(jq->jobs);
  while(j) {
    const char *tan;
    AB_USER *u;
    AH_MEDIUM *m;

    u=AH_Job_GetUser(j);
    assert(u);
    m=AH_User_GetMedium(u);
    assert(m);

    if (!AH_Medium_IsMounted(m)) {
      int rv;

      rv=AH_HBCI_GetMedium(AH_Job_GetHbci(j), u, &m);
      if (rv) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Could not mount medium (%d)", rv);
        return rv;
      }
    }

    tan=AH_Job_GetUsedTan(j);
    if (tan) {
      int rv;

      if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_TANUSED) {
        DBG_INFO(AQHBCI_LOGDOMAIN,
                 "TAN \"%s\" used", tan);
        rv=AH_Medium_SetTanStatus(m, tan, AB_Banking_TanStatusUsed);
      }
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN,
                 "TAN \"%s\" not used", tan);
        rv=AH_Medium_SetTanStatus(m, tan, AB_Banking_TanStatusUnused);
      }
      if (rv) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Error adjusting TAN status (%d)", rv);
        return rv;
      }
    } /* if tan */
    j=AH_Job_List_Next(j);
  }

  return 0;
}



void AH_JobQueue__AddAsUtf8(GWEN_BUFFER *buf, const char *txt) {
  while(*txt) {
    switch((unsigned char)(*txt)) {
    case 0xc4: /* AE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x84);
      break;
    case 0xd6: /* OE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x96);
      break;
    case 0xdc: /* UE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9c);
      break;
    case 0xdf: /* sharp-s */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9f);
      break;
    case 0xe4: /* ae */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa4);
      break;
    case 0xf6: /* oe */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xb6);
      break;
    case 0xfc: /* ue */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xbc);
      break;
    default:
      if (isascii(*txt))
        GWEN_Buffer_AppendByte(buf, *txt);
      else
        GWEN_Buffer_AppendByte(buf, ' ');
    }
    txt++;
  } /* while */
}


int AH_JobQueue_DispatchMessage(AH_JOBQUEUE *jq,
                                AH_MSG *msg,
                                GWEN_DB_NODE *db) {
  GWEN_DB_NODE *dbSecurity;
  GWEN_DB_NODE *dbCurr;
  const GWEN_KEYSPEC *ks;
  AH_JOB *j;
  AH_DIALOG *dlg;
  const char *p;
  int tanRecycle;
  int rv;
  int dialogAborted=0;
  int abortQueue=0;

  assert(jq);
  assert(jq->usage);
  assert(msg);
  assert(db);

  dlg=AH_Msg_GetDialog(msg);
  assert(dlg);

  /* log all results */
  tanRecycle=0;
  dbCurr=GWEN_DB_GetFirstGroup(db);
  while(dbCurr) {
    if (strcasecmp(GWEN_DB_GroupName(dbCurr), "SegResult")==0 ||
        strcasecmp(GWEN_DB_GroupName(dbCurr), "MsgResult")==0) {
      int rcode;
      const char *p;
      int isMsgResult;
      GWEN_BUFFER *logmsg;
      AB_BANKING_LOGLEVEL level;
      GWEN_DB_NODE *dbResult;

      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found a result");

      level=AB_Banking_LogLevelNotice;
      isMsgResult=(strcasecmp(GWEN_DB_GroupName(dbCurr),
                              "MsgResult")==0);

      dbResult=GWEN_DB_FindFirstGroup(dbCurr, "result");
      while(dbResult) {
        rcode=GWEN_DB_GetIntValue(dbResult, "resultcode", 0, 0);
        p=GWEN_DB_GetCharValue(dbResult, "text", 0, "");
        if (rcode>=9000 && rcode<10000) {
	  DBG_INFO(AQHBCI_LOGDOMAIN,
		   "Result: Error (%d: %s)", rcode, p);
	  level=AB_Banking_LogLevelError;
	  if (isMsgResult) {
	    if (rcode==9800)
	      dialogAborted=1;
	    else if (rcode>9300 && rcode<9400)
	      abortQueue=1;
	  }
	}
        else if (rcode>=3000 && rcode<4000) {
	  DBG_INFO(AQHBCI_LOGDOMAIN,
		   "Result: Warning (%d: %s)", rcode, p);
          if (rcode==3910)
            tanRecycle=1;
          level=AB_Banking_LogLevelWarn;
        }
        else {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Segment result: Ok (%d: %s)", rcode, p);
          level=AB_Banking_LogLevelNotice;
        }

        logmsg=GWEN_Buffer_new(0, 256, 0, 1);
        if (p) {
          char numbuf[16];

          GWEN_Buffer_AppendString(logmsg, "HBCI: ");
          snprintf(numbuf, sizeof(numbuf), "%04d", rcode);
          GWEN_Buffer_AppendString(logmsg, numbuf);
          GWEN_Buffer_AppendString(logmsg, " - ");
          AH_JobQueue__AddAsUtf8(logmsg, p);
          if (isMsgResult)
            GWEN_Buffer_AppendString(logmsg, " (M)");
          else
            GWEN_Buffer_AppendString(logmsg, " (S)");
        }
        else {
          char numbuf[16];

          GWEN_Buffer_AppendString(logmsg, "HBCI: ");
          snprintf(numbuf, sizeof(numbuf), "%04d", rcode);
          GWEN_Buffer_AppendString(logmsg, numbuf);
          GWEN_Buffer_AppendString(logmsg, " - (no text)");
          if (isMsgResult)
            GWEN_Buffer_AppendString(logmsg, " (M)");
          else
            GWEN_Buffer_AppendString(logmsg, " (S)");
        }
        AB_Banking_ProgressLog(AH_Dialog_GetBankingApi(dlg),
                               0,
                               level,
                               GWEN_Buffer_GetStart(logmsg));
        GWEN_Buffer_free(logmsg);
        dbResult=GWEN_DB_FindNextGroup(dbResult, "result");
      } /* while results */
    }
  
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }


  /* prepare security group */
  dbSecurity=GWEN_DB_Group_new("security");
  p=AH_Dialog_GetDialogId(dlg);
  assert(p);
  GWEN_DB_SetIntValue(dbSecurity,
                      GWEN_DB_FLAGS_DEFAULT,
                      "msgnum",
                      AH_Msg_GetMsgNum(msg));
  GWEN_DB_SetCharValue(dbSecurity, GWEN_DB_FLAGS_DEFAULT, "dialogId", p);

  /* get all signers */
  ks=GWEN_KeySpec_List_First(AH_Msg_GetSigners(msg));
  while(ks) {
    const char *p;

    p=GWEN_KeySpec_GetOwner(ks);
    if (!p) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signer with no name?!");
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Adding signer \"%s\"", p);
      GWEN_DB_SetCharValue(dbSecurity, GWEN_DB_FLAGS_DEFAULT, "signer", p);
    }
    ks=GWEN_KeySpec_List_Next(ks);
  } /* while */

  /* set crypter */
  ks=AH_Msg_GetCrypter(msg);
  if (ks) {
    const char *p;

    p=GWEN_KeySpec_GetOwner(ks);
    if (!p) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Crypter with no name?!");
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Storing crypter \"%s\"", p);
      GWEN_DB_SetCharValue(dbSecurity, GWEN_DB_FLAGS_DEFAULT, "crypter", p);
    }
  }

  /* remove attach points of all jobs */
  j=AH_Job_List_First(jq->jobs);
  while(j) {
    AH_JOB_STATUS st;

    st=AH_Job_GetStatus(j);
    if (st==AH_JobStatusSent) {
      if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_ATTACHABLE) {
        GWEN_DB_NODE *args;

        AH_Job_SubFlags(j, AH_JOB_FLAGS_HASATTACHPOINT);

        /* remove the attach point */
        args=AH_Job_GetArguments(j);
        if (GWEN_DB_DeleteVar(args, "attach")) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Attach point removed");
        }
      } /* if job is attachable */
    } /* if status matches */

    j=AH_Job_List_Next(j);
  } /* while */

  dbCurr=GWEN_DB_GetFirstGroup(db);
  while(dbCurr) {
    GWEN_DB_NODE *dbResponse;
    GWEN_DB_NODE *dbData;
    int segNum;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Handling response \"%s\"",
             GWEN_DB_GroupName(dbCurr));

    dbResponse=GWEN_DB_Group_new("response");
    /* add security group */
    GWEN_DB_AddGroup(dbResponse, GWEN_DB_Group_dup(dbSecurity));
    /* create data group */
    dbData=GWEN_DB_GetGroup(dbResponse, GWEN_DB_FLAGS_DEFAULT, "data");
    assert(dbData);
    /* store copy of original response there */
    GWEN_DB_AddGroup(dbData, GWEN_DB_Group_dup(dbCurr));

    segNum=GWEN_DB_GetIntValue(dbCurr, "head/ref", 0, 0);
    if (segNum) {

      /* search for job to which this response belongs */
      j=AH_Job_List_First(jq->jobs);
      while(j) {
        AH_JOB_STATUS st;

        st=AH_Job_GetStatus(j);
        if (st==AH_JobStatusSent ||
	    st==AH_JobStatusAnswered) {
	  DBG_INFO(AQHBCI_LOGDOMAIN,
		   "Checking whether job \"%s\" has segment %d",
		   AH_Job_GetName(j), segNum);
	  if ((AH_Msg_GetMsgNum(msg)==AH_Job_GetMsgNum(j)) &&
	      AH_Job_HasSegment(j, segNum)) {
	    DBG_INFO(AQHBCI_LOGDOMAIN,
		     "Job \"%s\" claims to have the segment %d",
		     AH_Job_GetName(j), segNum);
	    break;
	  }
        }
        else {
	  DBG_INFO(AQHBCI_LOGDOMAIN,
		   "Skipping job \"%s\" because of status \"%s\" (%d)",
		   AH_Job_GetName(j), AH_Job_StatusName(st), st);
	}
        j=AH_Job_List_Next(j);
      } /* while */
      if (j) {
        /* check for attachability */
        if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_ATTACHABLE) {
	  /* job is attachable, check whether this is segment result */
	  if (strcasecmp(GWEN_DB_GroupName(dbCurr), "SegResult")==0) {
            GWEN_DB_NODE *dbResult;

	    dbResult=GWEN_DB_FindFirstGroup(dbCurr, "result");
	    while(dbResult) {
	      int rcode;
  
	      rcode=GWEN_DB_GetIntValue(dbResult, "resultcode", 0, 0);
	      /* it is a segment result, does it contain an attach point ? */
	      if (rcode==3040) {
		const char *p;
  
		/* it should... */
		p=GWEN_DB_GetCharValue(dbResult, "param", 0, 0);
		if (!p) {
		  DBG_ERROR(AQHBCI_LOGDOMAIN,
			    "Segment result 3040 without attachpoint");
		}
		else {
		  GWEN_DB_NODE *args;

		  /* store the attach point */
		  DBG_INFO(AQHBCI_LOGDOMAIN, "Storing attach point");
		  args=AH_Job_GetArguments(j);
		  GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS,
				       "attach", p);
		  AH_Job_AddFlags(j, AH_JOB_FLAGS_HASATTACHPOINT);
		}
	      } /* if code 3040 (means "more data available") */
	      dbResult=GWEN_DB_FindNextGroup(dbResult, "result");
	    } /* while */
          } /* if segresult */
        } /* if attachable */

        /* check for segment results */
        if (strcasecmp(GWEN_DB_GroupName(dbCurr), "SegResult")==0) {
	  GWEN_DB_NODE *dbResult;

	  dbResult=GWEN_DB_FindFirstGroup(dbCurr, "result");
	  while(dbResult) {
	    int rcode;
	    const char *p;
  
	    rcode=GWEN_DB_GetIntValue(dbResult, "resultcode", 0, 0);
	    p=GWEN_DB_GetCharValue(dbResult, "text", 0, "");
	    if (rcode>=9000 && rcode<10000) {
	      DBG_INFO(AQHBCI_LOGDOMAIN,
		       "Segment result: Error (%d: %s)", rcode, p);
	      AH_Job_AddFlags(j, AH_JOB_FLAGS_HASERRORS);
	      AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_HASERRORS);
	    }
	    else if (rcode>=3000 && rcode<4000) {
	      DBG_INFO(AQHBCI_LOGDOMAIN,
		       "Segment result: Warning (%d: %s)", rcode, p);
	      AH_Job_AddFlags(j, AH_JOB_FLAGS_HASWARNINGS);
	      AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_HASWARNINGS);
	    }
	    else {
	      DBG_INFO(AQHBCI_LOGDOMAIN,
		       "Segment result: Ok (%d: %s)", rcode, p);
	    }
	    dbResult=GWEN_DB_FindNextGroup(dbResult, "result");
	  } /* while */
	} /* if SegResult */

        DBG_INFO(AQHBCI_LOGDOMAIN, "Adding response \"%s\" to job \"%s\"",
                 GWEN_DB_GroupName(dbCurr),
                 AH_Job_GetName(j));
        AH_Job_AddResponse(j, dbResponse);
        AH_Job_SetStatus(j, AH_JobStatusAnswered);
      } /* if matching job found */
      else {
        GWEN_TYPE_UINT32 plusFlags;

	DBG_WARN(AQHBCI_LOGDOMAIN,
		 "No job found, adding response \"%s\" to all jobs",
		 GWEN_DB_GroupName(dbCurr));

        /* add response to all jobs (as queue response) and to queue */
        plusFlags=0;
        if (strcasecmp(GWEN_DB_GroupName(dbCurr), "MsgResult")==0) {
          int rcode;
          const char *p;
          /* FIXME: This code will never be used, I guess, since
           * a MsgResult wil most likely not have a reference segment... */
          rcode=GWEN_DB_GetIntValue(dbCurr, "result/resultcode", 0, 0);
          p=GWEN_DB_GetCharValue(dbCurr, "result/text", 0, "");
          if (rcode>=9000 && rcode<10000) {
            DBG_INFO(AQHBCI_LOGDOMAIN, "Msg result: Error (%d: %s)", rcode, p);
            plusFlags|=AH_JOB_FLAGS_HASERRORS;
            AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_HASERRORS);
          }
          else if (rcode>=3000 && rcode<4000) {
            DBG_INFO(AQHBCI_LOGDOMAIN, "Msg result: Warning (%d: %s)", rcode, p);
            plusFlags|=AH_JOB_FLAGS_HASWARNINGS;
            AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_HASWARNINGS);
          }
          else {
            DBG_INFO(AQHBCI_LOGDOMAIN, "Msg result: Ok (%d: %s)", rcode, p);
          }
        }
        else if (strcasecmp(GWEN_DB_GroupName(dbCurr), "SegResult")==0) {
          GWEN_DB_NODE *dbResult;

          dbResult=GWEN_DB_FindFirstGroup(dbCurr, "result");
          while(dbResult) {
            int rcode;

            rcode=GWEN_DB_GetIntValue(dbResult, "resultcode", 0, 0);
            if (rcode==9340) {
              DBG_ERROR(AQHBCI_LOGDOMAIN,
                        "Found a segresult: %d", rcode);
              if (jq->usedPin) {
                AH_MEDIUM *m;

                DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad pin");
                m=AH_User_GetMedium(jq->user);
                assert(m);
                if (AH_User_GetCryptMode(jq->user)==AH_CryptMode_Pintan) {
                  DBG_INFO(AQHBCI_LOGDOMAIN, "Marking pin as bad");
                  AH_Medium_SetPinStatus(m,
                                         jq->usedPin,
                                         AB_Banking_PinStatusBad);
                }
              }
            }
            dbResult=GWEN_DB_FindNextGroup(dbResult, "result");
          } /* while */
        } /* if segresult */

        j=AH_Job_List_First(jq->jobs);
        while(j) {
          AH_JOB_STATUS st;

          st=AH_Job_GetStatus(j);
          if (st==AH_JobStatusSent ||
              st==AH_JobStatusAnswered) {
	    AH_Job_AddFlags(j, plusFlags);
            AH_Job_AddResponse(j, GWEN_DB_Group_dup(dbResponse));
            AH_Job_SetStatus(j, AH_JobStatusAnswered);
          }
          else {
            DBG_ERROR(AQHBCI_LOGDOMAIN,
                      "Status %d of job doesn't match", st);
          }
          j=AH_Job_List_Next(j);
        } /* while */
        GWEN_DB_Group_free(dbResponse);
      }
    }
    else {
      GWEN_TYPE_UINT32 plusFlags;

      /* no reference segment number, add response to all jobs and
       * to the queue */
      DBG_DEBUG(AQHBCI_LOGDOMAIN,
                "No segment reference number, "
                "adding response \"%s\" to all jobs",
               GWEN_DB_GroupName(dbCurr));

      /* add response to all jobs (as queue response) and to queue */
      plusFlags=0;
      if (strcasecmp(GWEN_DB_GroupName(dbCurr), "MsgResult")==0) {
        int rcode;
        const char *p;

        rcode=GWEN_DB_GetIntValue(dbCurr, "result/resultcode", 0, 0);
        p=GWEN_DB_GetCharValue(dbCurr, "result/text", 0, "");
        if (rcode>=9000 && rcode<10000) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Msg result: Error (%d: %s)", rcode, p);
          plusFlags|=AH_JOB_FLAGS_HASERRORS;
          AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_HASERRORS);
        }
        else if (rcode>=3000 && rcode<4000) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Msg result: Warning (%d: %s)", rcode, p);
          plusFlags|=AH_JOB_FLAGS_HASWARNINGS;
          AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_HASWARNINGS);
        }
        else {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Msg result: Ok (%d: %s)", rcode, p);
        }
      }

      j=AH_Job_List_First(jq->jobs);
      while(j) {
        AH_JOB_STATUS st;

        st=AH_Job_GetStatus(j);
        if (st==AH_JobStatusSent ||
            st==AH_JobStatusAnswered) {
          AH_Job_AddFlags(j, plusFlags);
          AH_Job_AddResponse(j, GWEN_DB_Group_dup(dbResponse));
          AH_Job_SetStatus(j, AH_JobStatusAnswered);
        }
        j=AH_Job_List_Next(j);
      } /* while */
      GWEN_DB_Group_free(dbResponse);
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  /* set usedTan status accordingly */
  j=AH_Job_List_First(jq->jobs);
  while(j) {
    const char *utan;

    utan=AH_Job_GetUsedTan(j);
    if (utan) {
      AH_JOB_STATUS st;

      AH_Job_AddFlags(j, AH_JOB_FLAGS_TANUSED);
      st=AH_Job_GetStatus(j);
      if (st==AH_JobStatusSent ||
          st==AH_JobStatusAnswered) {
        if (tanRecycle)
          AH_Job_SubFlags(j, AH_JOB_FLAGS_TANUSED);
      }
      else {
        DBG_WARN(AQHBCI_LOGDOMAIN, "Bad status %d", st);
      }
    }
    else {
      DBG_WARN(AQHBCI_LOGDOMAIN, "No TAN");
    }
    j=AH_Job_List_Next(j);
  } /* while */

  /* tell the application/medium about used and unused TANs */
  rv=AH_JobQueue__CheckTans(jq);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error checking TANs (%d)", rv);
  }
  if (dialogAborted) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Dialog logically aborted by peer");
    return AB_ERROR_ABORTED;
  }

  if (abortQueue) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN,
	       "Aborting queue");
    return AB_ERROR_ABORTED;
  }

  return rv;
}



GWEN_TYPE_UINT32 AH_JobQueue_GetFlags(AH_JOBQUEUE *jq){
  assert(jq);
  assert(jq->usage);
  return jq->flags;
}



void AH_JobQueue_SetFlags(AH_JOBQUEUE *jq, GWEN_TYPE_UINT32 f){
  assert(jq);
  assert(jq->usage);
  jq->flags=f;
}



void AH_JobQueue_AddFlags(AH_JOBQUEUE *jq, GWEN_TYPE_UINT32 f){
  assert(jq);
  assert(jq->usage);
  jq->flags|=f;
}



void AH_JobQueue_SubFlags(AH_JOBQUEUE *jq, GWEN_TYPE_UINT32 f){
  assert(jq);
  assert(jq->usage);
  jq->flags&=~f;
}



AB_USER *AH_JobQueue_GetUser(const AH_JOBQUEUE *jq){
  assert(jq);
  assert(jq->usage);
  return jq->user;
}



void AH_JobQueue_SetJobStatusOnMatch(AH_JOBQUEUE *jq,
                                     AH_JOB_STATUS matchSt,
                                     AH_JOB_STATUS newSt) {
  AH_JOB *j;

  assert(jq);
  assert(jq->usage);

  j=AH_Job_List_First(jq->jobs);
  while(j) {
    if (matchSt==AH_JobStatusAll ||
        AH_Job_GetStatus(j)==matchSt)
      AH_Job_SetStatus(j, newSt);
    j=AH_Job_List_Next(j);
  } /* while */
}



void AH_JobQueue_Dump(AH_JOBQUEUE *jq, FILE *f, unsigned int insert) {
  GWEN_TYPE_UINT32 k;
  AH_JOB *j;
  GWEN_STRINGLISTENTRY *se;


  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "JobQueue:\n");

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Usage   : %d\n", jq->usage);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Owner   : %s\n", AB_User_GetCustomerId(jq->user));

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Flags: %08x ( ", jq->flags);
  if (jq->flags & AH_JOBQUEUE_FLAGS_CRYPT)
    fprintf(f, "CRYPT ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_SIGN)
    fprintf(f, "SIGN ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_BEGINDIALOG)
    fprintf(f, "BEGINDIALOG ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_ENDDIALOG)
    fprintf(f, "ENDDIALOG ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_ISDIALOG)
    fprintf(f, "ISDIALOG ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_OUTBOX)
    fprintf(f, "OUTBOX ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_HASWARNINGS)
    fprintf(f, "HASWARNINGS ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_HASERRORS)
    fprintf(f, "HASERRORS ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_DLGSTARTED)
    fprintf(f, "DLGSTARTED ");
  fprintf(f, ")\n");

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Signers:\n");

  se=GWEN_StringList_FirstEntry(jq->signers);
  while(se) {
    for (k=0; k<insert; k++)
      fprintf(f, " ");
    fprintf(f, "  \"%s\"\n", GWEN_StringListEntry_Data(se));
    se=GWEN_StringListEntry_Next(se);
  } /* while se */

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Jobs:\n");
  j=AH_Job_List_First(jq->jobs);
  while(j) {
    AH_Job_Dump(j, f, insert+2);
    j=AH_Job_List_Next(j);
  } /* while j */
}


unsigned int AH_JobQueue_GetCount(const AH_JOBQUEUE *jq){
  assert(jq);
  assert(jq->usage);
  return AH_Job_List_GetCount(jq->jobs);
}




