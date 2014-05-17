/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "outbox_p.h"
#include "aqhbci_l.h"
#include "job_l.h"
#include "accountjob_l.h"
#include "jobqueue_l.h"
#include "hbci_l.h"
#include "adminjobs_l.h"
#include "dialog_l.h"
#include "jobmultitransfer_l.h"
#include <aqhbci/provider.h>
#include <aqbanking/job_be.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/imexporter.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>


GWEN_LIST_FUNCTIONS(AH_OUTBOX__CBOX, AH_Outbox__CBox);




AH_OUTBOX__CBOX *AH_Outbox__CBox_new(AH_HBCI *hbci,
				     AB_USER *u,
				     AH_OUTBOX *ob) {
  AH_OUTBOX__CBOX *cbox;

  assert(hbci);
  assert(u);
  GWEN_NEW_OBJECT(AH_OUTBOX__CBOX, cbox);
  cbox->usage=1;
  GWEN_LIST_INIT(AH_OUTBOX__CBOX, cbox);
  cbox->user=u;
  cbox->todoQueues=AH_JobQueue_List_new();
  cbox->finishedQueues=AH_JobQueue_List_new();
  cbox->todoJobs=AH_Job_List_new();
  cbox->finishedJobs=AH_Job_List_new();
  cbox->pendingJobs=AB_Job_List2_new();
  cbox->hbci=hbci;
  cbox->outbox=ob;

  return cbox;
}



void AH_Outbox__CBox_free(AH_OUTBOX__CBOX *cbox){
  if (cbox) {
    assert(cbox->usage);
    if (--(cbox->usage)==0) {
      GWEN_LIST_FINI(AH_OUTBOX__CBOX, cbox);
      AB_Job_List2_free(cbox->pendingJobs);
      AH_JobQueue_List_free(cbox->todoQueues);
      AH_JobQueue_List_free(cbox->finishedQueues);
      AH_Job_List_free(cbox->todoJobs);
      AH_Job_List_free(cbox->finishedJobs);

      GWEN_FREE_OBJECT(cbox);
    }
  }
}



void AH_Outbox__CBox_AddTodoJob(AH_OUTBOX__CBOX *cbox, AH_JOB *j) {
  assert(cbox);
  assert(j);

  AH_Job_SetStatus(j, AH_JobStatusToDo);
  AH_Job_List_Add(j, cbox->todoJobs);
}



void AH_Outbox__CBox_AddPendingJob(AH_OUTBOX__CBOX *cbox, AB_JOB *bj) {
  assert(cbox);
  assert(bj);

  AB_Job_List2_PushBack(cbox->pendingJobs, bj);
}



int AH_Outbox__CBox_CheckPending(AH_OUTBOX__CBOX *cbox) {
  AH_JOB *j;

  if (AB_Job_List2_GetSize(cbox->pendingJobs)==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No pending jobs to check");
    return 0;
  }

  j=AH_Job_List_First(cbox->finishedJobs);
  while(j) {
    if (AH_Job_GetStatus(j)!=AH_JobStatusError) {
      const char *s;
  
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Got a job without errors");
      s=AH_Job_GetName(j);
      assert(s);
      if (strcasecmp(s, "JobGetStatus")==0) {
        GWEN_DB_NODE *dbResponses;
        GWEN_DB_NODE *dbCurr;

        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Got a GetStatus job");
        dbResponses=AH_Job_GetResponses(j);
        assert(dbResponses);
        dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
        while (dbCurr) {
          GWEN_DB_NODE *dbResult;
          int rv;

          rv=AH_Job_CheckEncryption(j, dbCurr);
          if (rv) {
            DBG_WARN(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
            return rv;
          }
          rv=AH_Job_CheckSignature(j, dbCurr);
          if (rv) {
            DBG_WARN(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
            return rv;
          }

          dbResult=GWEN_DB_GetGroup(dbCurr,
                                    GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                    "data/GetStatusResponse");
          if (dbResult) {
            const char *rDialogId;
            uint32_t rMsgNum;
            uint32_t rSegNum;
            int rCode;
  
            DBG_DEBUG(AQHBCI_LOGDOMAIN, "Checking status response");
            rDialogId=GWEN_DB_GetCharValue(dbResult, "msgref/dialogId", 0, 0);
            rMsgNum=GWEN_DB_GetIntValue(dbResult, "msgref/msgNum", 0, 0);
            rSegNum=GWEN_DB_GetIntValue(dbResult, "refSegNum", 0, 0);
            rCode=GWEN_DB_GetIntValue(dbResult, "result/resultcode", 0, 0);

            if (rDialogId && rMsgNum && rSegNum && rCode) {
              AB_JOB_LIST2_ITERATOR *it;
    
              /* find pending job for this result */
              it=AB_Job_List2_First(cbox->pendingJobs);
              if (it) {
                AB_JOB *bj;
    
                bj=AB_Job_List2Iterator_Data(it);
                while(bj) {
                  GWEN_DB_NODE *dbJ;

                  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Checking pending job");
                  dbJ=AB_Job_GetProviderData(bj,
                                             AH_HBCI_GetProvider(cbox->hbci));
                  assert(dbJ);
                  dbJ=GWEN_DB_GetGroup(dbJ, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                       "msgRef");
                  if (dbJ) {
                    const char *jDialogId;
                    uint32_t jMsgNum;
                    uint32_t jFirstSeg;
                    uint32_t jLastSeg;
  
                    DBG_DEBUG(AQHBCI_LOGDOMAIN,
                             "Pending job has a message reference");
                    jDialogId=GWEN_DB_GetCharValue(dbJ, "dialogId", 0, 0);
                    jMsgNum=GWEN_DB_GetIntValue(dbJ, "msgnum", 0, 0);
                    jFirstSeg=GWEN_DB_GetIntValue(dbJ, "firstSeg", 0, 0);
                    jLastSeg=GWEN_DB_GetIntValue(dbJ, "lastSeg", 0, 0);
                    if (jDialogId && jMsgNum && jFirstSeg && jLastSeg) {
                      if ((rMsgNum==jMsgNum) &&
                          ((rSegNum>=jFirstSeg &&
                            rSegNum<=jLastSeg)) &&
                          (strcasecmp(rDialogId, jDialogId)==0)) {
                        /* result matches */
                        if (rCode>=9000 && rCode<=9999) {
                          DBG_INFO(AQHBCI_LOGDOMAIN,
                                   "Error result for pending job");
                          AB_Job_SetStatus(bj, AB_Job_StatusError);
                        }
                        else {
                          if (AB_Job_GetStatus(bj)==AB_Job_StatusPending) {
                            /* only modify status here if job is still
                             * pending. So if some result flagged an error
                             * the result will not be changed
                             */
                            if (rCode==10) {
                              DBG_INFO(AQHBCI_LOGDOMAIN,
                                       "Job is still pending");
                            }
                            else {
                              DBG_INFO(AQHBCI_LOGDOMAIN,
                                       "Pending job now finished");
                              AB_Job_SetStatus(bj, AB_Job_StatusFinished);
                            }
                          } /* if status is pending */
                          else {
                            DBG_INFO(AQHBCI_LOGDOMAIN,
                                     "Status is not \"pending\"");
                          }
                        } /* if non-error response */
                        break;
                      } /* if result is for this job */
                      else {
                        DBG_DEBUG(AQHBCI_LOGDOMAIN,
                                 "Result is not for this pending job");
                      }
                    } /* if all needed fields in job are valid */
                    else {
                      DBG_WARN(AQHBCI_LOGDOMAIN,
                               "Pending job is incomplete");
                    }
                  } /* if job has a message reference */
                  else {
                    DBG_WARN(AQHBCI_LOGDOMAIN,
                             "Pending job has no message reference");
                  }
                  bj=AB_Job_List2Iterator_Next(it);
                } /* while */
                AB_Job_List2Iterator_free(it);
              } /* if there are pending jobs */
            } /* if all needed fields in result are valid */
          } /* if current response group is a result */
          else {
            DBG_DEBUG(AQHBCI_LOGDOMAIN, "Not a status response");
          }
          dbCurr=GWEN_DB_GetNextGroup(dbCurr);
        } /* while */
      } /* if jobGetStatus */
    } /* if job is ok */
    else {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Skipping job, it has errors");
    }
    j=AH_Job_List_Next(j);
  } /* while jobs */

  return 0;
}



void AH_Outbox__CBox_Finish(AH_OUTBOX__CBOX *cbox){
  AH_JOBQUEUE *jq;

  assert(cbox);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Finishing customer box");
  while((jq=AH_JobQueue_List_First(cbox->finishedQueues))) {
    AH_JOB_LIST *jl;
    AH_JOB *j;

    jl=AH_JobQueue_TakeJobList(jq);
    assert(jl);
    while((j=AH_Job_List_First(jl))) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Moving job \"%s\" from finished queue to finished jobs",
               AH_Job_GetName(j));
      AH_Job_List_Del(j);
      AH_Job_List_Add(j, cbox->finishedJobs);
    } /* while */
    AH_Job_List_free(jl);
    AH_JobQueue_free(jq);
  } /* while */

  while((jq=AH_JobQueue_List_First(cbox->todoQueues))) {
    AH_JOB_LIST *jl;
    AH_JOB *j;

    jl=AH_JobQueue_TakeJobList(jq);
    assert(jl);
    while((j=AH_Job_List_First(jl))) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Moving job \"%s\" from todo queue to finished jobs",
               AH_Job_GetName(j));
      AH_Job_List_Del(j);
      AH_Job_List_Add(j, cbox->finishedJobs);
    } /* while */
    AH_Job_List_free(jl);
    AH_JobQueue_free(jq);
  } /* while */

  if (AH_Job_List_GetCount(cbox->todoJobs)) {
    AH_JOB *j;

    while((j=AH_Job_List_First(cbox->todoJobs))) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Moving job \"%s\" from todo queue to finished jobs",
               AH_Job_GetName(j));
      AH_Job_List_Del(j);
      AH_Job_List_Add(j, cbox->finishedJobs);
    } /* while */
  }


  /* check for results for pending jobs */
  AH_Outbox__CBox_CheckPending(cbox);
}



AB_USER *AH_Outbox__CBox_GetUser(const AH_OUTBOX__CBOX *cbox){
  assert(cbox);
  return cbox->user;
}



AH_JOB_LIST *AH_Outbox__CBox_TakeFinishedJobs(AH_OUTBOX__CBOX *cbox){
  AH_JOB_LIST *jl;

  assert(cbox);
  jl=cbox->finishedJobs;
  cbox->finishedJobs=AH_Job_List_new();
  return jl;
}



GWEN_TIME *AH_Outbox__CBox_GetEarliestPendingDate(AH_OUTBOX__CBOX *cbox) {
  AB_JOB_LIST2_ITERATOR *it;
  GWEN_TIME *ti;

  assert(cbox);
  assert(cbox->pendingJobs);
  ti=0;
  it=AB_Job_List2_First(cbox->pendingJobs);
  if (it) {
    AB_JOB *bj;

    bj=AB_Job_List2Iterator_Data(it);
    while(bj) {
      GWEN_DB_NODE *db;
      GWEN_TIME *tti;

      db=AB_Job_GetProviderData(bj, AH_HBCI_GetProvider(cbox->hbci));
      assert(db);

      tti=AB_Job_DateFromDb(db, "sendtime");
      if (tti) {
        if (!ti)
          ti=tti;
        else {
          if (GWEN_Time_Diff(tti, ti)>0) {
            GWEN_Time_free(ti);
            ti=tti;
          }
          else
            GWEN_Time_free(tti);
        }
      }

      bj=AB_Job_List2Iterator_Next(it);
    }
    AB_Job_List2Iterator_free(it);
  }

  return ti;
}



GWEN_TIME *AH_Outbox__CBox_GetLatestPendingDate(AH_OUTBOX__CBOX *cbox) {
  AB_JOB_LIST2_ITERATOR *it;
  GWEN_TIME *ti;

  assert(cbox);
  assert(cbox->pendingJobs);
  ti=0;
  it=AB_Job_List2_First(cbox->pendingJobs);
  if (it) {
    AB_JOB *bj;

    bj=AB_Job_List2Iterator_Data(it);
    while(bj) {
      GWEN_DB_NODE *db;
      GWEN_TIME *tti;

      db=AB_Job_GetProviderData(bj, AH_HBCI_GetProvider(cbox->hbci));
      assert(db);
      tti=AB_Job_DateFromDb(db, "sendtime");
      if (tti) {
        if (!ti)
          ti=tti;
        else {
          if (GWEN_Time_Diff(ti, tti)>0) {
            GWEN_Time_free(ti);
            ti=tti;
          }
          else
            GWEN_Time_free(tti);
        }
      }
      bj=AB_Job_List2Iterator_Next(it);
    }
    AB_Job_List2Iterator_free(it);
  }

  return ti;
}



int AH_Outbox__CBox_Prepare(AH_OUTBOX__CBOX *cbox){
  AH_JOB *j;
  unsigned int errors;
  AH_JOBQUEUE *jq;
  int firstJob;

  assert(cbox);

  errors=0;

  /* call AH_Job_Prepare() for all jobs */
  j=AH_Job_List_First(cbox->todoJobs);
  while(j) {
    AH_JOB_STATUS st;
    AH_JOB *next;

    next=AH_Job_List_Next(j);
    st=AH_Job_GetStatus(j);
    if (st==AH_JobStatusToDo) {
      int rv=AH_Job_Prepare(j);
      if (rv<0 && rv!=GWEN_ERROR_NOT_SUPPORTED) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	AH_Job_SetStatus(j, AH_JobStatusError);
	AH_Job_List_Del(j);
	AH_Job_List_Add(j, cbox->finishedJobs);
	errors++;
      }
    } /* if status TODO */
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Skip job \"%s\" for its status \"%s\" (%d)",
	       AH_Job_GetName(j), AH_Job_StatusName(st), st);
      AH_Job_SetStatus(j, AH_JobStatusError);
      AH_Job_List_Del(j);
      AH_Job_List_Add(j, cbox->finishedJobs);
      errors++;
    }

    j=next;
  } /* while */


  /* add JobGetStatus if there are any pending jobs for this customer */
  if (AB_Job_List2_GetSize(cbox->pendingJobs)) {
    AH_JOB *sj;
    GWEN_TIME *t1;
    GWEN_TIME *t2;

    /* have some pending jobs, add status call */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Will ask for status reports");
    t1=AH_Outbox__CBox_GetEarliestPendingDate(cbox);
    t2=AH_Outbox__CBox_GetLatestPendingDate(cbox);
    sj=AH_Job_GetStatus_new(cbox->user, t1, t2);
    GWEN_Time_free(t2);
    GWEN_Time_free(t1);
    if (sj) {
      AH_Job_AddFlags(sj, AH_JOB_FLAGS_OUTBOX);
      AH_Job_AddSigner(sj, AB_User_GetUserId(cbox->user));
      AH_Outbox__CBox_AddTodoJob(cbox, sj);
    }
    else {
      DBG_WARN(AQHBCI_LOGDOMAIN,
               "JobGetStatus not available, cannot check pending jobs");
    }
  } /* if attached jobs */

  /* move all dialog jobs to new queues or to the list of finished jobs */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing dialog jobs");
  j=AH_Job_List_First(cbox->todoJobs);
  while(j) {
    AH_JOB_STATUS st;
    AH_JOB *next;

    next=AH_Job_List_Next(j);
    st=AH_Job_GetStatus(j);
    if (st==AH_JobStatusToDo) {
      if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_DLGJOB) {
        /* this is a dialog job, create a new queue for it */
        AH_JOBQUEUE *jq;

        DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing dialog job \"%s\"",
                 AH_Job_GetName(j));
        jq=AH_JobQueue_new(cbox->user);
        AH_Job_List_Del(j);
        if (AH_JobQueue_AddJob(jq, j)!=AH_JobQueueAddResultOk) {
          /* error adding a single job to the queue */
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Could not add dialog job \"%s\" to queue",
                    AH_Job_GetName(j));
          /* set status to ERROR and move to finished queue */
          AH_Job_SetStatus(j, AH_JobStatusError);
          AH_Job_List_Add(j, cbox->finishedJobs);
          AH_JobQueue_free(jq);
          errors++;
        }
        else {
          AH_Job_Log(j, GWEN_LoggerLevel_Info,
                     "Dialog job enqueued");
          /* job added. This is a dialog job, so we need to begin and
           * and end the dialog */
          AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_BEGINDIALOG);
          /*AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_ENDDIALOG); DEBUG */
          AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_ISDIALOG);
          AH_JobQueue_List_Add(jq, cbox->todoQueues);
        } /* if added to queue */
      } /* if dialog job */
    } /* if status TODO */
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Skip job \"%s\" for its status \"%s\" (%d)",
               AH_Job_GetName(j), AH_Job_StatusName(st), st);
      AH_Job_List_Add(j, cbox->finishedJobs);
    }

    j=next;
  } /* while */

  /* now todoJobs only contains non-dialog jobs with a correct status,
   * append them to new queues as needed */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing non-dialog jobs");
  jq=AH_JobQueue_new(cbox->user);
  AH_JobQueue_AddFlags(jq, AH_JOBQUEUE_FLAGS_BEGINDIALOG);
  firstJob=1;
  DBG_INFO(AQHBCI_LOGDOMAIN, "We have %d jobs to handle",
           AH_Job_List_GetCount(cbox->todoJobs));
  while(AH_Job_List_GetCount(cbox->todoJobs)) {
    int jobsAdded;
    int queueCreated;
    AH_JOB_LIST *retryJobs;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Still some jobs left todo");
    jobsAdded=0;
    queueCreated=0;
    retryJobs=AH_Job_List_new();
    while( (j=AH_Job_List_First(cbox->todoJobs)) ) {
      AH_JOBQUEUE_ADDRESULT res;

      DBG_INFO(AQHBCI_LOGDOMAIN, "Queueing job \"%s\"", AH_Job_GetName(j));
      AH_Job_List_Del(j);
      res=AH_JobQueue_AddJob(jq, j);
      if (res!=AH_JobQueueAddResultOk) {
	DBG_INFO(AQHBCI_LOGDOMAIN,
		 "Could not add job \"%s\" to the current queue",
		 AH_Job_GetName(j));

        if (firstJob) {
          /* error adding a single job to the queue */
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add single non-dialog job \"%s\" to queue",
                    AH_Job_GetName(j));
          /* set status to ERROR and move to finished queue */
          AH_Job_SetStatus(j, AH_JobStatusError);
          AH_Job_List_Add(j, cbox->finishedJobs);
          AH_Job_Log(j, GWEN_LoggerLevel_Error,
                     "Could not enqueing HBCI-job");
          errors++;
        } /* if first job failed */
        else {
          /* not the first job, check for reason of failure */
          if (res==AH_JobQueueAddResultQueueFull) {
            /* queue is full, so add it to the todo queue list and start
             * a new queue */
            DBG_INFO(AQHBCI_LOGDOMAIN, "Queue full, starting next one");
            AH_JobQueue_List_Add(jq, cbox->todoQueues);
            jq=AH_JobQueue_new(cbox->user);
            firstJob=1;
	    queueCreated=1;
	    /* put job back into queue (same pos, try it again in next loop)*/
	    AH_Job_List_Insert(j, cbox->todoJobs);
	    break;
          }
	  else if (res==AH_JobQueueAddResultJobLimit) {
	    DBG_INFO(AQHBCI_LOGDOMAIN,
		     "Job \"%s\" does not fit into queue, will retry later",
		     AH_Job_GetName(j));
	    /* move job to the end of the queue (retry it later) */
            AH_Job_List_Add(j, retryJobs);
          }
	  else {
            /* error adding a job to the queue */
            DBG_ERROR(AQHBCI_LOGDOMAIN,
                      "Could not add non-dialog job \"%s\" to queue for "
                      "unknown reason %d",
                      AH_Job_GetName(j), res);
            /* set status to ERROR and move to finished queue */
            AH_Job_SetStatus(j, AH_JobStatusError);
            AH_Job_List_Add(j, cbox->finishedJobs);
            errors++;
          }
        } /* if it wasn't the first job to fail */
      } /* if adding to the queue failed */
      else {
        /* job added successfully */
        DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" successfully added",
                 AH_Job_GetName(j));
        AH_Job_Log(j, GWEN_LoggerLevel_Info,
                   "HBCI-job enqueued (1)");
        firstJob=0;
        jobsAdded++;
      }
    } /* while */

    /* put back all jobs we dismissed */
    j=AH_Job_List_First(retryJobs);
    if (j) {
      while(j) {
	AH_JOB *jnext;
        jnext=AH_Job_List_Next(j);
	DBG_NOTICE(AQHBCI_LOGDOMAIN,
		   "Moving job \"%s\" back to queue",
		   AH_Job_GetName(j));
	AH_Job_List_Del(j);
	AH_Job_List_Add(j, cbox->todoJobs);
	j=jnext;
      }

      /* there are some retry jobs, retry them */
      if (AH_JobQueue_GetCount(jq)!=0) {
	AH_JobQueue_List_Add(jq, cbox->todoQueues);
	jq=AH_JobQueue_new(cbox->user);
	firstJob=1;
	queueCreated=1;
      }
    }
    AH_Job_List_free(retryJobs);
    retryJobs=NULL;

    /* check whether we could do something in the last loop */
    if (!jobsAdded && !queueCreated) {
      AH_JOB *j;

      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could do nothing in last loop, "
                "discarding remaining jobs");
      j=AH_Job_List_First(cbox->todoJobs);
      while(j) {
        AH_Job_SetStatus(j, AH_JobStatusError);
        AH_Job_List_Del(j);
        AH_Job_List_Add(j, cbox->finishedJobs);
        errors++;
        j=AH_Job_List_Next(j);
      } /* while */

      /* break the loop */
      break;
    } /* if we couldn't do anything */
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Still something to do");
    }
  } /* while still todoJobs */

  /* check whether to free the current queue */
  if (AH_JobQueue_GetCount(jq)==0) {
    /* current queue is empty, free it */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Last queue is empty, deleting it");
    AH_JobQueue_free(jq);
  }
  else {
    /* it is not, so add it to the todo list */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Adding last queue");
    AH_JobQueue_List_Add(jq, cbox->todoQueues);
  }

  if (errors) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Some errors (%d) occurred", errors);
    return -1;
  }

  return 0;
}



int AH_Outbox__CBox_SendQueue(AH_OUTBOX__CBOX *cbox,
			      AH_DIALOG *dlg,
			      AH_JOBQUEUE *jq){
  AH_MSG *msg;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding queue");
  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Info,
		       I18N("Encoding queue"));
  msg=AH_JobQueue_ToMessage(jq, dlg);
  if (!msg) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not encode queue");
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Unable to encode"));
    return GWEN_ERROR_GENERIC;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Sending queue");
  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Info,
		       I18N("Sending queue"));

  rv=AH_Dialog_SendMessage(dlg, msg);
  AH_Msg_free(msg);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not send message");
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Unable to send (network error)"));
    return rv;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Message sent");
  return 0;
}



int AH_Outbox__CBox_RecvQueue(AH_OUTBOX__CBOX *cbox,
			      AH_DIALOG *dlg,
			      AH_JOBQUEUE *jq){
  AH_MSG *msg;
  GWEN_DB_NODE *rsp;
  int rv;

  assert(cbox);

  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Info,
		       I18N("Waiting for response"));

  rv=AH_Dialog_RecvMessage(dlg, &msg);
  if (rv>=200 && rv<300)
    rv=0;
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Error receiving response (%d)", rv);
    GWEN_Gui_ProgressLog2(0,
                          GWEN_LoggerLevel_Error,
                          I18N("Error receiving response (%d)"), rv);
    return rv;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Got a message");
  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Info,
		       I18N("Response received"));

  /* try to dispatch the message */
  rsp=GWEN_DB_Group_new("response");
  if (AH_Msg_DecodeMsg(msg, rsp, GWEN_MSGENGINE_READ_FLAGS_DEFAULT)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not decode this message:");
    AH_Msg_Dump(msg, 2);
    GWEN_DB_Group_free(rsp);
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Bad response (unable to decode)"));
    return GWEN_ERROR_GENERIC;
  }

  /* transform from ISO 8859-1 to UTF8 */
  AB_ImExporter_DbFromIso8859_1ToUtf8(rsp);

  /* check for message reference */
  if (AH_Msg_GetMsgRef(msg)==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unrequested message, deleting it");
    AH_Msg_Dump(msg, 2);
    GWEN_DB_Dump(rsp, 2);
    GWEN_DB_Group_free(rsp);
    AH_Msg_free(msg);
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("Bad response (bad message reference)"));
    return GWEN_ERROR_GENERIC;
  }

  rv=AH_JobQueue_DispatchMessage(jq, msg, rsp);
  if (rv) {
    if (rv==GWEN_ERROR_ABORTED) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Dialog aborted by server");
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   I18N("Dialog aborted by server"));
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not dispatch response");
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   I18N("Bad response (unable to dispatch)"));
    }
    GWEN_DB_Group_free(rsp);
    AH_Msg_free(msg);
    return rv;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Message dispatched");
  GWEN_DB_Group_free(rsp);
  AH_Msg_free(msg);
  return 0;
}



int AH_Outbox__CBox_SendAndRecvQueue(AH_OUTBOX__CBOX *cbox,
				     AH_DIALOG *dlg,
				     AH_JOBQUEUE *jq){
  int rv;

  if ((AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_NEEDTAN) &&
      AH_Dialog_GetItanProcessType(dlg)!=0) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "iTAN mode");

    rv=AH_Outbox__CBox_Itan(cbox, dlg, jq);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Normal mode");
    rv=AH_Outbox__CBox_SendQueue(cbox, dlg, jq);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error sending queue");
      return rv;
    }

    AH_JobQueue_SetJobStatusOnMatch(jq, AH_JobStatusEncoded,
                                    AH_JobStatusSent);

    rv=AH_Outbox__CBox_RecvQueue(cbox, dlg, jq);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error receiving queue response");
      return rv;
    }
  }

  return 0;
}




int AH_Outbox__CBox_OpenDialog(AH_OUTBOX__CBOX *cbox,
			       AH_DIALOG *dlg,
			       uint32_t jqFlags) {
  AH_JOBQUEUE *jqDlgOpen;
  AH_JOB *jDlgOpen;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating dialog open request");
  AH_Dialog_SetItanProcessType(dlg, 0);

  if ((jqFlags & AH_JOBQUEUE_FLAGS_CRYPT) ||
      (jqFlags & AH_JOBQUEUE_FLAGS_SIGN)) {
    /* sign and crypt, not anonymous */
    DBG_NOTICE(AQHBCI_LOGDOMAIN,
               "Creating non-anonymous dialog open request");
    jDlgOpen=AH_Job_new("JobDialogInit", cbox->user, 0, 0);
    if (!jDlgOpen) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create job JobDialogInit");
      return GWEN_ERROR_GENERIC;
    }
    if (jqFlags & AH_JOBQUEUE_FLAGS_SIGN)
      AH_Job_AddSigner(jDlgOpen,
                       AB_User_GetUserId(cbox->user));
    AH_Dialog_SubFlags(dlg, AH_DIALOG_FLAGS_ANONYMOUS);

    if (AH_User_GetCryptMode(cbox->user)==AH_CryptMode_Pintan) {
      if (AH_User_HasTanMethodOtherThan(cbox->user, 999) &&
	  !(jqFlags & AH_JOBQUEUE_FLAGS_NOITAN)) {
	/* only use itan if any other mode than singleStep is available
	 * and the job queue does not request non-ITAN mode
	 */
	rv=AH_Outbox__CBox_SelectItanMode(cbox, dlg);
	if (rv) {
	  DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	  return rv;
	}
      }
    }
  }
  else {
    /* neither sign nor crypt, use anonymous dialog */
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating anonymous dialog open request");
    jDlgOpen=AH_Job_new("JobDialogInitAnon", cbox->user, 0, 0);
    if (!jDlgOpen) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create job JobDialogInitAnon");
      return GWEN_ERROR_GENERIC;
    }
    AH_Dialog_AddFlags(dlg, AH_DIALOG_FLAGS_ANONYMOUS);
  }

  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Notice,
		       I18N("Opening dialog"));
  jqDlgOpen=AH_JobQueue_new(cbox->user);
  AH_JobQueue_AddFlags(jqDlgOpen, AH_JOBQUEUE_FLAGS_OUTBOX);
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Enqueueing dialog open request");
  if (AH_JobQueue_AddJob(jqDlgOpen, jDlgOpen)!=
      AH_JobQueueAddResultOk) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add single job to queue");
    AH_Job_free(jDlgOpen);
    AH_JobQueue_free(jqDlgOpen);
    return GWEN_ERROR_GENERIC;
  }

  rv=AH_Outbox__CBox_SendAndRecvQueue(cbox, dlg, jqDlgOpen);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not exchange message");
    AH_JobQueue_free(jqDlgOpen);
    return rv;
  }
  if (AH_Job_HasErrors(jDlgOpen)) {
    /* TODO: check for iTAN related error and try again */
    if (AH_Job_HasItanResult(jDlgOpen)) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN,
		 "Adjusting to iTAN modes of the server");
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Notice,
			   I18N("Adjusting to iTAN modes of the server"));
      /* do not call AH_Job_CommitSystemData() here, the iTAN modes have already
       * been caught by AH_JobQueue_DispatchMessage()
        AH_Job_CommitSystemData(jDlgOpen); */
      AH_JobQueue_free(jqDlgOpen);
      return 1;
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error opening dialog, aborting");
      AH_JobQueue_free(jqDlgOpen);
      return GWEN_ERROR_GENERIC;
    }
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Dialog open request done");
  rv=AH_Job_CommitSystemData(jDlgOpen, 0);
  AH_JobQueue_free(jqDlgOpen);
  return rv;
}



int AH_Outbox__CBox_CloseDialog(AH_OUTBOX__CBOX *cbox,
				AH_DIALOG *dlg,
				uint32_t jqFlags) {
  AH_JOBQUEUE *jqDlgClose;
  AH_JOB *jDlgClose;
  GWEN_DB_NODE *db;
  uint32_t dlgFlags;
  int rv;

  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Notice,
		       I18N("Closing dialog"));
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Sending dialog close request (flags=%08x)",
	     jqFlags);
  dlgFlags=AH_Dialog_GetFlags(dlg);
  jDlgClose=AH_Job_new("JobDialogEnd", cbox->user, 0, 0);
  if (!jDlgClose) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create job JobDialogEnd");
    return GWEN_ERROR_GENERIC;
  }

  /* set some parameters */
  db=AH_Job_GetArguments(jDlgClose);
  assert(db);
  GWEN_DB_SetCharValue(db,
		       GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "dialogId",
		       AH_Dialog_GetDialogId(dlg));

  if (!(dlgFlags & AH_DIALOG_FLAGS_ANONYMOUS)) {
    /* sign and encrypt job */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Will encrypt and sign dialog close request");
    AH_Job_AddSigner(jDlgClose, AB_User_GetUserId(cbox->user));
    AH_Job_AddFlags(jDlgClose,
		    AH_JOB_FLAGS_CRYPT |
		    AH_JOB_FLAGS_SIGN);
  }
  jqDlgClose=AH_JobQueue_new(cbox->user);

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Adding dialog close request to queue");
  if (AH_JobQueue_AddJob(jqDlgClose, jDlgClose)!=
      AH_JobQueueAddResultOk) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add single job to queue");
    AH_JobQueue_free(jqDlgClose);
    return GWEN_ERROR_GENERIC;
  }

  rv=AH_Outbox__CBox_SendAndRecvQueue(cbox, dlg, jqDlgClose);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not exchange message");
    AH_JobQueue_free(jqDlgClose);
    return rv;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Dialog closed");
  rv=AH_Job_CommitSystemData(jDlgClose, 0);
  AH_JobQueue_free(jqDlgClose);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not commit system data");
    return rv;
  }
  return 0;
}



void AH_Outbox__CBox_HandleQueueError(AH_OUTBOX__CBOX *cbox,
                                      AH_JOBQUEUE *jq,
                                      const char *logStr) {
  AH_JOB *j;
  AH_JOB_LIST *jl;

  jl=AH_JobQueue_TakeJobList(jq);
  assert(jl);

  while((j=AH_Job_List_First(jl))) {
    AH_Job_List_Del(j);
    if (AH_Job_GetStatus(j)!=AH_JobStatusAnswered) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Setting status of job \"%s\" to ERROR",
               AH_Job_GetName(j));
      AH_Job_SetStatus(j, AH_JobStatusError);
      if (logStr)
        AH_Job_Log(j, GWEN_LoggerLevel_Error, logStr);
    }
    AH_Job_List_Add(j, cbox->finishedJobs);
  }
  AH_Job_List_free(jl);
  AH_JobQueue_free(jq);
}



int AH_Outbox__CBox_PerformQueue(AH_OUTBOX__CBOX *cbox,
				 AH_DIALOG *dlg,
				 AH_JOBQUEUE *jq) {
  int rv;

  for (;;) {
    AH_JOBQUEUE *jqTodo;
    int jobsTodo;
    uint32_t jqFlags;
    AH_JOB *j;
    AH_JOB_LIST *jl;

    jobsTodo=0;
    jl=AH_JobQueue_TakeJobList(jq);
    assert(jl);
    jqTodo=AH_JobQueue_new(AH_JobQueue_GetUser(jq));
    /* copy some flags */
    jqFlags=AH_JobQueue_GetFlags(jq);
    jqFlags&=~(AH_JOBQUEUE_FLAGS_CRYPT |
               AH_JOBQUEUE_FLAGS_SIGN |
               AH_JOBQUEUE_FLAGS_NOSYSID |
               AH_JOBQUEUE_FLAGS_NOITAN);
    AH_JobQueue_SetFlags(jqTodo, (jqFlags&AH_JOBQUEUE_FLAGS_COPYMASK));

    /* copy todo jobs */
    while((j=AH_Job_List_First(jl))) {
      AH_Job_List_Del(j);
      if (AH_Job_GetStatus(j)==AH_JobStatusAnswered) {
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message finished");
	/* prepare job for next message
	 * (if attachpoint or multi-message job)
	 */
	AH_Job_PrepareNextMessage(j);
	if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_HASMOREMSGS) {
	  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Requeueing job");
	  /* we shall redo this job */
	  if (AH_JobQueue_AddJob(jqTodo, j)!=
	      AH_JobQueueAddResultOk){
	    DBG_ERROR(AQHBCI_LOGDOMAIN,
		      "That's weird, I could not add the job to redo queue");
            AH_Job_Log(j, GWEN_LoggerLevel_Error,
                       "Could not re-enqueue HBCI-job");
            AH_Job_SetStatus(j, AH_JobStatusError);
	  }
	  else {
	    jobsTodo++;
            AH_Job_Log(j, GWEN_LoggerLevel_Info,
                       "HBCI-job re-enqueued (multi-message job)");
	    j=0; /* mark that this job has been dealt with */
	  }
	} /* if more messages */
	else {
	  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Not requeing job");
	}
      } /* if status matches */
      else if (AH_Job_GetStatus(j)==AH_JobStatusEnqueued) {
	if (AH_JobQueue_AddJob(jqTodo, j)!=
	    AH_JobQueueAddResultOk){
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "That's weird, I could not add the job to redo queue");
	  AH_Job_SetStatus(j, AH_JobStatusError);
          AH_Job_Log(j, GWEN_LoggerLevel_Error,
                     "Could not enqueue HBCI-job");
	}
	else {
	  jobsTodo++;
          AH_Job_Log(j, GWEN_LoggerLevel_Info,
                     "HBCI-job enqueued (2)");
	  j=0; /* mark that this job has been dealt with */
	}
      }
      else {
	DBG_DEBUG(AQHBCI_LOGDOMAIN, "Bad status \"%s\" (%d)",
		  AH_Job_StatusName(AH_Job_GetStatus(j)),
		  AH_Job_GetStatus(j));
	if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
	  AH_Job_Dump(j, stderr, 4);
      }
      if (j) {
	/* move job to finished list if we still have the job */
	AH_Job_List_Add(j, cbox->finishedJobs);
      }
    } /* while */

    AH_Job_List_free(jl);
    AH_JobQueue_free(jq);
    jq=jqTodo;

    if (!jobsTodo)
      break;

    /* jq now contains all jobs to be executed */
    rv=AH_Outbox__CBox_SendAndRecvQueue(cbox, dlg, jq);
    if (rv) {
      AH_Outbox__CBox_HandleQueueError(cbox, jq,
                                       "Error performing queue");
      return rv;
    } /* if error */
  } /* for */

  AH_JobQueue_free(jq);
  return 0;
}



int AH_Outbox__CBox_PerformNonDialogQueues(AH_OUTBOX__CBOX *cbox,
					   AH_JOBQUEUE_LIST *jql){
  AH_DIALOG *dlg;
  AH_JOBQUEUE *jq;
  int rv=0;
  int i;
  uint32_t jqflags;

  if (AH_JobQueue_List_GetCount(jql)==0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No queues to handle, doing nothing");
    AH_JobQueue_List_free(jql);
    return 0;
  }

  for(i=0; i<2; i++) {
    dlg=AH_Dialog_new(cbox->user);
    rv=AH_Dialog_Connect(dlg);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
	       "Could not begin a dialog for customer \"%s\" (%d)",
	       AB_User_GetCustomerId(cbox->user), rv);
      /* finish all queues */
      AH_Outbox__CBox_HandleQueueListError(cbox, jql,
					   "Could not begin dialog");
      AH_Dialog_free(dlg);
      return rv;
    }
  
    jq=AH_JobQueue_List_First(jql);
    jqflags=AH_JobQueue_GetFlags(jq);
  
    /* open dialog */
    rv=AH_Outbox__CBox_OpenDialog(cbox, dlg, jqflags);
    if (rv==0)
      break;
    else if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Could not open dialog");
      AH_Dialog_Disconnect(dlg);
      /* finish all queues */
      AH_Outbox__CBox_HandleQueueListError(cbox, jql,
					   "Could not open dialog");
      AH_Dialog_free(dlg);
      return rv;
    }
    else if (rv==1) {
      AH_Dialog_Disconnect(dlg);
      AH_Dialog_free(dlg);
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Info,
			   I18N("Retrying to open dialog"));
    }
  }
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not open dialog");
    AH_Dialog_Disconnect(dlg);
    /* finish all queues */
    AH_Outbox__CBox_HandleQueueListError(cbox, jql,
					 "Could not open dialog");
    AH_Dialog_free(dlg);
    return rv;
  }

  /* handle queues */
  rv=0;
  while((jq=AH_JobQueue_List_First(jql))) {
    AH_JobQueue_List_Del(jq);
    rv=AH_Outbox__CBox_PerformQueue(cbox, dlg, jq);
    if (rv)
      break;
  } /* while */

  if (rv) {
    /* finish all remaining queues */
    AH_Outbox__CBox_HandleQueueListError(cbox, jql,
                                         "Could not send ");
    AH_Dialog_Disconnect(dlg);
    AH_Dialog_free(dlg);
    return rv;
  }

  /* close dialog */
  rv=AH_Outbox__CBox_CloseDialog(cbox, dlg, jqflags);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not close dialog, ignoring");
    /*AH_HBCI_EndDialog(cbox->hbci, dlg);
     return rv;*/
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Closing connection");
  AH_Dialog_Disconnect(dlg);
  AH_Dialog_free(dlg);

  AH_JobQueue_List_free(jql);
  return 0;
}



int AH_Outbox__CBox_PerformDialogQueue(AH_OUTBOX__CBOX *cbox,
				       AH_JOBQUEUE *jq){
  AH_DIALOG *dlg;
  int rv;

  /* open connection */
  dlg=AH_Dialog_new(cbox->user);
  rv=AH_Dialog_Connect(dlg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not begin a dialog for customer \"%s\" (%d)",
             AB_User_GetCustomerId(cbox->user), rv);
    /* finish all queues */
    AH_Outbox__CBox_HandleQueueError(cbox, jq,
                                     "Could not begin dialog");
    AH_Dialog_free(dlg);
    return rv;
  }

  /* select iTAN mode */
  if (!(AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_NOITAN)) {
    rv=AH_Outbox__CBox_SelectItanMode(cbox, dlg);
    if (rv) {
      AH_Dialog_Disconnect(dlg);
      AH_Dialog_free(dlg);
      return rv;
    }
  }

  /* handle queue */
  rv=AH_Outbox__CBox_PerformQueue(cbox, dlg, jq);
  if (rv) {
    AH_Dialog_Disconnect(dlg);
    AH_Dialog_free(dlg);
    return rv;
  }

  /* close connection */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Closing connection");
  AH_Dialog_Disconnect(dlg);
  AH_Dialog_free(dlg);

  return 0;
}



void AH_Outbox__CBox_ExtractMatchingQueues(AH_JOBQUEUE_LIST *jql,
					   AH_JOBQUEUE_LIST *jqlWanted,
					   AH_JOBQUEUE_LIST *jqlRest,
					   uint32_t jqflags,
					   uint32_t jqmask) {
  AH_JOBQUEUE *jq;

  while((jq=AH_JobQueue_List_First(jql))) {
    uint32_t flags;

    AH_JobQueue_List_Del(jq);
    flags=AH_JobQueue_GetFlags(jq);
    if ((flags^jqflags)  & jqmask)
      /* no match */
      AH_JobQueue_List_Add(jq, jqlRest);
    else
      AH_JobQueue_List_Add(jq, jqlWanted);
  } /* while */
}



void AH_Outbox__CBox_HandleQueueListError(AH_OUTBOX__CBOX *cbox,
                                          AH_JOBQUEUE_LIST *jql,
                                          const char *logStr){
  AH_JOBQUEUE *jq;

  while((jq=AH_JobQueue_List_First(jql))) {
    AH_JobQueue_List_Del(jq);
    AH_Outbox__CBox_HandleQueueError(cbox, jq, logStr);
  } /* while */
  AH_JobQueue_List_free(jql);
}



int AH_Outbox__CBox_SendAndRecvDialogQueues(AH_OUTBOX__CBOX *cbox) {
  AH_JOBQUEUE_LIST *jqlWanted;
  AH_JOBQUEUE_LIST *jqlRest;
  int rv;

  jqlWanted=AH_JobQueue_List_new();
  jqlRest=AH_JobQueue_List_new();
  AH_Outbox__CBox_ExtractMatchingQueues(cbox->todoQueues,
					jqlWanted,
					jqlRest,
					AH_JOBQUEUE_FLAGS_ISDIALOG,
                                        AH_JOBQUEUE_FLAGS_ISDIALOG);
  AH_JobQueue_List_free(cbox->todoQueues);
  cbox->todoQueues=jqlRest;
  if (AH_JobQueue_List_GetCount(jqlWanted)) {
    AH_JOBQUEUE *jq;

    /* there are matching queues, handle them */
    while((jq=AH_JobQueue_List_First(jqlWanted))) {
      AH_JobQueue_List_Del(jq);
      rv=AH_Outbox__CBox_PerformDialogQueue(cbox, jq);
      if (rv) {
	DBG_INFO(AQHBCI_LOGDOMAIN,
		 "Error performing queue (%d)", rv);
        AH_Outbox__CBox_HandleQueueListError(cbox, jqlWanted,
                                             "Could not perform "
                                             "dialog queue");
        AH_Outbox__CBox_HandleQueueListError(cbox, cbox->todoQueues,
                                             "Could not perform "
                                             "dialog queue");
        cbox->todoQueues=AH_JobQueue_List_new();
	return rv;
      }
    } /* while */
  }
  AH_JobQueue_List_free(jqlWanted);
  return 0;
}



int AH_Outbox__CBox_SendAndRecvSelected(AH_OUTBOX__CBOX *cbox,
					uint32_t jqflags,
					uint32_t jqmask){
  AH_JOBQUEUE_LIST *jqlWanted;
  AH_JOBQUEUE_LIST *jqlRest;
  int rv;

  jqlWanted=AH_JobQueue_List_new();
  jqlRest=AH_JobQueue_List_new();
  AH_Outbox__CBox_ExtractMatchingQueues(cbox->todoQueues,
					jqlWanted,
					jqlRest, jqflags, jqmask);
  AH_JobQueue_List_free(cbox->todoQueues);
  cbox->todoQueues=jqlRest;
  if (AH_JobQueue_List_GetCount(jqlWanted)) {
    /* there are matching queues, handle them */
    rv=AH_Outbox__CBox_PerformNonDialogQueues(cbox, jqlWanted);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"Error performing queue (%d)", rv);
      AH_Outbox__CBox_HandleQueueListError(cbox, cbox->todoQueues,
                                           "Error performing "
                                           "selected jobs");
      cbox->todoQueues=AH_JobQueue_List_new();
      return rv;
    }
  } /* if matching queuees */
  else
    AH_JobQueue_List_free(jqlWanted);
  return 0;
}



int AH_Outbox__CBox_SendAndRecvBox(AH_OUTBOX__CBOX *cbox){
  int rv;

  /* dialog queues */
  rv=AH_Outbox__CBox_SendAndRecvDialogQueues(cbox);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing dialog queues (%d)", rv);
    return rv;
  }

  /* non-dialog queues: unsigned, uncrypted */
  rv=AH_Outbox__CBox_SendAndRecvSelected(cbox,
					 0,
					 AH_JOBQUEUE_FLAGS_ISDIALOG |
					 AH_JOBQUEUE_FLAGS_SIGN |
					 AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (-S, -C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: unsigned, crypted */
  rv=AH_Outbox__CBox_SendAndRecvSelected(cbox,
					 AH_JOBQUEUE_FLAGS_CRYPT,
					 AH_JOBQUEUE_FLAGS_ISDIALOG |
					 AH_JOBQUEUE_FLAGS_SIGN |
					 AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (-S, +C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: signed, uncrypted */
  rv=AH_Outbox__CBox_SendAndRecvSelected(cbox,
					 AH_JOBQUEUE_FLAGS_SIGN,
					 AH_JOBQUEUE_FLAGS_ISDIALOG |
					 AH_JOBQUEUE_FLAGS_SIGN |
					 AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (+S, -C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: signed, crypted */
  rv=AH_Outbox__CBox_SendAndRecvSelected(cbox,
					 AH_JOBQUEUE_FLAGS_SIGN |
					 AH_JOBQUEUE_FLAGS_CRYPT,
					 AH_JOBQUEUE_FLAGS_ISDIALOG |
					 AH_JOBQUEUE_FLAGS_SIGN |
					 AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (+S, +C: %d)", rv);
    return rv;
  }

  return 0;
}




















AH_OUTBOX *AH_Outbox_new(AH_HBCI *hbci) {
  AH_OUTBOX *ob;

  assert(hbci);
  GWEN_NEW_OBJECT(AH_OUTBOX, ob);
  GWEN_INHERIT_INIT(AH_OUTBOX, ob);

  ob->hbci=hbci;
  ob->userBoxes=AH_Outbox__CBox_List_new();
  ob->finishedJobs=AH_Job_List_new();
  ob->usage=1;
  return ob;
}



void AH_Outbox_free(AH_OUTBOX *ob){
  if (ob) {
    assert(ob->usage);
    if (--(ob->usage)==0) {
      AH_Outbox__CBox_List_free(ob->userBoxes);
      AH_Job_List_free(ob->finishedJobs);
      GWEN_INHERIT_FINI(AH_OUTBOX, ob);
      GWEN_FREE_OBJECT(ob);
    }
  }
}



void AH_Outbox_Attach(AH_OUTBOX *ob){
  assert(ob);
  ob->usage++;
}





int AH_Outbox_Prepare(AH_OUTBOX *ob){
  AH_OUTBOX__CBOX *cbox;
  unsigned int errors;

  assert(ob);

  errors=0;
  cbox=AH_Outbox__CBox_List_First(ob->userBoxes);
  while(cbox) {
    AB_USER *u;

    u=AH_Outbox__CBox_GetUser(cbox);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing queues for customer \"%s\"",
             AB_User_GetCustomerId(u));
    if (AH_Outbox__CBox_Prepare(cbox)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error preparing cbox");
      errors++;
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing queues for customer \"%s\": done",
               AB_User_GetCustomerId(u));
    }
    cbox=AH_Outbox__CBox_List_Next(cbox);
  } /* while */

  if (errors) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "%d errors occurred", errors);
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int AH_Outbox_LockUsers(AH_OUTBOX *ob, AB_USER_LIST2 *lockedUsers){
  AH_OUTBOX__CBOX *cbox;
  AB_BANKING *ab;

  assert(ob);

  ab=AH_HBCI_GetBankingApi(ob->hbci);

  cbox=AH_Outbox__CBox_List_First(ob->userBoxes);
  while(cbox) {
    int rv;
    char tbuf[256];

    DBG_INFO(AQHBCI_LOGDOMAIN, "Locking customer \"%s\"",
	     AB_User_GetCustomerId(cbox->user));
    snprintf(tbuf, sizeof(tbuf)-1,
	     I18N("Locking user %s"),
	     AB_User_GetUserId(cbox->user));
    tbuf[sizeof(tbuf)-1]=0;
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Info,
			 tbuf);
    rv=AB_Banking_BeginExclUseUser(ab, cbox->user);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
	       "Could not lock customer [%s] (%d)",
	       AB_User_GetCustomerId(cbox->user), rv);
      snprintf(tbuf, sizeof(tbuf)-1,
	       I18N("Could not lock user %s (%d)"),
	       AB_User_GetUserId(cbox->user), rv);
      tbuf[sizeof(tbuf)-1]=0;
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   tbuf);
      AH_Outbox_UnlockUsers(ob, lockedUsers, 1); /* abandon */
      return rv;
    }
    cbox->isLocked=1;
    AB_User_List2_PushBack(lockedUsers, cbox->user);

    cbox=AH_Outbox__CBox_List_Next(cbox);
  } /* while */

  return 0;
}



int AH_Outbox_UnlockUsers(AH_OUTBOX *ob, AB_USER_LIST2 *lockedUsers,
			  int abandon){
  int errors=0;
  AB_BANKING *ab;
  AB_USER_LIST2_ITERATOR *it;

  assert(ob);

  ab=AH_HBCI_GetBankingApi(ob->hbci);
  it=AB_User_List2_First(lockedUsers);
  if (it) {
    AB_USER *u;

    u=AB_User_List2Iterator_Data(it);
    while(u) {
      int rv;

      DBG_INFO(AQHBCI_LOGDOMAIN, "Unlocking customer \"%s\"",
	       AB_User_GetCustomerId(u));
      rv=AB_Banking_EndExclUseUser(ab, u, abandon);
      if (rv<0) {
	DBG_WARN(AQHBCI_LOGDOMAIN,
		 "Could not unlock customer [%s] (%d)",
		 AB_User_GetCustomerId(u), rv);
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



static AH_OUTBOX__CBOX *AH_Outbox__FindCBox(const AH_OUTBOX *ob,
                                            const AB_USER *u) {
  AH_OUTBOX__CBOX *cbox;

  assert(ob);
  assert(u);
  cbox=AH_Outbox__CBox_List_First(ob->userBoxes);
  while(cbox) {
    if (AH_Outbox__CBox_GetUser(cbox)==u) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "CBox for customer \"%s\" found",
                AB_User_GetCustomerId(u));
      return cbox;
    }
    cbox=AH_Outbox__CBox_List_Next(cbox);
  } /* while */
  DBG_INFO(AQHBCI_LOGDOMAIN, "CBox for customer \"%s\" not found",
           AB_User_GetCustomerId(u));
  return 0;
}





void AH_Outbox_AddJob(AH_OUTBOX *ob, AH_JOB *j){
  AB_USER *u;
  AH_OUTBOX__CBOX *cbox;

  assert(ob);
  assert(j);

  u=AH_Job_GetUser(j);
  assert(u);

  cbox=AH_Outbox__FindCBox(ob, u);
  if (!cbox) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating CBox for customer \"%s\"",
               AB_User_GetCustomerId(u));
    cbox=AH_Outbox__CBox_new(ob->hbci, u, ob);
    AH_Outbox__CBox_List_Add(cbox, ob->userBoxes);
  }
  /* attach to job so that it will never be destroyed from me */
  AH_Job_Attach(j);
  AH_Outbox__CBox_AddTodoJob(cbox, j);
}



void AH_Outbox_AddPendingJob(AH_OUTBOX *ob, AB_JOB *bj){
  AB_USER *u;
  AH_OUTBOX__CBOX *cbox;
  GWEN_DB_NODE *db;
  const char *customerId;
  const char *bankId;

  assert(ob);
  assert(bj);

  db=AB_Job_GetProviderData(bj, AH_HBCI_GetProvider(ob->hbci));
  assert(db);
  customerId=GWEN_DB_GetCharValue(db, "customerId", 0, 0);
  bankId=GWEN_DB_GetCharValue(db, "bankId", 0, 0);
  if (!customerId || !bankId) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Job has never been sent by AqHBCI");
    return;
  }

  u=AB_Banking_FindUser(AH_HBCI_GetBankingApi(ob->hbci),
                        AH_PROVIDER_NAME, 0, bankId, "*", customerId);
  if (!u) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Customer %s at bank %s not found",
              customerId, bankId);
    return;
  }

  cbox=AH_Outbox__FindCBox(ob, u);
  if (!cbox) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating CBox for customer \"%s\"",
               AB_User_GetCustomerId(u));
    cbox=AH_Outbox__CBox_new(ob->hbci, u, ob);
    AH_Outbox__CBox_List_Add(cbox, ob->userBoxes);
  }
  AH_Outbox__CBox_AddPendingJob(cbox, bj);
}



int AH_Outbox_StartSending(AH_OUTBOX *ob) {
  return AH_Outbox_Prepare(ob);
}



void AH_Outbox__FinishCBox(AH_OUTBOX *ob, AH_OUTBOX__CBOX *cbox){
  AH_JOB_LIST *jl;
  AH_JOB *j;

  assert(ob);
  assert(cbox);

  AH_Outbox__CBox_Finish(cbox);
  jl=AH_Outbox__CBox_TakeFinishedJobs(cbox);
  assert(jl);
  while((j=AH_Job_List_First(jl))) {
    int rv;
    AH_JOB_STATUS st;

    AH_Job_List_Del(j);
    st=AH_Job_GetStatus(j);
    if (st==AH_JobStatusAnswered) {
      rv=AH_Job_Process(j, ob->context);
      if (rv) {
        char buf[256];

	DBG_ERROR(AQHBCI_LOGDOMAIN,
		  "Error in job \"%s\": %d",
		  AH_Job_GetName(j), rv);
	buf[0]=0;
        buf[sizeof(buf)-1]=0;
        snprintf(buf, sizeof(buf)-1,
                 I18N("Error processing job %s"),
                 AH_Job_GetName(j));
	AH_Job_SetStatus(j, AH_JobStatusError);

        GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Error,
			     buf);
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

#if 0
  /* unlock customer if necessary */
  if (cbox->isLocked) {
    if (cbox->isLocked) {
      int rv;

      GWEN_Gui_ProgressLog2(0,
			    GWEN_LoggerLevel_Info,
			    I18N("Unlocking user %s"),
			    AB_User_GetUserId(cbox->user));
      DBG_INFO(AQHBCI_LOGDOMAIN,
	       "Unlocking customer \"%s\"",
	       AB_User_GetCustomerId(cbox->user));
      rv=AB_Banking_EndExclUseUser(AH_HBCI_GetBankingApi(cbox->hbci),
				   cbox->user,
				   0);
      cbox->isLocked=0;
      if (rv<0) {
	DBG_WARN(AQHBCI_LOGDOMAIN,
		 "Could not unlock customer [%s] (%d)",
		 AB_User_GetCustomerId(cbox->user), rv);
      }
    }
  }
#endif
}



static void AH_Outbox__FinishOutbox(AH_OUTBOX *ob) {
  AH_OUTBOX__CBOX *cbox;

  assert(ob);
  while((cbox=AH_Outbox__CBox_List_First(ob->userBoxes))) {
    AH_Outbox__FinishCBox(ob, cbox);
    AH_Outbox__CBox_List_Del(cbox);
    AH_Outbox__CBox_free(cbox);
  } /* while */
}



int AH_Outbox_SendAndRecv(AH_OUTBOX *ob){
  AH_OUTBOX__CBOX *cbox;
  int rv;
  int errors;

  errors=0;
  while((cbox=AH_Outbox__CBox_List_First(ob->userBoxes))) {
    AB_USER *u;

    u=AH_Outbox__CBox_GetUser(cbox);
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Sending next message for customer \"%s\"",
             AB_User_GetCustomerId(u));

    rv=AH_Outbox__CBox_SendAndRecvBox(cbox);
    AH_Outbox__FinishCBox(ob, cbox);
    AH_Outbox__CBox_List_Del(cbox);
    AH_Outbox__CBox_free(cbox);
    if (rv)
      errors++;
    if (rv==GWEN_ERROR_USER_ABORTED) {
      AH_Outbox__FinishOutbox(ob);
      return rv;
    }
  } /* while */

  AH_Outbox__FinishOutbox(ob);
  return 0;
}



AH_JOB_LIST *AH_Outbox_GetFinishedJobs(AH_OUTBOX *ob){
  assert(ob);
  assert(ob->usage);
  AH_Outbox__FinishOutbox(ob);
  return ob->finishedJobs;
}



void AH_Outbox_Commit(AH_OUTBOX *ob, int doLock){
  AH_JOB *j;

  assert(ob);
  j=AH_Job_List_First(ob->finishedJobs);
  while(j) {
    if (AH_Job_GetStatus(j)==AH_JobStatusAnswered) {
      /* only commit answered jobs */
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Committing job \"%s\"", AH_Job_GetName(j));
      AH_Job_Commit(j, doLock);
    }
    j=AH_Job_List_Next(j);
  } /* while */
}



void AH_Outbox_CommitSystemData(AH_OUTBOX *ob, int doLock){
  AH_JOB *j;

  assert(ob);
  j=AH_Job_List_First(ob->finishedJobs);
  while(j) {
    if (AH_Job_GetStatus(j)==AH_JobStatusAnswered) {
      /* only commit answered jobs */
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Committing job \"%s\"", AH_Job_GetName(j));
      AH_Job_DefaultCommitHandler(j, doLock);
    }
    j=AH_Job_List_Next(j);
  } /* while */
}



void AH_Outbox_Process(AH_OUTBOX *ob){
  AH_JOB *j;

  assert(ob);
  j=AH_Job_List_First(ob->finishedJobs);
  while(j) {
    if (AH_Job_GetStatus(j)==AH_JobStatusAnswered) {
      int rv;

      /* only process answered jobs */
      DBG_DEBUG(AQHBCI_LOGDOMAIN,
		"Processing job \"%s\"", AH_Job_GetName(j));
      rv=AH_Job_Process(j, ob->context);
      if (rv) {
	char buf[256];

	DBG_INFO(AQHBCI_LOGDOMAIN,
		 "Error processing job \"%s\": %d",
		 AH_Job_GetName(j), rv);
	AH_Job_SetStatus(j, AH_JobStatusError);

	buf[0]=0;
	buf[sizeof(buf)-1]=0;
	snprintf(buf, sizeof(buf)-1,
		 I18N("Error processing job %s"),
		 AH_Job_GetName(j));

	GWEN_Gui_ProgressLog(
			       0,
			       GWEN_LoggerLevel_Error,
			       buf);
      }
    }
    j=AH_Job_List_Next(j);
  } /* while */
}



unsigned int AH_Outbox_CountTodoJobs(AH_OUTBOX *ob){
  unsigned int cnt;
  AH_OUTBOX__CBOX *cbox;

  assert(ob);
  cnt=0;
  cbox=AH_Outbox__CBox_List_First(ob->userBoxes);
  while(cbox) {
    AH_JOBQUEUE *jq;

    cnt+=AH_Job_List_GetCount(cbox->todoJobs);
    jq=AH_JobQueue_List_First(cbox->todoQueues);
    while(jq) {
      if (!(AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_OUTBOX)) {
        const AH_JOB_LIST *jl;

	jl=AH_JobQueue_GetJobList(jq);
	if (jl) {
	  AH_JOB *j;

	  j=AH_Job_List_First(jl);
	  while(j) {
	    if (!(AH_Job_GetFlags(j) & AH_JOB_FLAGS_OUTBOX))
	      cnt++;

	    j=AH_Job_List_Next(j);
	  } /* while */
	}
      }
      jq=AH_JobQueue_List_Next(jq);
    } /* while */
    cbox=AH_Outbox__CBox_List_Next(cbox);
  } /* while */

  return cnt;
}



unsigned int AH_Outbox__CountJobList(const AH_JOB_LIST *jl) {
  AH_JOB *j;
  unsigned int cnt;

  assert(jl);
  cnt=0;
  j=AH_Job_List_First(jl);
  while(j) {
    if (!(AH_Job_GetFlags(j) & AH_JOB_FLAGS_OUTBOX))
      cnt++;
    j=AH_Job_List_Next(j);
  } /* while */

  return cnt;
}



unsigned int AH_Outbox_CountFinishedJobs(AH_OUTBOX *ob){
  unsigned int cnt;
  AH_OUTBOX__CBOX *cbox;

  assert(ob);
  cnt=0;

  cnt+=AH_Outbox__CountJobList(ob->finishedJobs);

  cbox=AH_Outbox__CBox_List_First(ob->userBoxes);
  while(cbox) {
    AH_JOBQUEUE *jq;

    /* count jobs in queues */
    jq=AH_JobQueue_List_First(cbox->finishedQueues);
    while(jq) {
      if (!(AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_OUTBOX)) {
	const AH_JOB_LIST *jl;

	jl=AH_JobQueue_GetJobList(jq);
	if (jl) {
	  AH_JOB *j;

	  j=AH_Job_List_First(jl);
	  while(j) {
	    if (!(AH_Job_GetFlags(j) & AH_JOB_FLAGS_OUTBOX))
	      cnt++;

	    j=AH_Job_List_Next(j);
	  } /* while */
	}
      }
      jq=AH_JobQueue_List_Next(jq);
    } /* while */

    /* count other finished jobs */
    cnt+=AH_Outbox__CountJobList(cbox->finishedJobs);

    cbox=AH_Outbox__CBox_List_Next(cbox);
  } /* while */

  return cnt;
}



int AH_Outbox__Execute(AH_OUTBOX *ob){
  unsigned int jobCount;
  int rv;

  assert(ob);
  jobCount=AH_Outbox_CountTodoJobs(ob);
  if (jobCount==0) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Empty outbox");
    return 0;
  }

  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Notice,
		       I18N("AqHBCI started"));

  rv=AH_Outbox_StartSending(ob);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not start sending outbox.");
    return rv;
  }

  rv=AH_Outbox_SendAndRecv(ob);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error while sending outbox.");
    return rv;
  }

  rv=AB_Banking_ExecutionProgress(AH_HBCI_GetBankingApi(ob->hbci));
  if (rv==GWEN_ERROR_USER_ABORTED) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User aborted (%d)", rv);
    AH_Outbox__FinishOutbox(ob);
    return rv;
  }

  GWEN_Gui_ProgressLog(0,
		       GWEN_LoggerLevel_Notice,
		       I18N("AqHBCI finished."));
  return 0;
}



int AH_Outbox_Execute(AH_OUTBOX *ob,
                      AB_IMEXPORTER_CONTEXT *ctx,
		      int withProgress, int nounmount, int doLock) {
  int rv;
  uint32_t pid=0;
  AB_USER_LIST2 *lockedUsers = NULL;

  assert(ob);

  if (withProgress) {
    pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
			       GWEN_GUI_PROGRESS_ALLOW_EMBED |
			       GWEN_GUI_PROGRESS_SHOW_PROGRESS |
			       GWEN_GUI_PROGRESS_SHOW_ABORT,
			       I18N("Executing Jobs"),
			       I18N("Now the jobs are send via their "
				    "backends to the credit institutes."),
			       AH_Outbox_CountTodoJobs(ob),
			       0);
  }

  ob->context=ctx;

  if (doLock) {
    GWEN_Gui_ProgressLog(pid,
			 GWEN_LoggerLevel_Info,
			 I18N("Locking users"));
    lockedUsers=AB_User_List2_new();
    rv=AH_Outbox_LockUsers(ob, lockedUsers);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_Gui_ProgressLog(pid,
			   GWEN_LoggerLevel_Error,
			   I18N("Unable to lock users"));
      AB_User_List2_free(lockedUsers);
    }
  }
  else
    rv=0;

  if (rv==0) {
    GWEN_Gui_ProgressLog(pid,
			 GWEN_LoggerLevel_Info,
			 I18N("Executing HBCI jobs"));
    rv=AH_Outbox__Execute(ob);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    }
    if (doLock) {
      int rv2;

      rv2=AH_Outbox_UnlockUsers(ob, lockedUsers, 0);
      if (rv2<0) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	GWEN_Gui_ProgressLog(pid,
			     GWEN_LoggerLevel_Error,
			     I18N("Unable to unlock users"));
      }
      AB_User_List2_free(lockedUsers);
      if (rv==0 && rv2!=0)
	rv=rv2;
    }
  }

  /* unmount currently mounted medium */
  if (!nounmount)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(ob->hbci));
  if (withProgress) {
    GWEN_Gui_ProgressEnd(pid);
  }

  ob->context=0;
  return rv;
}



AH_JOB *AH_Outbox__FindTransferJobInCheckJobList(const AH_JOB_LIST *jl,
                                                 AB_USER *u,
                                                 AB_ACCOUNT *a,
                                                 const char *jobName) {
  AH_JOB *j;

  assert(jl);
  j=AH_Job_List_First(jl);
  while(j) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Checking job \"%s\"",
              AH_Job_GetName(j));
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



AH_JOB *AH_Outbox_FindTransferJob(AH_OUTBOX *ob,
                                  AB_USER *u,
                                  AB_ACCOUNT *a,
                                  const char *jobName) {
  AH_OUTBOX__CBOX *cbox;
  AH_JOB *j;

  assert(ob);
  assert(u);
  assert(a);
  assert(jobName);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Searching for %s job", jobName);
  cbox=AH_Outbox__CBox_List_First(ob->userBoxes);
  while(cbox) {
    if (cbox->user==u) {
      AH_JOBQUEUE *jq;

      /* check jobs in lists */
      j=AH_Outbox__FindTransferJobInCheckJobList(cbox->todoJobs, u, a, jobName);
      if (j)
        return j;

      /* check jobs in queues */
      jq=AH_JobQueue_List_First(cbox->todoQueues);
      while(jq) {
        const AH_JOB_LIST *jl;

        jl=AH_JobQueue_GetJobList(jq);
        if (jl) {
          j=AH_Outbox__FindTransferJobInCheckJobList(jl, u, a, jobName);
          if (j)
            return j;
        }
        jq=AH_JobQueue_List_Next(jq);
      } /* while */
    }
    else {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Customer doesn't match");
    }

    cbox=AH_Outbox__CBox_List_Next(cbox);
  } /* while */

  DBG_INFO(AQHBCI_LOGDOMAIN, "No matching multi job found");
  return 0;
}



#include "itan.inc"
#include "itan1.inc"
#include "itan2.inc"




