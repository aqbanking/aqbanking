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

#include "jobqueue_addjob.h"

#include "aqhbci/banking/user_l.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


AH_JOBQUEUE_ADDRESULT _checkJobTypes(AH_JOBQUEUE *jq, AH_JOB *jobToAdd);
AH_JOBQUEUE_ADDRESULT _checkJobFlags(AH_JOBQUEUE *jq, AH_JOB *jobToAdd);
AH_JOBQUEUE_ADDRESULT _checkSigners(AH_JOBQUEUE *jq, AH_JOB *jobToAdd);

int _countJobTypes(const AH_JOBQUEUE *jq, const AH_JOB *jobToAdd);
int _countJobsOtherThanTan(const AH_JOBQUEUE *jq);
int _countJobsOfType(const AH_JOBQUEUE *jq, const char *jobTypeName);
int _list2HasAllEntriesOfList1(const GWEN_STRINGLIST *stringList1, const GWEN_STRINGLIST *stringList2);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AH_JOBQUEUE_ADDRESULT AH_JobQueue_AddJob(AH_JOBQUEUE *jq, AH_JOB *j)
{

  assert(jq);

  /* job owner must equal queue owner */
  if (AH_Job_GetUser(j)!=AH_JobQueue_GetUser(jq)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Owner of the job doesn't match");
    return AH_JobQueueAddResultJobLimit;
  }

  if (AH_JobQueue_GetCount(jq)<1) {
    const GWEN_STRINGLIST *sl;

    sl=AH_Job_GetSigners(j);
    if (sl) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Copying %d signers from job to queue", GWEN_StringList_Count(sl));
      AH_JobQueue_SetSigners(jq, GWEN_StringList_dup(sl));
    }
  }
  else {
    if (strcasecmp(AH_Job_GetName(j), "JobTan")!=0) {
      AH_JOBQUEUE_ADDRESULT jobQueueResult;

      jobQueueResult=_checkJobFlags(jq, j);
      if (jobQueueResult!=AH_JobQueueAddResultOk) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", jobQueueResult);
        return jobQueueResult;
      }

      jobQueueResult=_checkJobTypes(jq, j);
      if (jobQueueResult!=AH_JobQueueAddResultOk) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", jobQueueResult);
        return jobQueueResult;
      }

      jobQueueResult=_checkSigners(jq, j);
      if (jobQueueResult!=AH_JobQueueAddResultOk) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", jobQueueResult);
        return jobQueueResult;
      }

      if (AH_JobQueue_GetSecClass(jq)==0)
        AH_JobQueue_SetSecClass(jq, AH_Job_GetSecurityClass(j));
      else {
        if (AH_JobQueue_GetSecClass(jq)!=AH_Job_GetSecurityClass(j)) {
          DBG_INFO(AQHBCI_LOGDOMAIN,
                   "Job's security class doesn't match that of the queue (%d != %d)",
                   AH_JobQueue_GetSecClass(jq), AH_Job_GetSecurityClass(j));
          return AH_JobQueueAddResultJobLimit;
        }
      }
    } /* if not JobTan */
  }

  /* update maximum security profile */
  if (AH_Job_GetSecurityProfile(j)>AH_JobQueue_GetSecProfile(jq))
    AH_JobQueue_SetSecProfile(jq, AH_Job_GetSecurityProfile(j));

  if (strcasecmp(AH_Job_GetName(j), "JobTan")!=0) {
    /* adjust queue flags according to current job */
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_CRYPT)
      AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_CRYPT);
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_SIGN)
      AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_SIGN);
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_NEEDTAN)
      AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_NEEDTAN);
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_NOSYSID)
      AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_NOSYSID);
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_SIGNSEQONE)
      AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_SIGNSEQONE);
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_NOITAN)
      AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_NOITAN);
  }

  /* actually add job to queue */
  AH_Job_List_Add(j, AH_JobQueue_GetJobList(jq));
  AH_Job_SetStatus(j, AH_JobStatusEnqueued);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Job added to the queue (flags: %08x)", AH_JobQueue_GetFlags(jq));
  return AH_JobQueueAddResultOk;
}



AH_JOBQUEUE_ADDRESULT _checkJobTypes(AH_JOBQUEUE *jq, AH_JOB *jobToAdd)
{
  AB_USER *user;
  AH_BPD *bpd;
  int maxNumOfThisJobPerMsg;
  int maxJobTypes;
  int jobTypeCount;
  int thisJobTypeCount;

  /* sample some variables */
  user=AH_JobQueue_GetUser(jq);
  bpd=AH_User_GetBpd(user);
  maxNumOfThisJobPerMsg=AH_Job_GetJobsPerMsg(jobToAdd);
  maxJobTypes=AH_Bpd_GetJobTypesPerMsg(bpd);

  /* count jobs */
  jobTypeCount=_countJobTypes(jq, jobToAdd);
  thisJobTypeCount=_countJobsOfType(jq, AH_Job_GetName(jobToAdd))+1;

  /* checks */
  if (maxNumOfThisJobPerMsg && thisJobTypeCount>maxNumOfThisJobPerMsg) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Too many jobs of this kind (limit is %d)", maxNumOfThisJobPerMsg);
    return AH_JobQueueAddResultJobLimit;
  }

  if (maxJobTypes && jobTypeCount>maxJobTypes) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Too many different job types (limit is %d)", maxJobTypes);
    return AH_JobQueueAddResultJobLimit;
  }

  return AH_JobQueueAddResultOk;
}



int _countJobTypes(const AH_JOBQUEUE *jq, const AH_JOB *jobToAdd)
{
  AH_JOB *j;
  GWEN_STRINGLIST *jobTypeList;
  int jobTypeCount;

  jobTypeList=GWEN_StringList_new();
  j=AH_JobQueue_GetFirstJob(jq);
  while (j) {
    const char *jobType;

    jobType=AH_Job_GetName(j);
    if (strcasecmp(jobType, "JobTan")!=0)
      GWEN_StringList_AppendString(jobTypeList, jobType, 0, 1);
    j=AH_Job_List_Next(j);
  } /* while */

  if (strcasecmp(AH_Job_GetName(jobToAdd), "JobTan")!=0)
    GWEN_StringList_AppendString(jobTypeList, AH_Job_GetName(jobToAdd), 0, 1);

  jobTypeCount=GWEN_StringList_Count(jobTypeList);
  GWEN_StringList_free(jobTypeList);
  return jobTypeCount;
}



int _countJobsOfType(const AH_JOBQUEUE *jq, const char *jobTypeName)
{
  AH_JOB *j;
  int jobCount=0;

  j=AH_JobQueue_GetFirstJob(jq);
  while (j) {
    if (strcasecmp(AH_Job_GetName(j), jobTypeName)==0)
      jobCount++;
    j=AH_Job_List_Next(j);
  } /* while */

  return jobCount;
}



int _countJobsOtherThanTan(const AH_JOBQUEUE *jq)
{
  AH_JOB *j;
  int jobCount=0;

  j=AH_JobQueue_GetFirstJob(jq);
  while (j) {
    if (strcasecmp(AH_Job_GetName(j), "JobTan")!=0)
      jobCount++;
    j=AH_Job_List_Next(j);
  } /* while */

  return jobCount;
}



AH_JOBQUEUE_ADDRESULT _checkJobFlags(AH_JOBQUEUE *jq, AH_JOB *jobToAdd)
{
  if (strcasecmp(AH_Job_GetName(jobToAdd), "JobTan")!=0 && strcasecmp(AH_Job_GetName(jobToAdd), "JobAcknowledge")!=0) {
    uint32_t flagsInJobToAdd;
    uint32_t flagsInFirstJob;
    AH_JOB *firstJob;

    firstJob=AH_JobQueue_GetFirstJob(jq);

    flagsInJobToAdd=AH_Job_GetFlags(jobToAdd);
    flagsInFirstJob=AH_Job_GetFlags(firstJob);

    if ((flagsInJobToAdd & AH_JOB_FLAGS_SINGLE) || (flagsInJobToAdd & AH_JOB_FLAGS_DLGJOB)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "New jobs wants to be alone but queue is not empty");
      return AH_JobQueueAddResultQueueFull;
    }

    if ((flagsInFirstJob & AH_JOB_FLAGS_SINGLE) || (flagsInFirstJob & AH_JOB_FLAGS_DLGJOB)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Queue already contains a job which wants to be left alone");
      return AH_JobQueueAddResultQueueFull;
    }

    if ((flagsInJobToAdd & (AH_JOB_FLAGS_CRYPT |
                            AH_JOB_FLAGS_NEEDTAN |
                            AH_JOB_FLAGS_NOSYSID |
                            AH_JOB_FLAGS_NOITAN))
        !=
        (flagsInFirstJob & (AH_JOB_FLAGS_CRYPT |
                            AH_JOB_FLAGS_NEEDTAN |
                            AH_JOB_FLAGS_NOSYSID |
                            AH_JOB_FLAGS_NOITAN))) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Encryption/TAN/SysId flags for queue and this job differ");
      return AH_JobQueueAddResultJobLimit;
    }
  }

  return AH_JobQueueAddResultOk;
}



AH_JOBQUEUE_ADDRESULT _checkSigners(AH_JOBQUEUE *jq, AH_JOB *jobToAdd)
{

  if (strcasecmp(AH_Job_GetName(jobToAdd), "JobTan")!=0) {
    if (!_list2HasAllEntriesOfList1(AH_Job_GetSigners(jobToAdd), AH_JobQueue_GetSigners(jq)) ||
        !_list2HasAllEntriesOfList1(AH_JobQueue_GetSigners(jq), AH_Job_GetSigners(jobToAdd))) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Signers of the job differ from those of the queue");
      return AH_JobQueueAddResultJobLimit;
    }
  }

  return AH_JobQueueAddResultOk;
}



int _list2HasAllEntriesOfList1(const GWEN_STRINGLIST *stringList1, const GWEN_STRINGLIST *stringList2)
{
  GWEN_STRINGLISTENTRY *se;

  se=GWEN_StringList_FirstEntry(stringList1);
  while (se) {
    if (!GWEN_StringList_HasString(stringList2, GWEN_StringListEntry_Data(se))) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Entry from first list is missing in 2nd list");
      return 0;
    }
    se=GWEN_StringListEntry_Next(se);
  } /* while se */

  return 1;
}


