/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "outbox_p.h"

#include "aqhbci/ajobs/accountjob_l.h"

#include "aqhbci/applayer/cbox_prepare.h"
#include "aqhbci/applayer/cbox_queue.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <assert.h>


/*#define EXTREME_DEBUGGING */



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static unsigned int _countTodoJobs(AH_OUTBOX *ob);
static int _sendOutboxWithProbablyLockedUsers(AH_OUTBOX *ob);
static AH_JOB *_findTransferJobInCheckJobList(const AH_JOB_LIST *jl, AB_USER *u, AB_ACCOUNT *a, const char *jobName);
static int _prepare(AH_OUTBOX *ob);
static void _finishCBox(AH_OUTBOX *ob, AH_OUTBOX_CBOX *cbox);
static int _sendAndRecvCustomerBoxes(AH_OUTBOX *ob);
static int _lockUsers(AH_OUTBOX *ob, AB_USER_LIST2 *lockedUsers);
static int _unlockUsers(AH_OUTBOX *ob, AB_USER_LIST2 *lockedUsers, int abandon);
static void _finishRemainingCustomerBoxes(AH_OUTBOX *ob);
static AH_OUTBOX_CBOX *_findCBox(const AH_OUTBOX *ob, const AB_USER *u);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AH_OUTBOX *AH_Outbox_new(AB_PROVIDER *pro)
{
  AH_OUTBOX *ob;

  assert(pro);
  GWEN_NEW_OBJECT(AH_OUTBOX, ob);
  GWEN_INHERIT_INIT(AH_OUTBOX, ob);

  ob->provider=pro;
  ob->userBoxes=AH_OutboxCBox_List_new();
  ob->finishedJobs=AH_Job_List_new();
  ob->usage=1;
  return ob;
}



void AH_Outbox_free(AH_OUTBOX *ob)
{
  if (ob) {
    assert(ob->usage);
    if (--(ob->usage)==0) {
      AH_OutboxCBox_List_free(ob->userBoxes);
      AH_Job_List_free(ob->finishedJobs);
      GWEN_INHERIT_FINI(AH_OUTBOX, ob);
      GWEN_FREE_OBJECT(ob);
    }
  }
}



void AH_Outbox_Attach(AH_OUTBOX *ob)
{
  assert(ob);
  ob->usage++;
}



AB_IMEXPORTER_CONTEXT *AH_Outbox_GetImExContext(const AH_OUTBOX *ob)
{
  assert(ob);
  return ob->context;
}



AH_JOB_LIST *AH_Outbox_GetFinishedJobs(AH_OUTBOX *ob)
{
  assert(ob);
  assert(ob->usage);
  return ob->finishedJobs;
}



int AH_Outbox_Execute(AH_OUTBOX *ob,
                      AB_IMEXPORTER_CONTEXT *ctx,
                      int withProgress, int nounmount, int doLock)
{
  int rv;
  uint32_t pid=0;
  AB_USER_LIST2 *lockedUsers=NULL;

  assert(ob);

  if (withProgress) {
    pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                               GWEN_GUI_PROGRESS_ALLOW_EMBED |
                               GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                               GWEN_GUI_PROGRESS_SHOW_ABORT,
                               I18N("Executing Jobs"),
                               I18N("Now the jobs are sent via their "
                                    "backends to the credit institutes."),
                               _countTodoJobs(ob),
                               0);
  }

  ob->context=ctx;

  if (doLock) {
    lockedUsers=AB_User_List2_new();
    rv=_lockUsers(ob, lockedUsers);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AB_User_List2_free(lockedUsers);
    }
  }
  else
    rv=0;

  if (rv==0) {
    rv=_sendOutboxWithProbablyLockedUsers(ob);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    }
    if (doLock) {
      int rv2;

      rv2=_unlockUsers(ob, lockedUsers, 0);
      if (rv2<0) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv2);
      }
      AB_User_List2_free(lockedUsers);
      if (rv==0 && rv2!=0)
        rv=rv2;
    }
  }

  if (!nounmount)
    AB_Banking_ClearCryptTokenList(AB_Provider_GetBanking(ob->provider));

  if (withProgress) {
    GWEN_Gui_ProgressEnd(pid);
  }

  ob->context=NULL;
  return rv;
}



AH_JOB *AH_Outbox_FindTransferJob(AH_OUTBOX *ob, AB_USER *u, AB_ACCOUNT *a, const char *jobName)
{
  AH_OUTBOX_CBOX *cbox;
  AH_JOB *j;

  assert(ob);
  assert(u);
  assert(a);
  assert(jobName);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Searching for %s job", jobName);
  cbox=AH_OutboxCBox_List_First(ob->userBoxes);
  while (cbox) {
    if (AH_OutboxCBox_GetUser(cbox)==u) {
      AH_JOBQUEUE *jq;

      /* check jobs in lists */
      j=_findTransferJobInCheckJobList(AH_OutboxCBox_GetTodoJobs(cbox), u, a, jobName);
      if (j)
        return j;

      /* check jobs in queues */
      jq=AH_JobQueue_List_First(AH_OutboxCBox_GetTodoQueues(cbox));
      while (jq) {
        const AH_JOB_LIST *jl;

        jl=AH_JobQueue_GetJobList(jq);
        if (jl) {
          j=_findTransferJobInCheckJobList(jl, u, a, jobName);
          if (j)
            return j;
        }
        jq=AH_JobQueue_List_Next(jq);
      } /* while */
    }
    else {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Customer doesn't match");
    }

    cbox=AH_OutboxCBox_List_Next(cbox);
  } /* while */

  DBG_INFO(AQHBCI_LOGDOMAIN, "No matching multi job found");
  return 0;
}



int _sendOutboxWithProbablyLockedUsers(AH_OUTBOX *ob)
{
  unsigned int jobCount;
  int rv;

  assert(ob);
  jobCount=_countTodoJobs(ob);
  if (jobCount==0) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Empty outbox");
    return 0;
  }

  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice, I18N("AqHBCI started"));

  rv=_prepare(ob);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error preparing jobs for sending.");
    return rv;
  }

  rv=_sendAndRecvCustomerBoxes(ob);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error while sending outbox.");
    return rv;
  }

  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice, I18N("AqHBCI finished."));
  return 0;
}



int _lockUsers(AH_OUTBOX *ob, AB_USER_LIST2 *lockedUsers)
{
  AH_OUTBOX_CBOX *cbox;

  assert(ob);

  cbox=AH_OutboxCBox_List_First(ob->userBoxes);
  while (cbox) {
    int rv;
    AB_USER *user;

    user=AH_OutboxCBox_GetUser(cbox);

    DBG_INFO(AQHBCI_LOGDOMAIN, "Locking customer \"%lu\"",
             (unsigned long int) AB_User_GetUniqueId(user));
    GWEN_Gui_ProgressLog2(0,
			  GWEN_LoggerLevel_Info,
			  "Locking customer \"%lu\"",
			  (unsigned long int) AB_User_GetUniqueId(user));
    rv=AB_Provider_BeginExclUseUser(ob->provider, user);
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not lock customer [%lu] (%d)", (unsigned long int) AB_User_GetUniqueId(user), rv);
      GWEN_Gui_ProgressLog2(0,
                            GWEN_LoggerLevel_Error,
                            I18N("Could not lock user %lu (%d)"),
                            (unsigned long int) AB_User_GetUniqueId(user),
                            rv);
      _unlockUsers(ob, lockedUsers, 1); /* abandon */
      return rv;
    }
    AB_User_List2_PushBack(lockedUsers, user);

    cbox=AH_OutboxCBox_List_Next(cbox);
  } /* while */

  return 0;
}



int _unlockUsers(AH_OUTBOX *ob, AB_USER_LIST2 *lockedUsers, int abandon)
{
  int errors=0;
  AB_USER_LIST2_ITERATOR *it;

  assert(ob);

  it=AB_User_List2_First(lockedUsers);
  if (it) {
    AB_USER *u;

    u=AB_User_List2Iterator_Data(it);
    while (u) {
      int rv;

      DBG_INFO(AQHBCI_LOGDOMAIN, "Unlocking customer \"%lu\"",
               (unsigned long int) AB_User_GetUniqueId(u));
      GWEN_Gui_ProgressLog2(0,
                            GWEN_LoggerLevel_Info,
                            "Unlocking customer \"%lu\"",
                            (unsigned long int) AB_User_GetUniqueId(u));
      rv=AB_Provider_EndExclUseUser(ob->provider, u, abandon);
      if (rv<0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Could not unlock user %lu (%d)",
                  (unsigned long int) AB_User_GetUniqueId(u),
                  rv);
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Could not unlock user %lu (%d)"),
                              (unsigned long int) AB_User_GetUniqueId(u),
                              rv);
        errors++;
      }
      u=AB_User_List2Iterator_Next(it);
    }
    AB_User_List2Iterator_free(it);
  }

  if (errors)
    return GWEN_ERROR_GENERIC;

  return 0;
}



AH_OUTBOX_CBOX *_findCBox(const AH_OUTBOX *ob, const AB_USER *u)
{
  AH_OUTBOX_CBOX *cbox;

  assert(ob);
  assert(u);
  cbox=AH_OutboxCBox_List_First(ob->userBoxes);
  while (cbox) {
    if (AH_OutboxCBox_GetUser(cbox)==u) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "CBox for customer \"%s\" found", AB_User_GetCustomerId(u));
      return cbox;
    }
    cbox=AH_OutboxCBox_List_Next(cbox);
  } /* while */
  DBG_INFO(AQHBCI_LOGDOMAIN, "CBox for customer \"%s\" not found", AB_User_GetCustomerId(u));
  return 0;
}





void AH_Outbox_AddJob(AH_OUTBOX *ob, AH_JOB *j)
{
  AB_USER *u;
  AH_OUTBOX_CBOX *cbox;

  assert(ob);
  assert(j);

  u=AH_Job_GetUser(j);
  assert(u);

  cbox=_findCBox(ob, u);
  if (!cbox) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating CBox for customer \"%s\"", AB_User_GetCustomerId(u));
    cbox=AH_OutboxCBox_new(ob->provider, u, ob);
    AH_OutboxCBox_List_Add(cbox, ob->userBoxes);
  }
  /* attach to job so that it will never be destroyed from me */
  AH_Job_Attach(j);
  AH_OutboxCBox_AddTodoJob(cbox, j);
}



int _prepare(AH_OUTBOX *ob)
{
  AH_OUTBOX_CBOX *cbox;
  unsigned int errors;

  assert(ob);

  errors=0;
  cbox=AH_OutboxCBox_List_First(ob->userBoxes);
  while (cbox) {
    AB_USER *u;

    u=AH_OutboxCBox_GetUser(cbox);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing queues for customer \"%s\"", AB_User_GetCustomerId(u));
    if (AH_OutboxCBox_Prepare(cbox)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error preparing cbox");
      errors++;
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing queues for customer \"%s\": done", AB_User_GetCustomerId(u));
    }
    cbox=AH_OutboxCBox_List_Next(cbox);
  } /* while */

  if (errors) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "%d errors occurred", errors);
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



void _finishCBox(AH_OUTBOX *ob, AH_OUTBOX_CBOX *cbox)
{
  AH_JOB_LIST *jl;
  AH_JOB *j;

  assert(ob);
  assert(cbox);

  AH_OutboxCBox_Finish(cbox);
  jl=AH_OutboxCBox_TakeFinishedJobs(cbox);
  assert(jl);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Finishing customer outbox");
  while ((j=AH_Job_List_First(jl))) {
    int rv;
    AH_JOB_STATUS st;

    AH_Job_List_Del(j);
    st=AH_Job_GetStatus(j);
    if (st==AH_JobStatusAnswered) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Letting job \"%s\" process", AH_Job_GetName(j));
      rv=AH_Job_Process(j, ob->context);
      if (rv) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Error in job \"%s\": %d", AH_Job_GetName(j), rv);
        AH_Job_SetStatus(j, AH_JobStatusError);

        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Error,
                              I18N("Error processing job %s"),
                              AH_Job_GetName(j));
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Skipping job \"%s\" because of status \"%s\" (%d)",
               AH_Job_GetName(j), AH_Job_StatusName(st), st);
    }
    AH_Job_List_Add(j, ob->finishedJobs);
  } /* while */
  AH_Job_List_free(jl);
}



void _finishRemainingCustomerBoxes(AH_OUTBOX *ob)
{
  AH_OUTBOX_CBOX *cbox;

  assert(ob);
  while ((cbox=AH_OutboxCBox_List_First(ob->userBoxes))) {
    _finishCBox(ob, cbox);
    AH_OutboxCBox_List_Del(cbox);
    AH_OutboxCBox_free(cbox);
  } /* while */
}



int _sendAndRecvCustomerBoxes(AH_OUTBOX *ob)
{
  AH_OUTBOX_CBOX *cbox;
  int rv;
  int errors;

  errors=0;
  while ((cbox=AH_OutboxCBox_List_First(ob->userBoxes))) {
    AB_USER *u;

    u=AH_OutboxCBox_GetUser(cbox);
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Sending messages for customer \"%s\"",
             AB_User_GetCustomerId(u));

    rv=AH_OutboxCBox_SendAndRecvBox(cbox);
    _finishCBox(ob, cbox);
    AH_OutboxCBox_List_Del(cbox);
    AH_OutboxCBox_free(cbox);
    if (rv)
      errors++;
    if (rv==GWEN_ERROR_USER_ABORTED) {
      _finishRemainingCustomerBoxes(ob);
      return rv;
    }
  } /* while */

  return 0;
}



unsigned int _countTodoJobs(AH_OUTBOX *ob)
{
  unsigned int cnt;
  AH_OUTBOX_CBOX *cbox;

  assert(ob);
  cnt=0;
  cbox=AH_OutboxCBox_List_First(ob->userBoxes);
  while (cbox) {
    AH_JOB_LIST *todoJobs;
    AH_JOBQUEUE *jq;

    todoJobs=AH_OutboxCBox_GetTodoJobs(cbox);
    cnt+=AH_Job_List_GetCount(todoJobs);
    jq=AH_JobQueue_List_First(todoJobs);
    while (jq) {
      if (!(AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_OUTBOX)) {
        const AH_JOB_LIST *jl;

        jl=AH_JobQueue_GetJobList(jq);
        if (jl) {
          AH_JOB *j;

          j=AH_Job_List_First(jl);
          while (j) {
            if (!(AH_Job_GetFlags(j) & AH_JOB_FLAGS_OUTBOX))
              cnt++;

            j=AH_Job_List_Next(j);
          } /* while */
        }
      }
      jq=AH_JobQueue_List_Next(jq);
    } /* while */
    cbox=AH_OutboxCBox_List_Next(cbox);
  } /* while */

  return cnt;
}



AH_JOB *_findTransferJobInCheckJobList(const AH_JOB_LIST *jl, AB_USER *u, AB_ACCOUNT *a, const char *jobName)
{
  AH_JOB *j;

  assert(jl);
  j=AH_Job_List_First(jl);
  while (j) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Checking job \"%s\"", AH_Job_GetName(j));
    if (strcasecmp(AH_Job_GetName(j), jobName)==0 &&
        AH_AccountJob_GetAccount(j)==a) {
      if (AH_Job_GetTransferCount(j)<AH_Job_GetMaxTransfers(j))
        break;
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Job's already full");
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job doesn't match");
    }

    j=AH_Job_List_Next(j);
  } /* while */

  return j;
}



