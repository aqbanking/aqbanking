/***************************************************************************
    begin       : Tue Jun 03 2018
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_sendcmd.h"
#include "aqhbci/banking/provider_job.h"

#include <aqbanking/i18n_l.h>

#include <gwenhywfar/gui.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _addCommandToOutbox(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *t, AH_OUTBOX *outbox);
static int _addCommandsToOutbox(AB_PROVIDER *pro, const AB_USERQUEUE_LIST *uql, AB_IMEXPORTER_CONTEXT *ctx,
                                AH_OUTBOX *outbox);
static int _sampleResults(AB_PROVIDER *pro, AH_OUTBOX *outbox, AB_IMEXPORTER_CONTEXT *ctx);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_Provider_SendCommands(AB_PROVIDER *pro, AB_PROVIDERQUEUE *pq, AB_IMEXPORTER_CONTEXT *ctx)
{
  AB_USERQUEUE_LIST *uql;
  int rv;
  int rv2;
  AH_OUTBOX *outbox;

  assert(pro);

  /* sort into user queue list */
  uql=AB_UserQueue_List_new();
  rv=AB_Provider_SortProviderQueueIntoUserQueueList(pro, pq, uql);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Provider_FreeUsersAndAccountsFromUserQueueList(pro, uql);
    AB_UserQueue_List_free(uql);
    return rv;
  }

  /* add users to outbox */
  outbox=AH_Outbox_new(pro);
  rv=_addCommandsToOutbox(pro, uql, ctx, outbox);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Provider_FreeUsersAndAccountsFromUserQueueList(pro, uql);
    AB_UserQueue_List_free(uql);
    return rv;
  }

  /* actually send commands */
  rv=AH_Outbox_Execute(outbox, ctx, 0, 1, 1);
  if (rv<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error executing outbox (%d).", rv);
    rv=GWEN_ERROR_GENERIC;
  }

  /* gather results */
  rv2=_sampleResults(pro, outbox, ctx);
  if (rv2<0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error sampling results (%d)", rv2);
  }

  AH_Outbox_free(outbox);


  /* release accounts and users we loaded */
  AB_Provider_FreeUsersAndAccountsFromUserQueueList(pro, uql);

  AB_UserQueue_List_free(uql);

  /* error code from AH_Outbox_Execute is more important than that from _sampleResults */
  if (rv>=0)
    rv=rv2;

  return rv;
}




int _addCommandToOutbox(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *a, AB_TRANSACTION *t, AH_OUTBOX *outbox)
{
  int rv;
  int cmd;
  AH_JOB *mj=NULL;
  int jobIsNew=1;

  cmd=AB_Transaction_GetCommand(t);

  /* try to get an existing multi job to add the new one to */
  rv=AH_Provider_GetMultiHbciJob(pro, outbox, u, a, cmd, &mj);
  if (rv==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Reusing existing multi job");
    AB_Banking_LogMsgForJobId(AB_Provider_GetBanking(pro),
                              AB_Transaction_GetUniqueId(t),
                              "Reusing existing HBCI job %08x, look there for further logs",
                              AH_Job_GetId(mj));
    jobIsNew=0;
  }
  else {
    if (rv!=GWEN_ERROR_NOT_FOUND) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error looking for multi job (%d), ignoring", rv);
      AB_Banking_LogMsgForJobId(AB_Provider_GetBanking(pro),
                                AB_Transaction_GetUniqueId(t),
                                "Error searching for multi-job (%d)",
                                rv);
    }
  }

  /* create new job if necessary */
  if (mj==NULL) {
    rv=AH_Provider_CreateHbciJob(pro, u, a, cmd, &mj);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_LogMsgForJobId(AB_Provider_GetBanking(pro),
                                AB_Transaction_GetUniqueId(t),
                                "Error creating HbciJob (%d)",
                                rv);
      return rv;
    }
    AB_Banking_LogMsgForJobId(AB_Provider_GetBanking(pro), AB_Transaction_GetUniqueId(t), "Created new HBCI job (%08x)",
                              AH_Job_GetFlags(mj));
  }
  assert(mj);

  if (AH_Job_GetId(mj)==0) {
    int jid;

    /*jid=AB_Banking_GetNamedUniqueId(AB_Provider_GetBanking(pro), "job", 1);*/
    jid=AB_Transaction_GetUniqueId(t); /* reuse unique id */
    assert(jid);
    AH_Job_SetId(mj, jid);
  }

  if (jobIsNew) {
    int sigs;

    /* check whether we need to sign the job */
    sigs=AH_Job_GetMinSignatures(mj);
    if (sigs) {
      if (sigs>1) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Multiple signatures not yet supported");
        GWEN_Gui_ProgressLog(0,
                             GWEN_LoggerLevel_Error,
                             I18N("ERROR: Multiple signatures not yet supported"));
        AB_Banking_LogMsgForJobId(AB_Provider_GetBanking(pro), AH_Job_GetId(mj), "Multiple signatures not supported");
        AH_Job_free(mj);
        return GWEN_ERROR_GENERIC;
      }
      AH_Job_AddSigner(mj, AB_User_GetUserId(u));
    }
  }

  /* exchange arguments */
  AB_Banking_LogMsgForJobId(AB_Provider_GetBanking(pro), AH_Job_GetId(mj),
                            "Letting job handle command %08x",
                            (unsigned int) AB_Transaction_GetUniqueId(t));
  rv=AH_Job_HandleCommand(mj, t);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AB_Banking_LogMsgForJobId(AB_Provider_GetBanking(pro), AH_Job_GetId(mj),
                              "Error on AH_Job_HandleCommand(): %d", rv);
    if (jobIsNew)
      AH_Job_free(mj);
    return rv;
  }

  /* add command to job */
  AH_Job_AddCommand(mj, t);
  AB_Banking_LogCmdInfoMsgForJob(AB_Provider_GetBanking(pro), t, AH_Job_GetId(mj), "Added command to job: ");

  if (jobIsNew) {
    /* add job to outbox */
    AH_Outbox_AddJob(outbox, mj);
    AB_Banking_LogMsgForJobId(AB_Provider_GetBanking(pro), AH_Job_GetId(mj), "Job added to outbox");
    AH_Job_free(mj);
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Command successfully added");
  return 0;
}



int _addCommandsToOutbox(AB_PROVIDER *pro, const AB_USERQUEUE_LIST *uql, AB_IMEXPORTER_CONTEXT *ctx,
                         AH_OUTBOX *outbox)
{
  AB_USERQUEUE *uq;

  assert(pro);

  uq=AB_UserQueue_List_First(uql);
  while (uq) {
    AB_ACCOUNTQUEUE_LIST *aql;
    AB_USER *u;

    u=AB_UserQueue_GetUser(uq);
    assert(u);
    DBG_NOTICE(0, "Handling user \"%s\"", AB_User_GetUserId(u));
    aql=AB_UserQueue_GetAccountQueueList(uq);
    if (aql) {
      AB_ACCOUNTQUEUE *aq;

      aq=AB_AccountQueue_List_First(aql);
      while (aq) {
        AB_ACCOUNT *a;
        AB_TRANSACTION_LIST2 *tl2;

        a=AB_AccountQueue_GetAccount(aq);
        assert(a);

        /* read transactions */
        tl2=AB_AccountQueue_GetTransactionList(aq);
        if (tl2) {
          AB_TRANSACTION_LIST2_ITERATOR *it;

          it=AB_Transaction_List2_First(tl2);
          if (it) {
            AB_TRANSACTION *t;

            t=AB_Transaction_List2Iterator_Data(it);
            while (t) {
              int rv;

              rv=_addCommandToOutbox(pro, u, a, t, outbox);
              if (rv<0) {
                DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
                AB_Transaction_SetStatus(t, AB_Transaction_StatusError);
              }
              t=AB_Transaction_List2Iterator_Next(it);
            }

            AB_Transaction_List2Iterator_free(it);
          }
        }

        aq=AB_AccountQueue_List_Next(aq);
      }
    }

    uq=AB_UserQueue_List_Next(uq);
  }

  return 0;
}




int _sampleResults(AB_PROVIDER *pro, AH_OUTBOX *outbox, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_LIST *mjl;
  AH_JOB *j;

  assert(pro);

  assert(outbox);

  mjl=AH_Outbox_GetFinishedJobs(outbox);
  assert(mjl);

  j=AH_Job_List_First(mjl);
  while (j) {
    AB_MESSAGE_LIST *ml;
    int rv;

    if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Info) {
      const GWEN_STRINGLIST *sl;
      GWEN_STRINGLISTENTRY *se;

      /* exchange logs */
      sl=AH_Job_GetLogs(j);
      assert(sl);
      se=GWEN_StringList_FirstEntry(sl);
      if (se) {
        DBG_DEBUG(AQHBCI_LOGDOMAIN, "Logs for job %s", AH_Job_GetName(j));
        while (se) {
          const char *s;

          s=GWEN_StringListEntry_Data(se);
          assert(s);
          DBG_DEBUG(AQHBCI_LOGDOMAIN, "- %s", s);
          //AB_Job_LogRaw(bj, s);
          se=GWEN_StringListEntry_Next(se);
        }
      }
    }

    /* get remaining results */
    rv=AH_Job_HandleResults(j, ctx);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    }

    /* copy messages from AH_JOB to imexporter context */
    ml=AH_Job_GetMessages(j);
    if (ml) {
      AB_MESSAGE *msg;

      msg=AB_Message_List_First(ml);
      while (msg) {
        AB_ImExporterContext_AddMessage(ctx, AB_Message_dup(msg));
        msg=AB_Message_List_Next(msg);
      }
    }

    j=AH_Job_List_Next(j);
  } /* while j */

  return 0;
}




