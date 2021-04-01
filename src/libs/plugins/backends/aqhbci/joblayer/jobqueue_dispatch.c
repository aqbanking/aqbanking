/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "jobqueue_dispatch.h"

#include "aqhbci/banking/user_l.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static void _scanAllResultSegments(AH_JOBQUEUE *jq, GWEN_DB_NODE *db, uint32_t guiid);
static GWEN_DB_NODE *_sampleSecuritySegments(AH_JOBQUEUE *jq, AH_MSG *msg, GWEN_DB_NODE *db);
static void _removeAttachPoints(const AH_JOBQUEUE *jq);
static void _setUsedTanStatusInJobs(const AH_JOBQUEUE *jq);
static void _adjustSystemTanStatus(AH_JOBQUEUE *jq, uint32_t guiid);
static AH_JOB *_findReferencedJob(AH_JOBQUEUE *jq, int refMsgNum, int refSegNum);
static void _possiblyExtractJobAckCode(AH_JOB *j, GWEN_DB_NODE *dbSegment);
static void _possiblyExtractAttachPoint(AH_JOB *j, GWEN_DB_NODE *dbSegment);
static void _handleSegmentResultForAllJobs(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbSegment);
static void _handleSegmentResult(AH_JOBQUEUE *jq, AH_JOB *j, GWEN_DB_NODE *dbSegment);
static void _addResponseToAllJobs(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbPreparedJobResponse);
static void _handleResponseSegments(AH_JOBQUEUE *jq, AH_MSG *msg, GWEN_DB_NODE *db, GWEN_DB_NODE *dbSecurity);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_JobQueue_DispatchMessage(AH_JOBQUEUE *jq, AH_MSG *msg, GWEN_DB_NODE *db)
{
  GWEN_DB_NODE *dbSecurity;
  AH_DIALOG *dlg;
  uint32_t guiid;
  uint32_t jobQueueFlags;

  assert(jq);
  assert(msg);
  assert(db);

  dlg=AH_Msg_GetDialog(msg);
  assert(dlg);
  guiid=0;

  _removeAttachPoints(jq);
  _scanAllResultSegments(jq, db, guiid);

  dbSecurity=_sampleSecuritySegments(jq, msg, db);
  _handleResponseSegments(jq, msg, db, dbSecurity);
  GWEN_DB_Group_free(dbSecurity);

  _setUsedTanStatusInJobs(jq);
  _adjustSystemTanStatus(jq, guiid);

  jobQueueFlags=AH_JobQueue_GetFlags(jq);

  if (jobQueueFlags & (AH_JOBQUEUE_FLAGS_ACCESS_PROBLEM | AH_JOBQUEUE_FLAGS_DIALOG_ABORTED)) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Dialog logically aborted by peer, assuming bad PIN");
    if (AH_JobQueue_GetUsedPin(jq)) {
      GWEN_Gui_ProgressLog(guiid,
                           GWEN_LoggerLevel_Info,
                           I18N("Dialog aborted by bank, assuming bad PIN"));
      AH_User_SetPinStatus(AH_JobQueue_GetUser(jq), AH_JobQueue_GetUsedPin(jq), GWEN_Gui_PasswordStatus_Bad);
    }
    return GWEN_ERROR_ABORTED;
  }

  if (!(jobQueueFlags & AH_JOBQUEUE_FLAGS_BAD_PIN)) {
    if (AH_JobQueue_GetUsedPin(jq)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Dialog not aborted, assuming correct PIN");
      GWEN_Gui_ProgressLog(guiid,
                           GWEN_LoggerLevel_Info,
                           I18N("Dialog not aborted, assuming PIN is ok"));
      AH_User_SetPinStatus(AH_JobQueue_GetUser(jq), AH_JobQueue_GetUsedPin(jq), GWEN_Gui_PasswordStatus_Ok);
    }
  }

  return 0;
}



void _scanAllResultSegments(AH_JOBQUEUE *jq, GWEN_DB_NODE *db, uint32_t guiid)
{
  AB_USER *user;
  GWEN_DB_NODE *dbCurr;

  user=AH_JobQueue_GetUser(jq);

  dbCurr=GWEN_DB_GetFirstGroup(db);
  while (dbCurr) {
    if (strcasecmp(GWEN_DB_GroupName(dbCurr), "SegResult")==0 ||
        strcasecmp(GWEN_DB_GroupName(dbCurr), "MsgResult")==0) {
      int rcode;
      const char *p;
      int isMsgResult;
      GWEN_BUFFER *logmsg;
      GWEN_LOGGER_LEVEL level;
      GWEN_DB_NODE *dbResult;

      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found a result");

      level=GWEN_LoggerLevel_Notice;
      isMsgResult=(strcasecmp(GWEN_DB_GroupName(dbCurr), "MsgResult")==0);

      dbResult=GWEN_DB_FindFirstGroup(dbCurr, "result");
      while (dbResult) {
        rcode=GWEN_DB_GetIntValue(dbResult, "resultcode", 0, 0);
        p=GWEN_DB_GetCharValue(dbResult, "text", 0, "");

        if (rcode>=9000 && rcode<10000) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Result: Error (%d: %s)", rcode, p);
          AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_HASERRORS);
          level=GWEN_LoggerLevel_Error;

          if (isMsgResult) {
            if (rcode==9800)
              AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_DIALOG_ABORTED);
            else if (rcode>9300 && rcode<9400)
              AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_ACCESS_PROBLEM);
          }

          /* check for bad pins here */
          if (rcode==9340 || rcode==9942) {
            DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad PIN flagged: %d", rcode);
            AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_BAD_PIN | AH_JOBQUEUE_FLAGS_DIALOG_ABORTED);
            if (AH_JobQueue_GetUsedPin(jq)) {
              GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Error, I18N("PIN invalid according to server"));
              AH_User_SetPinStatus(user, AH_JobQueue_GetUsedPin(jq), GWEN_Gui_PasswordStatus_Bad);
            }
          }
        }
        else if (rcode>=3000 && rcode<4000) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Result: Warning (%d: %s)", rcode, p);
          AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_HASWARNINGS);
          level=GWEN_LoggerLevel_Warning;
          if (rcode==3910)
            AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_RECYCLE_TAN);
          else if (rcode==3920) {
            int i;

            AH_User_ClearTanMethodList(user);
            for (i=0; ; i++) {
              int j;

              j=GWEN_DB_GetIntValue(dbResult, "param", i, 0);
              if (j==0)
                break;
              AH_User_AddTanMethod(user, j);
            } /* for */
            if (i==0)
              /* add single step if empty list */
              AH_User_AddTanMethod(user, 999);
          }
        }
        else {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Segment result: Ok (%d: %s)", rcode, p);
          level=GWEN_LoggerLevel_Notice;
        }

        logmsg=GWEN_Buffer_new(0, 256, 0, 1);
        if (p)
          GWEN_Buffer_AppendArgs(logmsg, "HBCI: %04d - %s (%s)", rcode, p, isMsgResult?"M":"S");
        else
          GWEN_Buffer_AppendArgs(logmsg, "HBCI: %04d - (no text) (%s)", rcode, isMsgResult?"M":"S");
        GWEN_Gui_ProgressLog(guiid, level, GWEN_Buffer_GetStart(logmsg));
        GWEN_Buffer_free(logmsg);

        dbResult=GWEN_DB_FindNextGroup(dbResult, "result");
      } /* while results */
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }
}



GWEN_DB_NODE *_sampleSecuritySegments(AH_JOBQUEUE *jq, AH_MSG *msg, GWEN_DB_NODE *db)
{
  AH_DIALOG *dlg;
  const GWEN_STRINGLIST *msgSignerList;
  GWEN_DB_NODE *dbSecurity;
  const char *p;

  dlg=AH_Msg_GetDialog(msg);

  /* prepare security group */
  dbSecurity=GWEN_DB_Group_new("security");
  p=AH_Dialog_GetDialogId(dlg);
  assert(p);
  GWEN_DB_SetIntValue(dbSecurity, GWEN_DB_FLAGS_DEFAULT, "msgnum", AH_Msg_GetMsgNum(msg));
  GWEN_DB_SetCharValue(dbSecurity, GWEN_DB_FLAGS_DEFAULT, "dialogId", p);

  /* get all signers */
  msgSignerList=AH_Msg_GetSignerIdList(msg);
  if (msgSignerList) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(AH_Msg_GetSignerIdList(msg));
    while (se) {
      const char *p;

      p=GWEN_StringListEntry_Data(se);
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding signer \"%s\"", p);
      GWEN_DB_SetCharValue(dbSecurity, GWEN_DB_FLAGS_DEFAULT, "signer", p);
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* set crypter */
  p=AH_Msg_GetCrypterId(msg);
  if (p) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Storing crypter \"%s\"", p);
    GWEN_DB_SetCharValue(dbSecurity, GWEN_DB_FLAGS_DEFAULT, "crypter", p);
  }

  return dbSecurity;
}



void _removeAttachPoints(const AH_JOBQUEUE *jq)
{
  AH_JOB *j;

  /* remove attach points of all jobs */
  j=AH_JobQueue_GetFirstJob(jq);
  while (j) {
    AH_JOB_STATUS st;

    st=AH_Job_GetStatus(j);
    if (st==AH_JobStatusSent) {
      if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_ATTACHABLE) {
        GWEN_DB_NODE *args;

        AH_Job_SubFlags(j, AH_JOB_FLAGS_HASATTACHPOINT);

        /* remove the attach point */
        args=AH_Job_GetArguments(j);
        if (GWEN_DB_DeleteVar(args, "attach")) {
          DBG_DEBUG(AQHBCI_LOGDOMAIN, "Attach point removed");
        }
      } /* if job is attachable */
    } /* if status matches */

    j=AH_Job_List_Next(j);
  } /* while */
}



void _setUsedTanStatusInJobs(const AH_JOBQUEUE *jq)
{
  AH_JOB *j;

  /* set usedTan status accordingly */
  j=AH_JobQueue_GetFirstJob(jq);
  while (j) {
    const char *usedTan;

    usedTan=AH_Job_GetUsedTan(j);
    if (usedTan) {
      AH_JOB_STATUS st;

      AH_Job_AddFlags(j, AH_JOB_FLAGS_TANUSED);
      st=AH_Job_GetStatus(j);
      if (st==AH_JobStatusSent || st==AH_JobStatusAnswered) {
        if (AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_RECYCLE_TAN)
          AH_Job_SubFlags(j, AH_JOB_FLAGS_TANUSED);
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No TAN in job [%s]", AH_Job_GetName(j));
    }
    j=AH_Job_List_Next(j);
  } /* while */
}



void _adjustSystemTanStatus(AH_JOBQUEUE *jq, uint32_t guiid)
{
  AH_JOB *j;

  assert(jq);
  j=AH_JobQueue_GetFirstJob(jq);
  while (j) {
    const char *tan;
    AB_USER *user;

    user=AH_Job_GetUser(j);
    assert(user);

    tan=AH_Job_GetUsedTan(j);
    if (tan) {
      int rv;

      if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_TANUSED) {
        char tbuf[256];

        DBG_INFO(AQHBCI_LOGDOMAIN, "TAN \"%s\" used", tan);
        snprintf(tbuf, sizeof(tbuf)-1, I18N("TAN \"%s\" has been used, please strike it out."), tan);
        tbuf[sizeof(tbuf)-1]=0;
        GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Notice, tbuf);
        rv=AH_User_SetTanStatus(user, NULL, tan, GWEN_Gui_PasswordStatus_Used);
      }
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN, "TAN not used");
        rv=AH_User_SetTanStatus(user, NULL, tan, GWEN_Gui_PasswordStatus_Unused);
      }
      if (rv) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Error adjusting TAN status (%d), ignoring", rv);
        /*return rv;*/
      }
    } /* if tan */
    j=AH_Job_List_Next(j);
  }
}


AH_JOB *_findReferencedJob(AH_JOBQUEUE *jq, int refMsgNum, int refSegNum)
{
  AH_JOB *j;

  j=AH_JobQueue_GetFirstJob(jq);
  while (j) {
    AH_JOB_STATUS jobStatus;
    const char *jobName;

    jobName=AH_Job_GetName(j);
    jobStatus=AH_Job_GetStatus(j);

    if (jobStatus==AH_JobStatusSent || jobStatus==AH_JobStatusAnswered) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Checking whether job \"%s\" has segment %d", jobName, refSegNum);
      if ((AH_Job_GetMsgNum(j)==refMsgNum) && AH_Job_HasSegment(j, refSegNum)) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" claims to have the segment %d for msg %d", jobName, refSegNum, refMsgNum);
        return j;
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Skipping job \"%s\" because of status \"%s\" (%d)",
               jobName, AH_Job_StatusName(jobStatus), jobStatus);
    }
    j=AH_Job_List_Next(j);
  } /* while j */

  return NULL;
}


void _possiblyExtractJobAckCode(AH_JOB *j, GWEN_DB_NODE *dbSegment) {
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_ACKNOWLEDGE) {
    const char *responseName=AH_Job_GetResponseName(j);
    if (strcasecmp(GWEN_DB_GroupName(dbSegment), responseName)==0) {
      unsigned int byteSize = 0;
      const void* ackCode;
      ackCode = GWEN_DB_GetBinValue(dbSegment, "ackCode", 0, NULL, 0, &byteSize);
      if (ackCode) {
        GWEN_DB_NODE *args = AH_Job_GetArguments(j);
        GWEN_DB_SetBinValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "_tmpAckCode", ackCode, byteSize);
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found acknoledge code in job response, storing it.");
      }
    }
  }
}


void _possiblyExtractAttachPoint(AH_JOB *j, GWEN_DB_NODE *dbSegment)
{
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_ATTACHABLE) {
    /* job is attachable, check whether this is segment result */
    if (strcasecmp(GWEN_DB_GroupName(dbSegment), "SegResult")==0) {
      GWEN_DB_NODE *dbResult;

      dbResult=GWEN_DB_FindFirstGroup(dbSegment, "result");
      while (dbResult) {
        int rcode;

        rcode=GWEN_DB_GetIntValue(dbResult, "resultcode", 0, 0);
        /* it is a segment result, does it contain an attach point ? */
        if (rcode==3040) {
          const char *p;

          /* it should... */
          p=GWEN_DB_GetCharValue(dbResult, "param", 0, 0);
          if (!p) {
            DBG_ERROR(AQHBCI_LOGDOMAIN, "Segment result 3040 without attachpoint");
          }
          else {
            GWEN_DB_NODE *args;

            /* store the attach point */
            DBG_DEBUG(AQHBCI_LOGDOMAIN, "Storing attach point");
            args=AH_Job_GetArguments(j);
            GWEN_DB_SetCharValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "attach", p);
            AH_Job_AddFlags(j, AH_JOB_FLAGS_HASATTACHPOINT);
          }
        } /* if code 3040 (means "more data available") */
        dbResult=GWEN_DB_FindNextGroup(dbResult, "result");
      } /* while */
    } /* if segresult */
  } /* if attachable */
}



void _handleSegmentResultForAllJobs(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbSegment)
{
  AH_JOB *j;

  j=AH_JobQueue_GetFirstJob(jq);
  while (j) {
    _handleSegmentResult(jq, j, dbSegment);
    j=AH_Job_List_Next(j);
  } /* while j */
}



void _handleSegmentResult(AH_JOBQUEUE *jq, AH_JOB *j, GWEN_DB_NODE *dbSegment)
{
  if (strcasecmp(GWEN_DB_GroupName(dbSegment), "SegResult")==0) {
    GWEN_DB_NODE *dbResult;

    dbResult=GWEN_DB_FindFirstGroup(dbSegment, "result");
    while (dbResult) {
      int rcode;
      const char *rtext;

      rcode=GWEN_DB_GetIntValue(dbResult, "resultcode", 0, 0);
      rtext=GWEN_DB_GetCharValue(dbResult, "text", 0, "");

      if (rcode>=9000 && rcode<10000) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Segment result: Error (%d: %s)", rcode, rtext);
        if (!(AH_Job_GetFlags(j) & AH_JOB_FLAGS_IGNORE_ERROR)) {
          AH_Job_AddFlags(j, AH_JOB_FLAGS_HASERRORS);
          AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_HASERRORS);
        }
      }
      else if (rcode>=3000 && rcode<4000) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Segment result: Warning (%d: %s)", rcode, rtext);
        if (!(AH_Job_GetFlags(j) & AH_JOB_FLAGS_IGNORE_ERROR)) {
          AH_Job_AddFlags(j, AH_JOB_FLAGS_HASWARNINGS);
          AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_HASWARNINGS);
        }
      }
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Segment result: Ok (%d: %s)", rcode, rtext);
      }
      dbResult=GWEN_DB_FindNextGroup(dbResult, "result");
    } /* while */
  } /* if SegResult */
}



void _addResponseToAllJobs(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbPreparedJobResponse)
{
  AH_JOB *j;

  j=AH_JobQueue_GetFirstJob(jq);
  while (j) {
    AH_JOB_STATUS jobStatus;

    jobStatus=AH_Job_GetStatus(j);
    if (jobStatus==AH_JobStatusSent || jobStatus==AH_JobStatusAnswered)
      AH_Job_AddResponse(j, GWEN_DB_Group_dup(dbPreparedJobResponse));
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Status %d of job doesn't match", jobStatus);
    }
    j=AH_Job_List_Next(j);
  } /* while */
}



void _handleResponseSegments(AH_JOBQUEUE *jq, AH_MSG *msg, GWEN_DB_NODE *db, GWEN_DB_NODE *dbSecurity)
{
  GWEN_DB_NODE *dbCurr;

  DBG_INFO(AQHBCI_LOGDOMAIN,
	   "Handling responses for message %d (received message num is %d)",
	   AH_Msg_GetMsgRef(msg), AH_Msg_GetMsgNum(msg));

  dbCurr=GWEN_DB_GetFirstGroup(db);
  while (dbCurr) {
    GWEN_DB_NODE *dbPreparedJobResponse;
    GWEN_DB_NODE *dbData;
    int refSegNum;
    int segNum;

    refSegNum=GWEN_DB_GetIntValue(dbCurr, "head/ref", 0, 0);
    segNum=GWEN_DB_GetIntValue(dbCurr, "head/seq", 0, 0);
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Checking response \"%s\" (seg num %d, ref seg num %d)",
	     GWEN_DB_GroupName(dbCurr), segNum, refSegNum);

    /* use same name for main response group */
    dbPreparedJobResponse=GWEN_DB_Group_new(GWEN_DB_GroupName(dbCurr));
    /* add security group */
    GWEN_DB_AddGroup(dbPreparedJobResponse, GWEN_DB_Group_dup(dbSecurity));
    /* create data group */
    dbData=GWEN_DB_GetGroup(dbPreparedJobResponse, GWEN_DB_FLAGS_DEFAULT, "data");
    assert(dbData);
    /* store copy of original response there */
    GWEN_DB_AddGroup(dbData, GWEN_DB_Group_dup(dbCurr));

    if (refSegNum) {
      AH_JOB *j;

      /* search for job to which this response belongs */
      j=_findReferencedJob(jq, AH_Msg_GetMsgRef(msg), refSegNum);
      if (j) {
	DBG_INFO(AQHBCI_LOGDOMAIN,
		 "Job \"%s\" (msg %d, segs :%d-%d) claims response \"%s\" (ref msg %d, ref seg %d)",
		 AH_Job_GetName(j),
		 AH_Job_GetMsgNum(j),
		 AH_Job_GetFirstSegment(j),
		 AH_Job_GetLastSegment(j),
		 GWEN_DB_GroupName(dbCurr),
		 AH_Msg_GetMsgRef(msg),
		 refSegNum);

        _possiblyExtractJobAckCode(j, dbCurr);
        _possiblyExtractAttachPoint(j, dbCurr);

        /* check for segment results */
        if (strcasecmp(GWEN_DB_GroupName(dbCurr), "SegResult")==0)
          _handleSegmentResult(jq, j, dbCurr);

        DBG_INFO(AQHBCI_LOGDOMAIN, "Adding response \"%s\" to job \"%s\"", GWEN_DB_GroupName(dbCurr), AH_Job_GetName(j));
        AH_Job_AddResponse(j, GWEN_DB_Group_dup(dbPreparedJobResponse));
        AH_Job_SetStatus(j, AH_JobStatusAnswered);
      } /* if matching job found */
      else {
        DBG_WARN(AQHBCI_LOGDOMAIN, "No job found, adding response \"%s\" to all jobs", GWEN_DB_GroupName(dbCurr));

        /* add response to all jobs (as queue response) and to queue */
        if (strcasecmp(GWEN_DB_GroupName(dbCurr), "SegResult")==0) {
          _handleSegmentResultForAllJobs(jq, dbCurr);
          _addResponseToAllJobs(jq, dbPreparedJobResponse);
        }
      }
    } /* if refSegNum */
    else {
      /* no reference segment number, add response to all jobs */
      DBG_DEBUG(AQHBCI_LOGDOMAIN,
                "No segment reference number, "
                "adding response \"%s\" to all jobs",
                GWEN_DB_GroupName(dbCurr));

      /* add response to all jobs (as queue response) and to queue */
      if (strcasecmp(GWEN_DB_GroupName(dbCurr), "SegResult")==0)
        _handleSegmentResultForAllJobs(jq, dbCurr);

      else if (strcasecmp(GWEN_DB_GroupName(dbCurr), "MsgResult")==0)
        _addResponseToAllJobs(jq, dbPreparedJobResponse);
    }
    GWEN_DB_Group_free(dbPreparedJobResponse);

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */
}




