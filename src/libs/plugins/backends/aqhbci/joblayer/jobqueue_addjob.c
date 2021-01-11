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



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AH_JOBQUEUE_ADDRESULT AH_JobQueue_AddJob(AH_JOBQUEUE *jq, AH_JOB *j)
{
  AB_USER *user;
  int jobsPerMsg;
  int maxJobTypes;
  int jobCount;
  int jobTypeCount;
  int thisJobTypeCount;
  int hasSingle;
  int crypt;
  int needTAN;
  int noSysId;
  int signSeqOne;
  int noItan;
  GWEN_STRINGLIST *jobTypes;
  AH_JOB *cj;
  AH_BPD *bpd;

  assert(jq);

  user=AH_JobQueue_GetUser(jq);

  /* job owner must equal queue owner */
  if (AH_Job_GetUser(j)!=user) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Owner of the job doesn't match");
    return AH_JobQueueAddResultJobLimit;
  }

  /* sample some variables */
  bpd=AH_User_GetBpd(user);
  jobsPerMsg=AH_Job_GetJobsPerMsg(j);
  maxJobTypes=AH_Bpd_GetJobTypesPerMsg(bpd);

  jobCount=0;
  jobTypeCount=0;
  thisJobTypeCount=0;
  hasSingle=0;
  crypt=0;
  needTAN=0;
  noSysId=0;
  signSeqOne=0;
  noItan=0;
  jobTypes=GWEN_StringList_new();
  cj=AH_JobQueue_GetFirstJob(jq);
  while (cj) {
    jobCount++;
    GWEN_StringList_AppendString(jobTypes, AH_Job_GetName(cj), 0, 1);
    if (strcasecmp(AH_Job_GetName(cj), AH_Job_GetName(j))==0)
      thisJobTypeCount++;
    hasSingle|=((AH_Job_GetFlags(cj) & AH_JOB_FLAGS_SINGLE) ||
                (AH_Job_GetFlags(cj) & AH_JOB_FLAGS_DLGJOB));
    crypt|=(AH_Job_GetFlags(cj) & AH_JOB_FLAGS_CRYPT);
    needTAN|=(AH_Job_GetFlags(cj) & AH_JOB_FLAGS_NEEDTAN);
    noSysId|=(AH_Job_GetFlags(cj) & AH_JOB_FLAGS_NOSYSID);
    signSeqOne|=(AH_Job_GetFlags(cj) & AH_JOB_FLAGS_SIGNSEQONE);
    noItan|=(AH_Job_GetFlags(cj) & AH_JOB_FLAGS_NOITAN);
    cj=AH_Job_List_Next(cj);
  } /* while */
  /* Account for new job when checking limits for thisJobTypeCount and
   * jobTypeCount */
  thisJobTypeCount++;
  GWEN_StringList_AppendString(jobTypes, AH_Job_GetName(j), 0, 1);
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

    /* check security class */
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

    /* check for signers */
    if (!jobCount && !GWEN_StringList_Count(AH_JobQueue_GetSigners(jq))) {
      const GWEN_STRINGLIST *sl;

      /* no jobs in queue and no signers,
       * so simply copy the signers of this job */
      sl=AH_Job_GetSigners(j);
      if (sl) {
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Copying %d signers from job to queue", GWEN_StringList_Count(sl));
        AH_JobQueue_SetSigners(jq, GWEN_StringList_dup(sl));
      }
    }
    else {
      const GWEN_STRINGLIST *sl;
      GWEN_STRINGLISTENTRY *se;

      sl=AH_Job_GetSigners(j);
      if (GWEN_StringList_Count(sl)!=GWEN_StringList_Count(AH_JobQueue_GetSigners(jq))) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Number of signers of the job differs from that of the queue");
        return AH_JobQueueAddResultJobLimit;
      }
      se=GWEN_StringList_FirstEntry(sl);
      while (se) {
        if (!GWEN_StringList_HasString(AH_JobQueue_GetSigners(jq), GWEN_StringListEntry_Data(se))) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Signers of the job differ from those of the queue");
          return AH_JobQueueAddResultJobLimit;
        }
        se=GWEN_StringListEntry_Next(se);
      } /* while se */
    }

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

  /* update maximum security profile */
  if (AH_Job_GetSecurityProfile(j)>AH_JobQueue_GetSecProfile(jq))
    AH_JobQueue_SetSecProfile(jq, AH_Job_GetSecurityProfile(j));

  /* actually add job to queue */
  AH_Job_List_Add(j, AH_JobQueue_GetJobList(jq));
  AH_Job_SetStatus(j, AH_JobStatusEnqueued);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Job added to the queue (flags: %08x)", AH_JobQueue_GetFlags(jq));
  return AH_JobQueueAddResultOk;
}




