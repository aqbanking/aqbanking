/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "jobqueue_dispatch.h"
#include "jobqueue_bpd.h"
#include "jobqueue_account.h"

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

static GWEN_DB_NODE *_sampleResponseSegments(AH_JOBQUEUE *jq, AH_MSG *msg, GWEN_DB_NODE *db, GWEN_DB_NODE *dbSecurity);
static void _dispatchResponsesToJobQueue(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResponses);

static void _logResultSegment(int rcode, const char *p, int isMsgResult, GWEN_LOGGER_LEVEL level, uint32_t guiid);
static void _scanSingleResultSegment(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResult, int isMsgResult, uint32_t guiid);
static void _checkErrorResultSegment(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResult, int isMsgResult, uint32_t guiid);
static void _checkWarningResultSegment(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResult, int isMsgResult, uint32_t guiid);
static void _handleResult_3920(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResult);
static void _handleResult_3072(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResult, uint32_t guiid);


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
  GWEN_DB_NODE *dbCurr;

  dbCurr=GWEN_DB_GetFirstGroup(db);
  while (dbCurr) {
    if (strcasecmp(GWEN_DB_GroupName(dbCurr), "SegResult")==0 ||
        strcasecmp(GWEN_DB_GroupName(dbCurr), "MsgResult")==0) {
      int isMsgResult;
      GWEN_DB_NODE *dbResult;

      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found a result");
      isMsgResult=(strcasecmp(GWEN_DB_GroupName(dbCurr), "MsgResult")==0);

      dbResult=GWEN_DB_FindFirstGroup(dbCurr, "result");
      while (dbResult) {
	_scanSingleResultSegment(jq, dbResult, isMsgResult, guiid);
        dbResult=GWEN_DB_FindNextGroup(dbResult, "result");
      } /* while results */
    }

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }
}



void _scanSingleResultSegment(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResult, int isMsgResult, uint32_t guiid)
{
  int rcode;

  rcode=GWEN_DB_GetIntValue(dbResult, "resultcode", 0, 0);

  if (rcode>=9000 && rcode<10000)
    _checkErrorResultSegment(jq, dbResult, isMsgResult, guiid);
  else if (rcode>=3000 && rcode<4000)
    _checkWarningResultSegment(jq, dbResult, isMsgResult, guiid);
  else {
    const char *p;

    p=GWEN_DB_GetCharValue(dbResult, "text", 0, "");
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment result: Ok (%d: %s)", rcode, p);
    _logResultSegment(rcode, p, isMsgResult, GWEN_LoggerLevel_Notice, guiid);
  }
}



void _checkErrorResultSegment(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResult, int isMsgResult, uint32_t guiid)
{
  int rcode;
  const char *p;

  rcode=GWEN_DB_GetIntValue(dbResult, "resultcode", 0, 0);
  p=GWEN_DB_GetCharValue(dbResult, "text", 0, "");

  DBG_INFO(AQHBCI_LOGDOMAIN, "Result: Error (%d: %s)", rcode, p);
  _logResultSegment(rcode, p, isMsgResult, GWEN_LoggerLevel_Error, guiid),
  AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_HASERRORS);

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
      AB_USER *user;

      user=AH_JobQueue_GetUser(jq);
      GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Error, I18N("PIN invalid according to server"));
      AH_User_SetPinStatus(user, AH_JobQueue_GetUsedPin(jq), GWEN_Gui_PasswordStatus_Bad);
    }
  }
}



void _checkWarningResultSegment(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResult, int isMsgResult, uint32_t guiid)
{
  int rcode;
  const char *p;

  rcode=GWEN_DB_GetIntValue(dbResult, "resultcode", 0, 0);
  p=GWEN_DB_GetCharValue(dbResult, "text", 0, "");

  DBG_INFO(AQHBCI_LOGDOMAIN, "Result: Warning (%d: %s)", rcode, p);
  _logResultSegment(rcode, p, isMsgResult, GWEN_LoggerLevel_Warning, guiid);
  AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_HASWARNINGS);
  if (rcode==3910)
    AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_RECYCLE_TAN);
  else if (rcode==3920)
    _handleResult_3920(jq, dbResult);
  else if (rcode==3072)
    _handleResult_3072(jq, dbResult, guiid); /* extract new user/customer id */
}



void _handleResult_3920(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResult)
{
  int i;
  AB_USER *user;

  user=AH_JobQueue_GetUser(jq);
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



void _handleResult_3072(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResult, uint32_t guiid)
{
  const char *sUserId;

  sUserId=GWEN_DB_GetCharValue(dbResult, "param", 0, NULL);
  if (sUserId && *sUserId) {
    const char *sCustomerId;
    AB_USER *user;

    sCustomerId=GWEN_DB_GetCharValue(dbResult, "param", 1, NULL);
    if (!(sCustomerId && *sCustomerId))
      sCustomerId=sUserId;
    DBG_WARN(AQHBCI_LOGDOMAIN, "USERID/CUSTOMERID changed by bank");
    GWEN_Gui_ProgressLog(guiid, GWEN_LoggerLevel_Warning, "USERID/CUSTOMERID changed by bank");
    user=AH_JobQueue_GetUser(jq);
    AB_User_SetUserId(user, sUserId);
    AB_User_SetCustomerId(user, sCustomerId);
  }
}



void _logResultSegment(int rcode, const char *p, int isMsgResult, GWEN_LOGGER_LEVEL level, uint32_t guiid)
{
  GWEN_BUFFER *logmsg;

  logmsg=GWEN_Buffer_new(0, 256, 0, 1);
  if (p)
    GWEN_Buffer_AppendArgs(logmsg, "HBCI: %04d - %s (%s)", rcode, p, isMsgResult?"M":"S");
  else
    GWEN_Buffer_AppendArgs(logmsg, "HBCI: %04d - (no text) (%s)", rcode, isMsgResult?"M":"S");
  GWEN_Gui_ProgressLog(guiid, level, GWEN_Buffer_GetStart(logmsg));
  GWEN_Buffer_free(logmsg);
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
      DBG_INFO(AQHBCI_LOGDOMAIN, "Checking whether job \"%s\" has segment %d", jobName, refSegNum);
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



GWEN_DB_NODE *_sampleResponseSegments(AH_JOBQUEUE *jq, AH_MSG *msg, GWEN_DB_NODE *db, GWEN_DB_NODE *dbSecurity)
{
  GWEN_DB_NODE *dbAllResponses;
  GWEN_DB_NODE *dbCurr;
  int responsesAdded=0;

  DBG_INFO(AQHBCI_LOGDOMAIN,
	   "Handling responses for message %d (received message num is %d)",
	   AH_Msg_GetMsgRef(msg), AH_Msg_GetMsgNum(msg));

  dbAllResponses=GWEN_DB_Group_new("responses");

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
    GWEN_DB_SetIntValue(dbPreparedJobResponse, GWEN_DB_FLAGS_DEFAULT, "refSegNum", refSegNum);
    GWEN_DB_SetIntValue(dbPreparedJobResponse, GWEN_DB_FLAGS_DEFAULT, "refMsgNum", AH_Msg_GetMsgRef(msg));

    /* add security group */
    GWEN_DB_AddGroup(dbPreparedJobResponse, GWEN_DB_Group_dup(dbSecurity));
    /* create data group */
    dbData=GWEN_DB_GetGroup(dbPreparedJobResponse, GWEN_DB_FLAGS_DEFAULT, "data");
    assert(dbData);
    /* store copy of original response there */
    GWEN_DB_AddGroup(dbData, GWEN_DB_Group_dup(dbCurr));

    GWEN_DB_AddGroup(dbAllResponses, dbPreparedJobResponse);
    responsesAdded++;

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  if (responsesAdded<1) {
    GWEN_DB_Group_free(dbAllResponses);
    return NULL;
  }

  return dbAllResponses;
}



void _dispatchResponsesToJobQueue(AH_JOBQUEUE *jq, GWEN_DB_NODE *dbResponses)
{
  GWEN_DB_NODE *dbPreparedJobResponse;

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Dispatching response to job queue");
  dbPreparedJobResponse=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbPreparedJobResponse) {
    const char *groupName;
    int refSegNum;
    int refMsgNum;
    GWEN_DB_NODE *dbData;

    if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Dispatching this message:");
      GWEN_DB_Dump(dbPreparedJobResponse, 2);
    }

    refMsgNum=GWEN_DB_GetIntValue(dbPreparedJobResponse, "refMsgNum", 0, 0);
    refSegNum=GWEN_DB_GetIntValue(dbPreparedJobResponse, "refSegNum", 0, 0);
    groupName=GWEN_DB_GroupName(dbPreparedJobResponse);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Checking response \"%s\" (ref seg num %d)", groupName, refSegNum);
    dbData=GWEN_DB_GetGroup(dbPreparedJobResponse, GWEN_DB_FLAGS_DEFAULT, "data");
    assert(dbData);

    if (refSegNum) {
      AH_JOB *j;

      /* search for job to which this response belongs */
      j=_findReferencedJob(jq, refMsgNum, refSegNum);
      if (j) {
        const char *refJobName;

        refJobName=AH_Job_GetName(j);
        DBG_INFO(AQHBCI_LOGDOMAIN,
		 "Job \"%s\" (msg %d, segs :%d-%d) claims response \"%s\" (ref msg %d, ref seg %d)",
                 refJobName,
		 AH_Job_GetMsgNum(j),
		 AH_Job_GetFirstSegment(j),
		 AH_Job_GetLastSegment(j),
		 groupName,
		 refMsgNum,
		 refSegNum);
        if (!(strcasecmp(refJobName, "JobTan")==0 &&
              strcasecmp(groupName, "TanResponse")!=0 &&
              strcasecmp(groupName, "SegResult")!=0)) {
          _possiblyExtractJobAckCode(j, dbData);
          _possiblyExtractAttachPoint(j, dbData);

          /* check for segment results */
          if (strcasecmp(groupName, "SegResult")==0)
            _handleSegmentResult(jq, j, dbData);

          DBG_INFO(AQHBCI_LOGDOMAIN, "Adding response \"%s\" to job \"%s\"", groupName, AH_Job_GetName(j));
          AH_Job_AddResponse(j, GWEN_DB_Group_dup(dbPreparedJobResponse));
          AH_Job_SetStatus(j, AH_JobStatusAnswered);
        }
        else {
          DBG_NOTICE(AQHBCI_LOGDOMAIN,
                     "Not adding response \"%s\" to job \"%s\" (neither TanResponse nor SegResult)",
                     groupName, refJobName);
        }
      } /* if matching job found */
      else {
        DBG_WARN(AQHBCI_LOGDOMAIN, "No job found for response \"%s\"", groupName);
        if (strcasecmp(groupName, "SegResult")==0) {
          DBG_WARN(AQHBCI_LOGDOMAIN, "Adding response \"%s\" to all jobs", groupName);
	  _handleSegmentResultForAllJobs(jq, dbData);
          _addResponseToAllJobs(jq, dbPreparedJobResponse);
        }
      }
    } /* if refSegNum */
    else {
      /* no reference segment number, add response to all jobs */
      DBG_DEBUG(AQHBCI_LOGDOMAIN,
		"No segment reference number, adding response \"%s\" to all jobs",
		groupName);
      if (strcasecmp(groupName, "SegResult")==0)
        _handleSegmentResultForAllJobs(jq, dbData);

      else if (strcasecmp(groupName, "MsgResult")==0)
        _addResponseToAllJobs(jq, dbPreparedJobResponse);
    }

    dbPreparedJobResponse=GWEN_DB_GetNextGroup(dbPreparedJobResponse);
  } /* while */
}



void _handleResponseSegments(AH_JOBQUEUE *jq, AH_MSG *msg, GWEN_DB_NODE *db, GWEN_DB_NODE *dbSecurity)
{
  GWEN_DB_NODE *dbAllResponses;

  dbAllResponses=_sampleResponseSegments(jq, msg, db, dbSecurity);
  if (dbAllResponses) {
    AH_JOBQUEUE *jqRun;
    int queueNum=0;

    /* first extract all interesting data */
    AH_JobQueue_ReadBpd(jq, dbAllResponses);
    if (AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_IGNOREACCOUNTS) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Ignoring possibly received accounts");
    }
    else
      AH_JobQueue_ReadAccounts(jq, dbAllResponses);

    jqRun=jq;
    while(jqRun) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Dispatching responses to queue %d", queueNum);
      /* then dispatch to jobs in this and in reference queue */
      _dispatchResponsesToJobQueue(jqRun, dbAllResponses);
      queueNum++;
      jqRun=AH_JobQueue_GetReferenceQueue(jqRun);
      if (jqRun) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Queue %d has a reference queue", queueNum);
      }
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Queue %d has no reference queue", queueNum);
      }
    }
    GWEN_DB_Group_free(dbAllResponses);
  }
}



