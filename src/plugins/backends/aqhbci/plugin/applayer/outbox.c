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


#include "outbox_p.h"
#include "aqhbci_l.h"
#include "job_l.h"
#include "accountjob_l.h"
#include "jobqueue_l.h"
#include "hbci_l.h"
#include "adminjobs.h"
#include "jobmultitransfer_l.h"
#include <aqbanking/job_be.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/net.h>
#include <gwenhywfar/waitcallback.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>


GWEN_LIST_FUNCTIONS(AH_OUTBOX__CBOX, AH_Outbox__CBox);




AH_OUTBOX__CBOX *AH_Outbox__CBox_new(AH_HBCI *hbci, AH_CUSTOMER *cu,
				     AH_OUTBOX *ob) {
  AH_OUTBOX__CBOX *cbox;

  assert(hbci);
  assert(cu);
  GWEN_NEW_OBJECT(AH_OUTBOX__CBOX, cbox);
  cbox->usage=1;
  GWEN_LIST_INIT(AH_OUTBOX__CBOX, cbox);
  cbox->customer=cu;
  AH_Customer_Attach(cu);
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
      AH_Customer_free(cbox->customer);

      GWEN_FREE_OBJECT(cbox);
    }
  }
}



void AH_Outbox__CBox_Attach(AH_OUTBOX__CBOX *cbox){
  assert(cbox);
  cbox->usage++;
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



AB_JOB_LIST2 *AH_Outbox__CBox_GetPendingJobs(const AH_OUTBOX__CBOX *cbox){
  assert(cbox);
  return cbox->pendingJobs;
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
            GWEN_TYPE_UINT32 rMsgNum;
            GWEN_TYPE_UINT32 rSegNum;
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
                    GWEN_TYPE_UINT32 jMsgNum;
                    GWEN_TYPE_UINT32 jFirstSeg;
                    GWEN_TYPE_UINT32 jLastSeg;
  
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
    AH_JobQueue_free(jq);
  } /* while */

  /* check for results for pending jobs */
  AH_Outbox__CBox_CheckPending(cbox);
}



AH_CUSTOMER*
AH_Outbox__CBox_GetCustomer(const AH_OUTBOX__CBOX *cbox){
  assert(cbox);
  return cbox->customer;
}



AH_JOB_LIST*
AH_Outbox__CBox_TakeFinishedJobs(AH_OUTBOX__CBOX *cbox){
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

  /* add JobGetStatus if there are any pending jobs for this customer */
  if (AB_Job_List2_GetSize(cbox->pendingJobs)) {
    AH_JOB *sj;
    GWEN_TIME *t1;
    GWEN_TIME *t2;

    /* have some pending jobs, add status call */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Will ask for status reports");
    t1=AH_Outbox__CBox_GetEarliestPendingDate(cbox);
    t2=AH_Outbox__CBox_GetLatestPendingDate(cbox);
    sj=AH_Job_GetStatus_new(cbox->customer, t1, t2);
    GWEN_Time_free(t2);
    GWEN_Time_free(t1);
    if (sj) {
      AH_USER *u;

      AH_Job_AddFlags(sj, AH_JOB_FLAGS_OUTBOX);
      u=AH_Customer_GetUser(cbox->customer);
      assert(u);
      AH_Job_AddSigner(sj, AH_User_GetUserId(u));
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
        jq=AH_JobQueue_new(cbox->customer);
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
  jq=AH_JobQueue_new(cbox->customer);
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
          errors++;
        } /* if first job failed */
        else {
          /* not the first job, check for reason of failure */
          if (res==AH_JobQueueAddResultQueueFull) {
            /* queue is full, so add it to the todo queue list and start
             * a new queue */
            DBG_INFO(AQHBCI_LOGDOMAIN, "Queue full, starting next one");
            AH_JobQueue_List_Add(jq, cbox->todoQueues);
            jq=AH_JobQueue_new(cbox->customer);
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
      AH_Job_List_free(retryJobs);
      retryJobs=0;

      /* there are some retry jobs, retry them */
      if (AH_JobQueue_GetCount(jq)!=0) {
	AH_JobQueue_List_Add(jq, cbox->todoQueues);
	jq=AH_JobQueue_new(cbox->customer);
	firstJob=1;
	queueCreated=1;
      }
    }

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



int AH_Outbox__CBox_SendQueue(AH_OUTBOX__CBOX *cbox, int timeout,
			      AH_DIALOG *dlg,
			      AH_JOBQUEUE *jq){
  AH_MSG *msg;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding queue");
  AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			 0,
			 AB_Banking_LogLevelInfo,
                         I18N("Encoding queue"));
  msg=AH_JobQueue_ToMessage(jq, dlg);
  if (!msg) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not encode queue");
    AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			   0,
			   AB_Banking_LogLevelError,
                           I18N("Unable to encode"));
    return AB_ERROR_GENERIC;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Sending queue");
  AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			 0,
			 AB_Banking_LogLevelInfo,
                         I18N("Sending queue"));
  if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE)
    rv=AH_Dialog_SendMessage(dlg, msg);
  else
    rv=AH_Dialog_SendMessage_Wait(dlg, msg, timeout);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not send message");
    AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			   0,
                           AB_Banking_LogLevelError,
                           I18N("Unable to send (network error)"));
    return rv;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Message sent");
  return 0;
}



int AH_Outbox__CBox_RecvQueue(AH_OUTBOX__CBOX *cbox,
			      int timeout,
			      AH_DIALOG *dlg,
			      AH_JOBQUEUE *jq){
  AH_MSG *msg;
  time_t startt;
  int distance;
  AH_USER *u;
  AH_BANK *b;
  GWEN_DB_NODE *rsp;
  int rv;

  assert(cbox);
  u=AH_Customer_GetUser(cbox->customer);
  assert(u);
  b=AH_User_GetBank(u);
  assert(b);

  startt=time(0);
  if (timeout==GWEN_NETCONNECTION_TIMEOUT_NONE)
    distance=GWEN_NETCONNECTION_TIMEOUT_NONE;
  else if (timeout==GWEN_NETCONNECTION_TIMEOUT_FOREVER)
    distance=GWEN_NETCONNECTION_TIMEOUT_FOREVER;
  else {
    distance=AH_OUTBOX_TIME_DISTANCE;
    if (distance)
      if ((distance/1000)>timeout)
        distance=timeout*1000;
    if (!distance)
      distance=750;
  }

  AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			 0,
			 AB_Banking_LogLevelInfo,
                         I18N("Waiting for response"));


  msg=AH_Dialog_RecvMessage_Wait(dlg, timeout);
  if (!msg) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "No message within specified timeout, giving up");
    AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
                           0,
                           AB_Banking_LogLevelError,
                           I18N("No response (timeout)"));
    return AB_ERROR_NETWORK;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Got a message");
  AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			 0,
			 AB_Banking_LogLevelInfo,
                         I18N("Response received"));

  /* try to dispatch the message */
  rsp=GWEN_DB_Group_new("response");
  if (AH_Msg_DecodeMsg(msg, rsp, GWEN_MSGENGINE_READ_FLAGS_DEFAULT)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not decode this message:");
    AH_Msg_Dump(msg, stderr, 2);
    GWEN_DB_Group_free(rsp);
    AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			   0,
			   AB_Banking_LogLevelError,
                           I18N("Bad response (unable to decode)"));
    return AB_ERROR_GENERIC;
  }

  /* check for message reference */
  if (AH_Msg_GetMsgRef(msg)==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unrequested message, deleting it");
    GWEN_DB_Group_free(rsp);
    AH_Msg_free(msg);
    AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			   0,
			   AB_Banking_LogLevelError,
                           I18N("Bad response (bad message reference)"));
    return AB_ERROR_GENERIC;
  }

  rv=AH_JobQueue_DispatchMessage(jq, msg, rsp);
  if (rv) {
    if (rv==AB_ERROR_ABORTED) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Dialog aborted by server");
      AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
                             0,
                             AB_Banking_LogLevelError,
                             I18N("Dialog aborted by server"));
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not dispatch response");
      AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
                             0,
                             AB_Banking_LogLevelError,
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
				     int timeout,
				     AH_DIALOG *dlg,
				     AH_JOBQUEUE *jq){
  int rv;

  rv=AH_Outbox__CBox_SendQueue(cbox, timeout, dlg, jq);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error sending queue");
    return rv;
  }

  AH_JobQueue_SetJobStatusOnMatch(jq, AH_JobStatusEncoded,
				  AH_JobStatusSent);

  rv=AH_Outbox__CBox_RecvQueue(cbox, timeout, dlg, jq);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error receiving queue response");
    return rv;
  }


  return 0;
}




int AH_Outbox__CBox_OpenDialog(AH_OUTBOX__CBOX *cbox, int timeout,
			       AH_DIALOG *dlg,
			       GWEN_TYPE_UINT32 jqFlags) {
  AH_JOBQUEUE *jqDlgOpen;
  AH_JOB *jDlgOpen;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating dialog open request");
  if ((jqFlags & AH_JOBQUEUE_FLAGS_CRYPT) ||
      (jqFlags & AH_JOBQUEUE_FLAGS_SIGN)) {
    /* sign and crypt, not anonymous */
    DBG_NOTICE(AQHBCI_LOGDOMAIN,
	       "Creating non-anonymous dialog open request");
    jDlgOpen=AH_Job_new("JobDialogInit", cbox->customer, 0);
    if (!jDlgOpen) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create job JobDialogInit");
      return AB_ERROR_GENERIC;
    }
    if (jqFlags & AH_JOBQUEUE_FLAGS_SIGN)
      AH_Job_AddSigner(jDlgOpen,
		       AH_User_GetUserId(AH_Customer_GetUser(cbox->customer)));
    /* was before: AH_Customer_GetCustomerId(cbox->customer)); */
    AH_Dialog_SubFlags(dlg, AH_DIALOG_FLAGS_ANONYMOUS);
  }
  else {
    /* neither sign nor crypt, use anonymous dialog */
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating anonymous dialog open request");
    jDlgOpen=AH_Job_new("JobDialogInitAnon", cbox->customer, 0);
    if (!jDlgOpen) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create job JobDialogInitAnon");
      return AB_ERROR_GENERIC;
    }
    AH_Dialog_AddFlags(dlg, AH_DIALOG_FLAGS_ANONYMOUS);
  }

  AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
                         0,
                         AB_Banking_LogLevelNotice,
			 I18N("Opening dialog"));
  jqDlgOpen=AH_JobQueue_new(cbox->customer);
  AH_JobQueue_AddFlags(jqDlgOpen, AH_JOBQUEUE_FLAGS_OUTBOX);
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Enqueueing dialog open request");
  if (AH_JobQueue_AddJob(jqDlgOpen, jDlgOpen)!=
      AH_JobQueueAddResultOk) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add single job to queue");
    AH_Job_free(jDlgOpen);
    AH_JobQueue_free(jqDlgOpen);
    return AB_ERROR_GENERIC;
  }

  rv=AH_Outbox__CBox_SendAndRecvQueue(cbox, timeout, dlg, jqDlgOpen);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not exchange message");
    AH_JobQueue_free(jqDlgOpen);
    return rv;
  }
  if (AH_Job_HasErrors(jDlgOpen)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error opening dialog, aborting");
    AH_JobQueue_free(jqDlgOpen);
    return AB_ERROR_GENERIC;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Dialog open request done");
  rv=AH_Job_CommitSystemData(jDlgOpen);
  AH_JobQueue_free(jqDlgOpen);
  return rv;
}



int AH_Outbox__CBox_CloseDialog(AH_OUTBOX__CBOX *cbox,
				int timeout,
				AH_DIALOG *dlg,
				GWEN_TYPE_UINT32 jqFlags) {
  AH_JOBQUEUE *jqDlgClose;
  AH_JOB *jDlgClose;
  GWEN_DB_NODE *db;
  GWEN_TYPE_UINT32 dlgFlags;
  AH_USER *u;
  AH_BANK *b;
  int rv;

  u=AH_Customer_GetUser(cbox->customer);
  assert(u);
  b=AH_User_GetBank(u);
  assert(b);

  AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			 0,
			 AB_Banking_LogLevelNotice,
                         I18N("Closing dialog"));
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Sending dialog close request (flags=%08x)",
	     jqFlags);
  dlgFlags=AH_Dialog_GetFlags(dlg);
  jDlgClose=AH_Job_new("JobDialogEnd", cbox->customer, 0);
  if (!jDlgClose) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create job JobDialogEnd");
    return AB_ERROR_GENERIC;
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
    AH_Job_AddSigner(jDlgClose, AH_User_GetUserId(u));
    AH_Job_AddFlags(jDlgClose,
		    AH_JOB_FLAGS_CRYPT |
		    AH_JOB_FLAGS_SIGN);
  }
  jqDlgClose=AH_JobQueue_new(cbox->customer);

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Adding dialog close request to queue");
  if (AH_JobQueue_AddJob(jqDlgClose, jDlgClose)!=
      AH_JobQueueAddResultOk) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not add single job to queue");
    AH_JobQueue_free(jqDlgClose);
    return AB_ERROR_GENERIC;
  }

  rv=AH_Outbox__CBox_SendAndRecvQueue(cbox, timeout, dlg, jqDlgClose);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not exchange message");
    AH_JobQueue_free(jqDlgClose);
    return rv;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Dialog closed");
  rv=AH_Job_CommitSystemData(jDlgClose);
  AH_JobQueue_free(jqDlgClose);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not commit system data");
    return rv;
  }
  return 0;
}



void AH_Outbox__CBox_HandleQueueError(AH_OUTBOX__CBOX *cbox,
				      AH_JOBQUEUE *jq) {
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
    }
    AH_Job_List_Add(j, cbox->finishedJobs);
  }
  AH_JobQueue_free(jq);
}



int AH_Outbox__CBox_PerformQueue(AH_OUTBOX__CBOX *cbox,
				 AH_DIALOG *dlg,
				 AH_JOBQUEUE *jq,
				 int timeout) {
  int jobsTodo;
  int rv;

  jobsTodo=0;
  for (;;) {
    AH_JOBQUEUE *jqTodo;
    GWEN_TYPE_UINT32 jqFlags;
    AH_JOB *j;
    AH_JOB_LIST *jl;
    int finishedJobs;
    int finishedJobsBefore;
    int i;

    finishedJobsBefore=AH_Outbox_CountFinishedJobs(cbox->outbox);

    jobsTodo=0;
    jl=AH_JobQueue_TakeJobList(jq);
    assert(jl);
    jqTodo=AH_JobQueue_new(AH_JobQueue_GetCustomer(jq));
    /* copy some flags */
    jqFlags=AH_JobQueue_GetFlags(jq);
    jqFlags&=~(AH_JOBQUEUE_FLAGS_CRYPT |
               AH_JOBQUEUE_FLAGS_SIGN |
               AH_JOBQUEUE_FLAGS_NOSYSID);
    AH_JobQueue_SetFlags(jqTodo, (jqFlags&AH_JOBQUEUE_FLAGS_COPYMASK));

    /* copy todo jobs */
    while((j=AH_Job_List_First(jl))) {
      AH_Job_List_Del(j);
      /* prepare jobs for next message
       * (if attachpoint or multi-message job)
       */
      if (AH_Job_GetStatus(j)==AH_JobStatusAnswered) {
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
	    AH_Job_SetStatus(j, AH_JobStatusError);
	  }
	  else {
	    jobsTodo++;
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
	}
	else {
	  jobsTodo++;
	  j=0; /* mark that this job has been dealt with */
	}
      }
      else if (AH_Job_GetStatus(j)==AH_JobStatusAnswered) {
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message finished");
      }
      else {
	DBG_DEBUG(AQHBCI_LOGDOMAIN, "Bad status \"%s\" (%d)",
		  AH_Job_StatusName(AH_Job_GetStatus(j)),
		  AH_Job_GetStatus(j));
	if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevelDebug)
	  AH_Job_Dump(j, stderr, 4);
      }
      if (j) {
	/* move job to finished list if we still have the job */
	AH_Job_List_Add(j, cbox->finishedJobs);
      }
    } /* while */

    AH_JobQueue_free(jq);
    jq=jqTodo;

    finishedJobs=AH_Outbox_CountFinishedJobs(cbox->outbox)-finishedJobsBefore;
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Finished jobs: %d", finishedJobs);
    for (i=0; i<finishedJobs; i++) {
      int rv;

      rv=AB_Banking_ProgressAdvance(AH_HBCI_GetBankingApi(cbox->hbci), 0,
                                    AB_BANKING_PROGRESS_ONE);
      if (rv) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "User aborted");
        return rv;
      }
    }

    if (!jobsTodo)
      break;

    /* jq now contains all jobs to be executed */
    rv=AH_Outbox__CBox_SendAndRecvQueue(cbox, timeout, dlg, jq);
    if (rv) {
      AH_Outbox__CBox_HandleQueueError(cbox, jq);
      return rv;
    } /* if error */
  } /* for */

  AH_JobQueue_free(jq);
  return 0;
}



int AH_Outbox__CBox_PerformNonDialogQueues(AH_OUTBOX__CBOX *cbox,
					   int timeout,
					   AH_JOBQUEUE_LIST *jql){
  AH_DIALOG *dlg;
  AH_JOBQUEUE *jq;
  AH_USER *u;
  AH_BANK *b;
  int rv;
  GWEN_TYPE_UINT32 jqflags;

  u=AH_Customer_GetUser(cbox->customer);
  assert(u);
  b=AH_User_GetBank(u);
  assert(b);

  if (AH_JobQueue_List_GetCount(jql)==0) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No queues to handle, doing nothing");
    AH_JobQueue_List_free(jql);
    return 0;
  }

  rv=AH_HBCI_BeginDialog(cbox->hbci, cbox->customer, &dlg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not begin a dialog for customer \"%s\" (%d)",
             AH_Customer_GetCustomerId(cbox->customer), rv);
    /* finish all queues */
    AH_Outbox__CBox_HandleQueueListError(cbox, jql);
    return rv;
  }
  assert(dlg);

  jq=AH_JobQueue_List_First(jql);
  jqflags=AH_JobQueue_GetFlags(jq);

  /* open dialog */
  rv=AH_Outbox__CBox_OpenDialog(cbox, timeout, dlg, jqflags);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not open dialog");
    AH_HBCI_EndDialog(cbox->hbci, dlg);
    /* finish all queues */
    AH_Outbox__CBox_HandleQueueListError(cbox, jql);
    return rv;
  }

  /* handle queues */
  rv=0;
  while((jq=AH_JobQueue_List_First(jql))) {
    AH_JobQueue_List_Del(jq);
    rv=AH_Outbox__CBox_PerformQueue(cbox, dlg, jq, timeout);
    if (rv)
      break;
  } /* while */

  if (rv) {
    /* finish all remaining queues */
    AH_Outbox__CBox_HandleQueueListError(cbox, jql);
    AH_HBCI_EndDialog(cbox->hbci, dlg);
    return rv;
  }

  /* close dialog */
  rv=AH_Outbox__CBox_CloseDialog(cbox, timeout, dlg, jqflags);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not close dialog, ignoring");
    /*AH_HBCI_EndDialog(cbox->hbci, dlg);
     return rv;*/
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Closing connection");
  AH_HBCI_EndDialog(cbox->hbci, dlg);

  AH_JobQueue_List_free(jql);
  return 0;
}



int AH_Outbox__CBox_PerformDialogQueue(AH_OUTBOX__CBOX *cbox,
				       int timeout,
				       AH_JOBQUEUE *jq){
  AH_DIALOG *dlg;
  AH_USER *u;
  AH_BANK *b;
  int rv;

  u=AH_Customer_GetUser(cbox->customer);
  assert(u);
  b=AH_User_GetBank(u);
  assert(b);

  /* open connection */
  rv=AH_HBCI_BeginDialog(cbox->hbci, cbox->customer, &dlg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Could not begin a dialog for customer \"%s\" (%d)",
             AH_Customer_GetCustomerId(cbox->customer), rv);
    /* finish all queues */
    AH_Outbox__CBox_HandleQueueError(cbox, jq);
    return rv;
  }
  assert(dlg);

  /* handle queue */
  rv=AH_Outbox__CBox_PerformQueue(cbox, dlg, jq, timeout);
  if (rv) {
    AH_HBCI_EndDialog(cbox->hbci, dlg);
    return rv;
  }

  /* close connection */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Closing connection");
  AH_HBCI_EndDialog(cbox->hbci, dlg);

  return 0;
}



void AH_Outbox__CBox_ExtractMatchingQueues(AH_JOBQUEUE_LIST *jql,
					   AH_JOBQUEUE_LIST *jqlWanted,
					   AH_JOBQUEUE_LIST *jqlRest,
					   GWEN_TYPE_UINT32 jqflags,
					   GWEN_TYPE_UINT32 jqmask) {
  AH_JOBQUEUE *jq;

  while((jq=AH_JobQueue_List_First(jql))) {
    GWEN_TYPE_UINT32 flags;

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
					  AH_JOBQUEUE_LIST *jql){
  AH_JOBQUEUE *jq;

  while((jq=AH_JobQueue_List_First(jql))) {
    AH_JobQueue_List_Del(jq);
    AH_Outbox__CBox_HandleQueueError(cbox, jq);
  } /* while */
  AH_JobQueue_List_free(jql);
}



int AH_Outbox__CBox_SendAndRecvDialogQueues(AH_OUTBOX__CBOX *cbox,
					    int timeout) {
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
      rv=AH_Outbox__CBox_PerformDialogQueue(cbox, timeout, jq);
      if (rv) {
	DBG_ERROR(AQHBCI_LOGDOMAIN,
		  "Error performing queue (%d)", rv);
	AH_Outbox__CBox_HandleQueueListError(cbox, jqlWanted);
        AH_Outbox__CBox_HandleQueueListError(cbox, cbox->todoQueues);
        cbox->todoQueues=AH_JobQueue_List_new();
	return rv;
      }
    } /* while */
  }
  AH_JobQueue_List_free(jqlWanted);
  return 0;
}



int AH_Outbox__CBox_SendAndRecvSelected(AH_OUTBOX__CBOX *cbox,
					int timeout,
					GWEN_TYPE_UINT32 jqflags,
					GWEN_TYPE_UINT32 jqmask){
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
    rv=AH_Outbox__CBox_PerformNonDialogQueues(cbox, timeout, jqlWanted);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"Error performing queue (%d)", rv);
      AH_Outbox__CBox_HandleQueueListError(cbox, cbox->todoQueues);
      cbox->todoQueues=AH_JobQueue_List_new();
      return rv;
    } /* while */
  }
  else
    AH_JobQueue_List_free(jqlWanted);
  return 0;
}



int AH_Outbox__CBox_SendAndRecvBox(AH_OUTBOX__CBOX *cbox, int timeout){
  int rv;

  /* dialog queues */
  rv=AH_Outbox__CBox_SendAndRecvDialogQueues(cbox, timeout);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing dialog queues (%d)", rv);
    return rv;
  }

  /* non-dialog queues: unsigned, uncrypted */
  rv=AH_Outbox__CBox_SendAndRecvSelected(cbox, timeout,
					 0,
					 AH_JOBQUEUE_FLAGS_ISDIALOG |
					 AH_JOBQUEUE_FLAGS_SIGN |
					 AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (-S, -C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: unsigned, crypted */
  rv=AH_Outbox__CBox_SendAndRecvSelected(cbox, timeout,
					 AH_JOBQUEUE_FLAGS_CRYPT,
					 AH_JOBQUEUE_FLAGS_ISDIALOG |
					 AH_JOBQUEUE_FLAGS_SIGN |
					 AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (-S, +C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: signed, uncrypted */
  rv=AH_Outbox__CBox_SendAndRecvSelected(cbox, timeout,
					 AH_JOBQUEUE_FLAGS_SIGN,
					 AH_JOBQUEUE_FLAGS_ISDIALOG |
					 AH_JOBQUEUE_FLAGS_SIGN |
					 AH_JOBQUEUE_FLAGS_CRYPT);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error performing queues (+S, -C: %d)", rv);
    return rv;
  }

  /* non-dialog queues: signed, crypted */
  rv=AH_Outbox__CBox_SendAndRecvSelected(cbox, timeout,
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
  ob->customerBoxes=AH_Outbox__CBox_List_new();
  ob->finishedJobs=AH_Job_List_new();
  ob->usage=1;
  return ob;
}



void AH_Outbox_free(AH_OUTBOX *ob){
  if (ob) {
    assert(ob->usage);
    if (--(ob->usage)==0) {
      AH_Outbox__CBox_List_free(ob->customerBoxes);
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
  cbox=AH_Outbox__CBox_List_First(ob->customerBoxes);
  while(cbox) {
    AH_CUSTOMER *cu;

    cu=AH_Outbox__CBox_GetCustomer(cbox);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing queues for customer \"%s\"",
             AH_Customer_GetCustomerId(cu));
    if (AH_Outbox__CBox_Prepare(cbox)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error preparing cbox");
      errors++;
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Preparing queues for customer \"%s\": done",
               AH_Customer_GetCustomerId(cu));
    }
    cbox=AH_Outbox__CBox_List_Next(cbox);
  } /* while */

  if (errors) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "%d errors occurred", errors);
    return AB_ERROR_GENERIC;
  }

  return 0;
}



AH_OUTBOX__CBOX *AH_Outbox__FindCBox(const AH_OUTBOX *ob,
				     const AH_CUSTOMER *cu) {
  AH_OUTBOX__CBOX *cbox;

  assert(ob);
  assert(cu);
  cbox=AH_Outbox__CBox_List_First(ob->customerBoxes);
  while(cbox) {
    if (AH_Outbox__CBox_GetCustomer(cbox)==cu) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "CBox for customer \"%s\" found",
                AH_Customer_GetCustomerId(cu));
      return cbox;
    }
    cbox=AH_Outbox__CBox_List_Next(cbox);
  } /* while */
  DBG_INFO(AQHBCI_LOGDOMAIN, "CBox for customer \"%s\" not found",
           AH_Customer_GetCustomerId(cu));
  return 0;
}





void AH_Outbox_AddJob(AH_OUTBOX *ob, AH_JOB *j){
  AH_CUSTOMER *cu;
  AH_OUTBOX__CBOX *cbox;

  assert(ob);
  assert(j);

  cu=AH_Job_GetCustomer(j);
  assert(cu);

  cbox=AH_Outbox__FindCBox(ob, cu);
  if (!cbox) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating CBox for customer \"%s\"",
               AH_Customer_GetCustomerId(cu));
    cbox=AH_Outbox__CBox_new(ob->hbci, cu, ob);
    AH_Outbox__CBox_List_Add(cbox, ob->customerBoxes);
  }
  /* attach to job so that it will never be destroyed from me */
  AH_Job_Attach(j);
  AH_Outbox__CBox_AddTodoJob(cbox, j);
}



void AH_Outbox_AddPendingJob(AH_OUTBOX *ob, AB_JOB *bj){
  AH_CUSTOMER *cu;
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

  cu=AH_HBCI_FindCustomer(ob->hbci, 0, bankId, "*", customerId);
  if (!cu) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Customer %s at bank %s not found",
              customerId, bankId);
    return;
  }

  cbox=AH_Outbox__FindCBox(ob, cu);
  if (!cbox) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating CBox for customer \"%s\"",
               AH_Customer_GetCustomerId(cu));
    cbox=AH_Outbox__CBox_new(ob->hbci, cu, ob);
    AH_Outbox__CBox_List_Add(cbox, ob->customerBoxes);
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
      rv=AH_Job_Process(j);
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

        AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
                               0,
                               AB_Banking_LogLevelError,
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
}



void AH_Outbox__FinishOutbox(AH_OUTBOX *ob) {
  AH_OUTBOX__CBOX *cbox;

  assert(ob);
  while((cbox=AH_Outbox__CBox_List_First(ob->customerBoxes))) {
    AH_Outbox__FinishCBox(ob, cbox);
    AH_Outbox__CBox_List_Del(cbox);
    AH_Outbox__CBox_free(cbox);
  } /* while */
}



int AH_Outbox_SendAndRecv(AH_OUTBOX *ob, int timeout){
  AH_OUTBOX__CBOX *cbox;
  int rv;
  int errors;

  errors=0;
  while((cbox=AH_Outbox__CBox_List_First(ob->customerBoxes))) {
    AH_CUSTOMER *cu;

    cu=AH_Outbox__CBox_GetCustomer(cbox);
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Sending next message for customer \"%s\"",
	     AH_Customer_GetCustomerId(cu));

    rv=AH_Outbox__CBox_SendAndRecvBox(cbox, timeout);
    AH_Outbox__FinishCBox(ob, cbox);
    AH_Outbox__CBox_List_Del(cbox);
    AH_Outbox__CBox_free(cbox);
    if (rv)
      errors++;
    if (rv==AB_ERROR_USER_ABORT) {
      AH_Outbox__FinishOutbox(ob);
      return rv;
    }
  } /* while */

  AH_Outbox__FinishOutbox(ob);
  return 0;
}



AH_JOB_LIST *AH_Outbox_TakeFinishedJobs(AH_OUTBOX *ob){
  AH_JOB_LIST *jl;

  assert(ob);
  AH_Outbox__FinishOutbox(ob);
  jl=ob->finishedJobs;
  ob->finishedJobs=AH_Job_List_new();
  return jl;
}



AH_JOB_LIST *AH_Outbox_GetFinishedJobs(AH_OUTBOX *ob){
  assert(ob);
  AH_Outbox__FinishOutbox(ob);
  return ob->finishedJobs;
}



void AH_Outbox_Commit(AH_OUTBOX *ob){
  AH_JOB *j;

  assert(ob);
  j=AH_Job_List_First(ob->finishedJobs);
  while(j) {
    if (AH_Job_GetStatus(j)==AH_JobStatusAnswered) {
      /* only commit answered jobs */
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Committing job \"%s\"", AH_Job_GetName(j));
      AH_Job_Commit(j);
    }
    j=AH_Job_List_Next(j);
  } /* while */
}



void AH_Outbox_CommitSystemData(AH_OUTBOX *ob){
  AH_JOB *j;

  assert(ob);
  j=AH_Job_List_First(ob->finishedJobs);
  while(j) {
    if (AH_Job_GetStatus(j)==AH_JobStatusAnswered) {
      /* only commit answered jobs */
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Committing job \"%s\"", AH_Job_GetName(j));
      AH_Job_DefaultCommitHandler(j);
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
      rv=AH_Job_Process(j);
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
	AH_Job_SetStatus(j, AH_JobStatusError);

	AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(ob->hbci),
			       0,
			       AB_Banking_LogLevelError,
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
  cbox=AH_Outbox__CBox_List_First(ob->customerBoxes);
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

  cbox=AH_Outbox__CBox_List_First(ob->customerBoxes);
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
  unsigned int loop;
  unsigned int jobCount;
  unsigned int finishedJobs;
  unsigned int finishedJobsBefore;
  int rv;
  int i;

  assert(ob);
  jobCount=AH_Outbox_CountTodoJobs(ob);
  if (jobCount==0) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Empty outbox");
    return 0;
  }

  finishedJobsBefore=AH_Outbox_CountFinishedJobs(ob);

  AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(ob->hbci),
                         0,
                         AB_Banking_LogLevelNotice,
                         I18N("AqHBCI started"));

  rv=AH_Outbox_StartSending(ob);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not start sending outbox.");
    return rv;
  }

  loop=0;
  rv=AH_Outbox_SendAndRecv(ob, AH_HBCI_GetTransferTimeout(ob->hbci));
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error while sending outbox.");
    return rv;
  }

  finishedJobs=AH_Outbox_CountFinishedJobs(ob)-finishedJobsBefore;
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Finished jobs: %d", finishedJobs);
  for (i=0; i<finishedJobs; i++) {
    int rv;

    rv=AB_Banking_ProgressAdvance(AH_HBCI_GetBankingApi(ob->hbci), 0,
                                  AB_BANKING_PROGRESS_ONE);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "User aborted");
      return rv;
    }
  }

  AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(ob->hbci),
                         0,
                         AB_Banking_LogLevelNotice,
                         I18N("AqHBCI finished."));
  return 0;
}



int AH_Outbox_Execute(AH_OUTBOX *ob, int withProgress, int nounmount) {
  int rv;
  GWEN_TYPE_UINT32 pid=0;

  assert(ob);
  if (withProgress) {
    pid=AB_Banking_ProgressStart(AH_HBCI_GetBankingApi(ob->hbci),
				 I18N("Executing Jobs"),
				 I18N("Now the jobs are send via their "
				      "backends to the credit institutes."),
				 AH_Outbox_CountTodoJobs(ob));

  }

  GWEN_WaitCallback_Enter(AH_OUTBOX_EXECUTE_WCB_ID);
  rv=AH_Outbox__Execute(ob);
  /* unmount currently mounted medium */
  if (!nounmount)
    AH_HBCI_UnmountCurrentMedium(ob->hbci);
  if (withProgress) {
    AB_Banking_ProgressEnd(AH_HBCI_GetBankingApi(ob->hbci), pid);
  }
  GWEN_WaitCallback_Leave();
  return rv;
}



AH_JOB *AH_Outbox__FindTransferJobInCheckJobList(const AH_JOB_LIST *jl,
                                                 AH_CUSTOMER *cu,
                                                 AH_ACCOUNT *a,
                                                 int isTransfer) {
  AH_JOB *j;

  assert(jl);
  j=AH_Job_List_First(jl);
  while(j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Checking job \"%s\"",
              AH_Job_GetName(j));
    if (strcasecmp(AH_Job_GetName(j),
                   isTransfer?"JobMultiTransfer":"JobMultiDebitNote")
        ==0 &&
        AH_AccountJob_GetAccount(j)==a) {
      if (AH_Job_MultiTransferBase_GetTransferCount(j)<
          AH_Job_MultiTransferBase_GetMaxTransfers(j))
        break;
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Job's already full");
      }
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job doesn't match");
    }

    j=AH_Job_List_Next(j);
  } /* while */

  return j;
}



AH_JOB *AH_Outbox_FindTransferJob(AH_OUTBOX *ob,
                                  AH_CUSTOMER *cu,
                                  AH_ACCOUNT *a,
                                  int isTransfer) {
  AH_OUTBOX__CBOX *cbox;
  AH_JOB *j;

  assert(ob);
  assert(cu);
  assert(a);

  DBG_ERROR(AQHBCI_LOGDOMAIN, "Searching for %s job",
            isTransfer?"transfer":"debitnote");
  cbox=AH_Outbox__CBox_List_First(ob->customerBoxes);
  while(cbox) {
    if (cbox->customer==cu) {
      AH_JOBQUEUE *jq;

      /* check jobs in lists */
      j=AH_Outbox__FindTransferJobInCheckJobList(cbox->todoJobs,
                                                 cu, a, isTransfer);
      if (j)
        return j;

      /* check jobs in queues */
      jq=AH_JobQueue_List_First(cbox->todoQueues);
      while(jq) {
        const AH_JOB_LIST *jl;

        jl=AH_JobQueue_GetJobList(jq);
        if (jl) {
          j=AH_Outbox__FindTransferJobInCheckJobList(jl, cu, a, isTransfer);
          if (j)
            return j;
        }
        jq=AH_JobQueue_List_Next(jq);
      } /* while */
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Customer doesn't match");
    }

    cbox=AH_Outbox__CBox_List_Next(cbox);
  } /* while */

  DBG_INFO(AQHBCI_LOGDOMAIN, "No matching multi job found");
  return 0;
}







