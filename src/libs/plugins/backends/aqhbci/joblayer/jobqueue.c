/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobqueue_p.h"

#include "aqhbci/aqhbci_l.h"
#include "aqhbci/joblayer/job_l.h"
#include "aqhbci/banking/user_l.h"
#include "aqhbci/msglayer/message_l.h"
#include "aqhbci/msglayer/hbci_l.h"
#include "aqhbci/msglayer/dialog_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>

#include <assert.h>



GWEN_LIST_FUNCTIONS(AH_JOBQUEUE, AH_JobQueue);



AH_JOBQUEUE *AH_JobQueue_new(AB_USER *u)
{
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



void AH_JobQueue_free(AH_JOBQUEUE *jq)
{
  if (jq) {
    assert(jq->usage);
    if (--(jq->usage)==0) {
      GWEN_StringList_free(jq->signers);
      AH_Job_List_free(jq->jobs);
      free(jq->usedTan);
      free(jq->usedPin);

      if (jq->referenceQueue)
        AH_JobQueue_free(jq->referenceQueue);

      GWEN_LIST_FINI(AH_JOBQUEUE, jq);
      GWEN_FREE_OBJECT(jq);
    }
  }
}



void AH_JobQueue_Attach(AH_JOBQUEUE *jq)
{
  assert(jq);
  jq->usage++;
}



uint32_t AH_JobQueue_GetFlags(const AH_JOBQUEUE *jq)
{
  assert(jq);
  assert(jq->usage);
  return jq->flags;
}



void AH_JobQueue_SetFlags(AH_JOBQUEUE *jq, uint32_t f)
{
  assert(jq);
  assert(jq->usage);
  jq->flags=f;
}



void AH_JobQueue_AddFlags(AH_JOBQUEUE *jq, uint32_t f)
{
  assert(jq);
  assert(jq->usage);
  jq->flags|=f;
}



void AH_JobQueue_SubFlags(AH_JOBQUEUE *jq, uint32_t f)
{
  assert(jq);
  assert(jq->usage);
  jq->flags&=~f;
}



AB_USER *AH_JobQueue_GetUser(const AH_JOBQUEUE *jq)
{
  assert(jq);
  assert(jq->usage);
  return jq->user;
}



int AH_JobQueue_GetSecProfile(const AH_JOBQUEUE *jq)
{
  assert(jq);
  assert(jq->usage);
  return jq->secProfile;
}



void AH_JobQueue_SetSecProfile(AH_JOBQUEUE *jq, int i)
{
  assert(jq);
  assert(jq->usage);
  jq->secProfile=i;
}



int AH_JobQueue_GetSecClass(const AH_JOBQUEUE *jq)
{
  assert(jq);
  assert(jq->usage);
  return jq->secClass;
}



void AH_JobQueue_SetSecClass(AH_JOBQUEUE *jq, int i)
{
  assert(jq);
  assert(jq->usage);
  jq->secClass=i;
}



GWEN_STRINGLIST *AH_JobQueue_GetSigners(const AH_JOBQUEUE *jq)
{
  assert(jq);
  assert(jq->usage);
  return jq->signers;
}



void AH_JobQueue_SetSigners(AH_JOBQUEUE *jq, GWEN_STRINGLIST *signers)
{
  assert(jq);
  assert(jq->usage);
  if (jq->signers)
    GWEN_StringList_free(jq->signers);
  jq->signers=signers;
}





AH_JOBQUEUE *AH_JobQueue_fromQueue(AH_JOBQUEUE *oldq)
{
  AH_JOBQUEUE *jq;

  assert(oldq);

  jq=AH_JobQueue_new(oldq->user);
  jq->signers=GWEN_StringList_dup(oldq->signers);
  jq->secProfile=oldq->secProfile;
  jq->secClass=oldq->secClass;
  if (oldq->usedTan)
    jq->usedTan=strdup(oldq->usedTan);
  if (oldq->usedPin)
    jq->usedPin=strdup(oldq->usedPin);

  return jq;
}



void AH_JobQueue_SetUsedTan(AH_JOBQUEUE *jq, const char *s)
{
  assert(jq);
  assert(jq->usage);
  free(jq->usedTan);
  if (s)
    jq->usedTan=strdup(s);
  else
    jq->usedTan=0;
}



void AH_JobQueue_SetUsedPin(AH_JOBQUEUE *jq, const char *s)
{
  assert(jq);
  assert(jq->usage);
  free(jq->usedPin);
  if (s)
    jq->usedPin=strdup(s);
  else
    jq->usedPin=0;
}



AH_JOB_LIST *AH_JobQueue_GetJobList(const AH_JOBQUEUE *jq)
{
  assert(jq);
  assert(jq->usage);
  return jq->jobs;
}



AH_JOB_LIST *AH_JobQueue_TakeJobList(AH_JOBQUEUE *jq)
{
  AH_JOB_LIST *jl;

  assert(jq);
  assert(jq->usage);
  jl=jq->jobs;
  jq->jobs=AH_Job_List_new();
  return jl;
}



AH_JOB *AH_JobQueue_GetFirstJob(const AH_JOBQUEUE *jq)
{
  assert(jq);
  assert(jq->usage);
  if (jq->jobs)
    return AH_Job_List_First(jq->jobs);

  return NULL;
}



void AH_JobQueue_SetJobStatusOnMatch(AH_JOBQUEUE *jq,
                                     AH_JOB_STATUS matchSt,
                                     AH_JOB_STATUS newSt)
{
  AH_JOB *j;

  assert(jq);
  assert(jq->usage);

  j=AH_Job_List_First(jq->jobs);
  while (j) {
    if (matchSt==AH_JobStatusAll ||
        AH_Job_GetStatus(j)==matchSt)
      AH_Job_SetStatus(j, newSt);
    j=AH_Job_List_Next(j);
  } /* while */
}



void AH_JobQueue_Dump(AH_JOBQUEUE *jq, FILE *f, unsigned int insert)
{
  uint32_t k;
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
  if (jq->flags & AH_JOBQUEUE_FLAGS_ISDIALOG)
    fprintf(f, "ISDIALOG ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_NEEDTAN)
    fprintf(f, "NEEDTAN ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_NOSYSID)
    fprintf(f, "NOSYSID ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_NOITAN)
    fprintf(f, "NOITAN ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_SIGNSEQONE)
    fprintf(f, "SIGNSEQONE ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_OUTBOX)
    fprintf(f, "OUTBOX ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_HASWARNINGS)
    fprintf(f, "HASWARNINGS ");
  if (jq->flags & AH_JOBQUEUE_FLAGS_HASERRORS)
    fprintf(f, "HASERRORS ");
  fprintf(f, ")\n");

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Signers:\n");

  se=GWEN_StringList_FirstEntry(jq->signers);
  while (se) {
    for (k=0; k<insert; k++)
      fprintf(f, " ");
    fprintf(f, "  \"%s\"\n", GWEN_StringListEntry_Data(se));
    se=GWEN_StringListEntry_Next(se);
  } /* while se */

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Jobs:\n");
  j=AH_Job_List_First(jq->jobs);
  while (j) {
    AH_Job_Dump(j, f, insert+2);
    j=AH_Job_List_Next(j);
  } /* while j */
}



void AH_JobQueue_DumpJobList(const AH_JOBQUEUE *jq, FILE *f, unsigned int insert)
{
  uint32_t k;
  AH_JOB *j;

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "JobQueue:\n");

  j=AH_Job_List_First(jq->jobs);
  while (j) {
    AH_Job_DumpShort(j, f, insert+2);
    j=AH_Job_List_Next(j);
  } /* while j */
}


unsigned int AH_JobQueue_GetCount(const AH_JOBQUEUE *jq)
{
  assert(jq);
  assert(jq->usage);
  if (jq->jobs)
    return AH_Job_List_GetCount(jq->jobs);
  return 0;
}



const char *AH_JobQueue_GetUsedTan(const AH_JOBQUEUE *jq)
{
  assert(jq);
  assert(jq->usage);

  return jq->usedTan;
}



const char *AH_JobQueue_GetUsedPin(const AH_JOBQUEUE *jq)
{
  assert(jq);
  assert(jq->usage);

  return jq->usedPin;
}



AH_JOBQUEUE *AH_JobQueue_GetReferenceQueue(const AH_JOBQUEUE *jq)
{
  assert(jq);
  assert(jq->usage);

  return jq->referenceQueue;
}



void AH_JobQueue_SetReferenceQueue(AH_JOBQUEUE *jq, AH_JOBQUEUE *refq)
{
  assert(jq);
  assert(jq->usage);

  if (refq)
    AH_JobQueue_Attach(refq);
  if (jq->referenceQueue)
    AH_JobQueue_free(jq->referenceQueue);
  jq->referenceQueue=refq;
}





