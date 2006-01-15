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

#include "provider_p.h"
#include "aqhbci_l.h"
#include "account_l.h"
#include "hbci_l.h"
#include "outbox_l.h"
#include "user_l.h"
#include "medium_l.h"
#include <aqbanking/account_be.h>
#include <aqbanking/provider_be.h>
#include <aqbanking/job_be.h>
#include <aqhbci/user.h>
#include <aqhbci/jobgetbalance.h>
#include <aqhbci/jobgettransactions.h>
#include <aqhbci/jobgetstandingorders.h>
#include <aqhbci/jobgetdatedxfers.h>
#include <aqhbci/jobsingletransfer.h>
#include <aqhbci/jobmultitransfer.h>
#include <aqhbci/jobeutransfer.h>
#include <aqhbci/adminjobs.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/md.h>
#include <gwenhywfar/debug.h>

#include <ctype.h>
#include <stdlib.h>


#ifdef OS_WIN32
# define AH_PATH_SEP "\\"
#else
# define AH_PATH_SEP "/"
#endif



GWEN_INHERIT(AB_PROVIDER, AH_PROVIDER);


AB_PROVIDER *AH_Provider_new(AB_BANKING *ab, const char *name){
  AB_PROVIDER *pro;
  AH_PROVIDER *hp;
  GWEN_BUFFER *pbuf;

  pbuf=0;
  pro=AB_Provider_new(ab, name);
  assert(pro);

  AB_Provider_SetInitFn(pro, AH_Provider_Init);
  AB_Provider_SetFiniFn(pro, AH_Provider_Fini);
  AB_Provider_SetUpdateJobFn(pro, AH_Provider_UpdateJob);
  AB_Provider_SetAddJobFn(pro, AH_Provider_AddJob);
  AB_Provider_SetExecuteFn(pro, AH_Provider_Execute);
  AB_Provider_SetResetQueueFn(pro, AH_Provider_ResetQueue);
  AB_Provider_SetExtendUserFn(pro, AH_Provider_ExtendUser);
  AB_Provider_SetExtendAccountFn(pro, AH_Provider_ExtendAccount);
  AB_Provider_SetUpdateFn(pro, AH_Provider_Update);

  GWEN_NEW_OBJECT(AH_PROVIDER, hp);
  GWEN_INHERIT_SETDATA(AB_PROVIDER, AH_PROVIDER, pro, hp,
                       AH_Provider_FreeData);

  hp->hbci=AH_HBCI_new(pro);
  assert(hp->hbci);
  GWEN_Buffer_free(pbuf);

  hp->dbTempConfig=GWEN_DB_Group_new("tmpConfig");
  hp->bankingJobs=AB_Job_List2_new();

  /* create job plugin list */
  hp->jobPlugins=AH_JobPlugin_List_new();

  return pro;
}



void AH_Provider_FreeData(void *bp, void *p) {
  AH_PROVIDER *hp;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Destroying AH_PROVIDER");
  hp=(AH_PROVIDER*)p;
  AB_Job_List2_FreeAll(hp->bankingJobs);
  AH_Outbox_free(hp->outbox);
  AH_JobPlugin_List_free(hp->jobPlugins);

  GWEN_DB_Group_free(hp->dbTempConfig);
  AH_HBCI_free(hp->hbci);

  GWEN_FREE_OBJECT(hp);
}



int AH_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData) {
  AH_PROVIDER *hp;
  int rv;
  const char *logLevelName;

  if (!GWEN_Logger_IsOpen(AQHBCI_LOGDOMAIN)) {
    GWEN_Logger_Open(AQHBCI_LOGDOMAIN,
		     "aqhbci", 0,
		     GWEN_LoggerTypeConsole,
		     GWEN_LoggerFacilityUser);
  }

  logLevelName=getenv("AQHBCI_LOGLEVEL");
  if (logLevelName) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(logLevelName);
    if (ll!=GWEN_LoggerLevelUnknown) {
      GWEN_Logger_SetLevel(AQHBCI_LOGDOMAIN, ll);
      DBG_WARN(AQHBCI_LOGDOMAIN,
               "Overriding loglevel for AqHBCI with \"%s\"",
               logLevelName);
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown loglevel \"%s\"",
                logLevelName);
    }
  }

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Initializing AqHBCI backend");
  assert(pro);

  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  GWEN_DB_ClearGroup(hp->dbTempConfig, 0);

  hp->dbConfig=dbData;
  rv=AH_HBCI_Init(hp->hbci);

  AH_Provider_LoadAllJobPlugins(pro);

  return rv;
}



int AH_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData) {
  AH_PROVIDER *hp;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Deinitializing AqHBCI backend");

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  AB_Job_List2_FreeAll(hp->bankingJobs);
  hp->bankingJobs=AB_Job_List2_new();
  AH_Outbox_free(hp->outbox);
  hp->outbox=0;

  AH_JobPlugin_List_free(hp->jobPlugins);
  hp->jobPlugins=AH_JobPlugin_List_new();

  rv=AH_HBCI_Fini(hp->hbci);
  GWEN_DB_ClearGroup(hp->dbTempConfig, 0);
  hp->dbConfig=0;

  return rv;
}



const char *AH_Provider_GetProductName(const AB_PROVIDER *pro) {
  AH_HBCI *h;

  assert(pro);
  h=AH_Provider_GetHbci(pro);
  assert(h);
  return AH_HBCI_GetProductName(h);
}



const char *AH_Provider_GetProductVersion(const AB_PROVIDER *pro) {
  AH_HBCI *h;

  assert(pro);
  h=AH_Provider_GetHbci(pro);
  assert(h);
  return AH_HBCI_GetProductVersion(h);
}



const AH_MEDIUM_LIST *AH_Provider_GetMediaList(AB_PROVIDER *pro){
  AH_HBCI *h;

  assert(pro);
  h=AH_Provider_GetHbci(pro);
  assert(h);
  return AH_HBCI_GetMediaList(h);
}



int AH_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j){
  AH_PROVIDER *hp;
  GWEN_DB_NODE *dbAccount;
  GWEN_DB_NODE *dbJob;
  AH_JOB *mj;
  AB_USER *mu;
  AB_ACCOUNT *ma;
  int rv;
  GWEN_TYPE_UINT32 uFlags;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  dbAccount=AB_Account_GetProviderData(AB_Job_GetAccount(j));
  assert(dbAccount);

  dbJob=AB_Job_GetProviderData(j, pro);
  assert(dbJob);

  ma=AB_Job_GetAccount(j);
  assert(ma);

  /* determine customer to use */
  mu=AB_Account_GetFirstUser(ma);
  if (!mu) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No customer for this account");
    return AB_ERROR_NOT_AVAILABLE;
  }

  uFlags=AH_User_GetFlags(mu);

  mj=0;
  switch(AB_Job_GetType(j)) {

  case AB_Job_TypeGetBalance:
    mj=AH_Job_GetBalance_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeGetTransactions:
    mj=AH_Job_GetTransactions_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeGetStandingOrders:
    mj=AH_Job_GetStandingOrders_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeGetDatedTransfers:
    mj=AH_Job_GetDatedTransfers_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeTransfer:
    mj=0;
    if (!(uFlags & AH_USER_FLAGS_PREFER_SINGLE_TRANSFER)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Customer prefers multi jobs");
      /* create new job */
      mj=AH_Job_MultiTransfer_new(mu, ma);
      if (!mj) {
	DBG_WARN(AQHBCI_LOGDOMAIN,
		 "Multi-job not supported with this account, "
		 "using single-job");
      }
    }

    if (mj) {
      GWEN_DB_SetIntValue(dbJob, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "isMultiJob", 1);
    }
    else {
      GWEN_DB_SetIntValue(dbJob, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "isMultiJob", 0);
      mj=AH_Job_SingleTransfer_new(mu, ma);
      if (!mj) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
	return AB_ERROR_NOT_AVAILABLE;
      }
    }
    break;

  case AB_Job_TypeDebitNote:
    mj=0;
    if (!(uFlags & AH_USER_FLAGS_PREFER_SINGLE_DEBITNOTE)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Customer prefers multi jobs");
      /* create new job */
      mj=AH_Job_MultiDebitNote_new(mu, ma);
      if (!mj) {
	DBG_WARN(AQHBCI_LOGDOMAIN,
		 "Multi-job not supported with this account, "
		 "using single-job");
      }
    }
    if (mj) {
      GWEN_DB_SetIntValue(dbJob, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "isMultiJob", 1);
    }
    else {
      GWEN_DB_SetIntValue(dbJob, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "isMultiJob", 0);
      mj=AH_Job_SingleDebitNote_new(mu, ma);
      if (!mj) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
	return AB_ERROR_NOT_AVAILABLE;
      }
    }
    break;

  case AB_Job_TypeEuTransfer:
    mj=AH_Job_EuTransfer_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeCreateStandingOrder:
    mj=AH_Job_CreateStandingOrder_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeModifyStandingOrder:
    mj=AH_Job_ModifyStandingOrder_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeDeleteStandingOrder:
    mj=AH_Job_DeleteStandingOrder_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeCreateDatedTransfer:
    mj=AH_Job_CreateDatedTransfer_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeModifyDatedTransfer:
    mj=AH_Job_ModifyDatedTransfer_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeDeleteDatedTransfer:
    mj=AH_Job_DeleteDatedTransfer_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  default:
    return AB_ERROR_NOT_AVAILABLE;
  }

  /* exchange parameters */
  rv=AH_Job_Exchange(mj, j, AH_Job_ExchangeModeParams);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error exchanging params");
    AH_Job_free(mj);
    return rv;
  }

  /* free my job, it is no longer needed here */
  AH_Job_free(mj);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Job successfully updated");
  return 0;
}



int AH_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j){
  AH_PROVIDER *hp;
  GWEN_DB_NODE *dbAccount;
  GWEN_DB_NODE *dbJob;
  AH_JOB *mj;
  GWEN_TYPE_UINT32 jid;
  AB_ACCOUNT *ma;
  AB_USER *mu;
  int rv;
  int sigs;
  int jobIsNew=1;
  AB_JOB_STATUS jst;
  GWEN_TYPE_UINT32 uFlags;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  if (hp->outbox==0)
    hp->outbox=AH_Outbox_new(hp->hbci);
  assert(hp->outbox);

  dbJob=AB_Job_GetProviderData(j, pro);
  assert(dbJob);

  jst=AB_Job_GetStatus(j);
  if (jst==AB_Job_StatusPending) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Adding pending job for verification");
    AH_Outbox_AddPendingJob(hp->outbox, j);
    return 0;
  }

  jid=AB_Job_GetIdForProvider(j);
  if (jid) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Jobs has already been sent to this backend, rejecting");
    return AB_ERROR_INVALID;
  }

  dbAccount=AB_Account_GetProviderData(AB_Job_GetAccount(j));
  assert(dbAccount);

  ma=AB_Job_GetAccount(j);
  assert(ma);

  /* determine customer to use */
  mu=AB_Account_GetFirstUser(ma);
  if (!mu) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No customers noted for account \"%s/%s\"",
              AB_Account_GetBankCode(ma),
              AB_Account_GetAccountNumber(ma));
    return AB_ERROR_NOT_AVAILABLE;
  }

  uFlags=AH_User_GetFlags(mu);

  mj=0;
  switch(AB_Job_GetType(j)) {

  case AB_Job_TypeGetBalance:
    mj=AH_Job_GetBalance_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeGetTransactions:
    mj=AH_Job_GetTransactions_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeGetStandingOrders:
    mj=AH_Job_GetStandingOrders_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeGetDatedTransfers:
    mj=AH_Job_GetDatedTransfers_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeTransfer:
    mj=0;
    if (!(uFlags & AH_USER_FLAGS_PREFER_SINGLE_TRANSFER)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Customer prefers multi jobs");
      mj=AH_Outbox_FindTransferJob(hp->outbox, mu, ma, 1);
      if (mj) {
	/* simply add transfer to existing job */
        jobIsNew=0;
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No matching job found");
      }
      else {
	/* create new job */
	mj=AH_Job_MultiTransfer_new(mu, ma);
	if (!mj) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Multi-job not supported with this account, "
		    "using single-job");
	}
      }
    }
    if (!mj) {
      mj=AH_Job_SingleTransfer_new(mu, ma);
      if (!mj) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
	return AB_ERROR_NOT_AVAILABLE;
      }
    }
    break;

  case AB_Job_TypeDebitNote:
    mj=0;
    if (!(uFlags & AH_USER_FLAGS_PREFER_SINGLE_DEBITNOTE)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Customer prefers multi jobs");
      mj=AH_Outbox_FindTransferJob(hp->outbox, mu, ma, 0);
      if (mj) {
	/* simply add transfer to existing job */
        jobIsNew=0;
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No matching job found");
      }
      else {
	/* create new job */
	mj=AH_Job_MultiDebitNote_new(mu, ma);
	if (!mj) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Multi-job not supported with this account, "
		    "using single-job");
	}
      }
    }
    if (!mj) {
      mj=AH_Job_SingleDebitNote_new(mu, ma);
      if (!mj) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
	return AB_ERROR_NOT_AVAILABLE;
      }
    }
    break;

  case AB_Job_TypeEuTransfer:
    mj=AH_Job_EuTransfer_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeCreateStandingOrder:
    mj=AH_Job_CreateStandingOrder_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeModifyStandingOrder:
    mj=AH_Job_ModifyStandingOrder_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeDeleteStandingOrder:
    mj=AH_Job_DeleteStandingOrder_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeCreateDatedTransfer:
    mj=AH_Job_CreateDatedTransfer_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeModifyDatedTransfer:
    mj=AH_Job_ModifyDatedTransfer_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeDeleteDatedTransfer:
    mj=AH_Job_DeleteDatedTransfer_new(mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  default:
    mj=AH_Provider__GetPluginJob(hp, AB_Job_GetType(j), mu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
  } /* switch */
  assert(mj);

  if (jobIsNew) {
    /* check whether we need to sign the job */
    sigs=AH_Job_GetMinSignatures(mj);
    if (sigs) {
      if (sigs>1) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Multiple signatures not yet supported");
	AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(hp->hbci),
			       0,
			       AB_Banking_LogLevelError,
			       "ERROR: Multiple signatures not yet supported");
	return AB_ERROR_GENERIC;
      }
      AH_Job_AddSigner(mj, AB_User_GetUserId(mu));
    }
  }

  /* exchange arguments */
  rv=AH_Job_Exchange(mj, j, AH_Job_ExchangeModeArgs);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error exchanging params");
    AH_Job_free(mj);
    return rv;
  }

  /* store HBCI job, link both jobs */
  if (AH_Job_GetId(mj)==0) {
    jid=AB_Job_GetJobId(j);
    assert(jid);
    /* we now use the same id here */
    AH_Job_SetId(mj, jid);
  }
  AB_Job_SetIdForProvider(j, AH_Job_GetId(mj));

  if (jobIsNew) {
    /* prevent outbox from freeing this job */
    AH_Job_Attach(mj);
    /* add job to outbox */
    AH_Outbox_AddJob(hp->outbox, mj);
  }
  AB_Job_Attach(j);
  AB_Job_List2_PushBack(hp->bankingJobs, j);
  AB_Job_SetStatus(j, AB_Job_StatusSent);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Job successfully added");
  return 0;
}



AH_JOB *AH_Provider__FindMyJob(AH_JOB_LIST *mjl, GWEN_TYPE_UINT32 jid){
  AH_JOB *mj;

  assert(mjl);

  /* FIXME: This used to be DBG_ERROR, but isn't DBG_NOTICE sufficient? */
  DBG_WARN(AQHBCI_LOGDOMAIN, "Looking for id %08x", jid);
  mj=AH_Job_List_First(mjl);
  while(mj) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Comparing %08x", AH_Job_GetId(mj));
    if (AH_Job_GetId(mj)==jid)
      break;
    mj=AH_Job_List_Next(mj);
  }

  return mj;
}



int AH_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx){
  AH_PROVIDER *hp;
  int rv;
  AB_JOB_LIST2_ITERATOR *jit;
  int successfull;
  AH_JOB_LIST *mjl;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  successfull=0;
  if (hp->outbox==0) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Empty outbox");
    return 0;
  }

  rv=AH_Outbox_Execute(hp->outbox, 0, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error executing outbox.");
    rv=AB_ERROR_GENERIC;
  }

  mjl=AH_Outbox_GetFinishedJobs(hp->outbox);
  /* copy job results to Banking-job, set status etc */
  jit=AB_Job_List2_First(hp->bankingJobs);
  if (jit) {
    AB_JOB *bj;

    bj=AB_Job_List2Iterator_Data(jit);
    assert(bj);
    while(bj) {
      AH_JOB *mj;
      GWEN_DB_NODE *beData;
      const char *s;
      const GWEN_STRINGLIST *sl;
      GWEN_STRINGLISTENTRY *se;

      mj=AH_Provider__FindMyJob(mjl, AB_Job_GetIdForProvider(bj));
      assert(mj);

      beData=AB_Job_GetProviderData(bj, pro);
      assert(beData);

      /* store used TAN (if any) */
      s=AH_Job_GetUsedTan(mj);
      if (s) {
        GWEN_DB_SetCharValue(beData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "usedTan", s);
        AB_Job_SetUsedTan(bj, s);
      }

      if (getenv("AQHBCI_DEBUG_JOBS")) { /* DEBUG */
        GWEN_DB_NODE *dbDebug;

        dbDebug=GWEN_DB_GetGroup(beData, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                                 "rawParams");
        assert(dbDebug);
        GWEN_DB_AddGroupChildren(dbDebug,
                                 AH_Job_GetParams(mj));
        dbDebug=GWEN_DB_GetGroup(beData, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                                    "rawArgs");
        assert(dbDebug);
        GWEN_DB_AddGroupChildren(dbDebug,
                                 AH_Job_GetArguments(mj));

        dbDebug=GWEN_DB_GetGroup(beData, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                                 "rawResponses");
        assert(dbDebug);
        GWEN_DB_AddGroupChildren(dbDebug,
                                 AH_Job_GetResponses(mj));
      }

      /* exchange logs */
      sl=AH_Job_GetLogs(mj);
      assert(sl);
      se=GWEN_StringList_FirstEntry(sl);
      while(se) {
        const char *s;

        s=GWEN_StringListEntry_Data(se);
        assert(s);
        AB_Job_LogRaw(bj, s);
        se=GWEN_StringListEntry_Next(se);
      }


      /* exchange results */
      rv=AH_Job_Exchange(mj, bj, AH_Job_ExchangeModeResults);
      if (rv) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Error exchanging results");
        AB_Job_SetStatus(bj, AB_Job_StatusError);
        AB_Job_SetResultText(bj, "Could not exchange results");
      }
      else {
        /* exchange was ok */
        if (AH_Job_HasErrors(mj)) {
          if (AB_Job_GetStatus(bj)==AB_Job_StatusSent) {
            AB_Job_SetStatus(bj, AB_Job_StatusError);
            /* TODO: Copy errors */
            AB_Job_SetResultText(bj, "Job contains errors");
            AB_Job_Log(bj, AB_Banking_LogLevelWarn, "aqhbci",
                       "Job contains errors");
          }
        }
        else {
          /* job is ok */
          if (AB_Job_GetStatus(bj)==AB_Job_StatusSent) {
            AB_Job_SetStatus(bj, AB_Job_StatusFinished);
            AB_Job_Log(bj, AB_Banking_LogLevelNotice, "aqhbci",
                       "Job finished successfully");
            AB_Job_SetResultText(bj, "Ok.");
          }
          successfull++;
        }
      }
      bj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  /* free outbox, the next AddJob call will create a new one */
  AH_Outbox_free(hp->outbox);
  hp->outbox=0;

  /* release all jobs from my hold. If the application still has a hold
   * on a job then the following "free" will not actually free
   * that job but decrement its usage counter. */
  AB_Job_List2_FreeAll(hp->bankingJobs);
  hp->bankingJobs=AB_Job_List2_new();

  if (!successfull)
    return AB_ERROR_GENERIC;

  return 0;
}



int AH_Provider_ResetQueue(AB_PROVIDER *pro){
  AH_PROVIDER *hp;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  /* free outbox, the next AddJob call will create a new one */
  AH_Outbox_free(hp->outbox);
  hp->outbox=0;

  /* release all jobs from my hold. If the application still has a hold
   * on a job then the following "free" will not actually free
   * that job but decrement its usage counter. */
  AB_Job_List2_FreeAll(hp->bankingJobs);
  hp->bankingJobs=AB_Job_List2_new();

  return 0;
}



int AH_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
                           AB_PROVIDER_EXTEND_MODE em) {
  AH_User_Extend(u, pro, em);
  return 0;
}



int AH_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
                              AB_PROVIDER_EXTEND_MODE em){
  AH_Account_Extend(a, pro, em);
  return 0;
}



int AH_Provider_Update(AB_PROVIDER *pro,
                       GWEN_TYPE_UINT32 lastVersion,
                       GWEN_TYPE_UINT32 currentVersion) {
  AH_PROVIDER *hp;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  return AH_HBCI_Update(hp->hbci, lastVersion, currentVersion);
}




AH_HBCI *AH_Provider_GetHbci(const AB_PROVIDER *pro){
  AH_PROVIDER *hp;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  return hp->hbci;
}



AH_JOB *AH_Provider__GetPluginJob(AH_PROVIDER *hp,
                                  AB_JOB_TYPE jt,
                                  AB_USER *mcu,
                                  AB_ACCOUNT *ma){
  AH_JOBPLUGIN *jp;
  AH_JOB *j;

  /* search for a plugin which supports this job */
  jp=AH_JobPlugin_List_First(hp->jobPlugins);
  while(jp) {
    if (AH_JobPlugin_CheckType(jp, jt))
      break;
    jp=AH_JobPlugin_List_Next(jp);
  }

  if (!jp) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No plugin found for job type %d", jt);
    return 0;
  }

  /* let the plugin create the job */
  j=AH_JobPlugin_Factory(jp, jt, mcu, ma);
  if (!j) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Plugin found, but job not created (type %d)", jt);
    return 0;
  }

  return j;
}



AH_JOBPLUGIN *AH_Provider_FindJobPlugin(AH_PROVIDER *hp, const char *name) {
  AH_JOBPLUGIN *jp;

  jp=AH_JobPlugin_List_First(hp->jobPlugins);
  while(jp) {
    if (strcasecmp(name, AH_JobPlugin_GetName(jp))==0)
      break;
    jp=AH_JobPlugin_List_Next(jp);
  }

  return jp;
}



AH_JOBPLUGIN *AH_Provider_LoadJobPlugin(AH_PROVIDER *hp,
                                        const char *path,
                                        const char *modname){
  GWEN_LIBLOADER *ll;
  AH_JOBPLUGIN *jp;
  AH_JOBPLUGIN_NEWFN fn;
  void *p;
  const char *s;
  GWEN_ERRORCODE err;
  GWEN_BUFFER *mbuf;

  ll=GWEN_LibLoader_new();
  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(mbuf, tolower((int)(*(s++))));

  if (GWEN_LibLoader_OpenLibraryWithPath(ll,
                                         path,
                                         modname)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Could not load job plugin \"%s\"", modname);
    GWEN_Buffer_free(mbuf);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* create name of init function */
  GWEN_Buffer_AppendString(mbuf, "_factory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(mbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
    GWEN_Buffer_free(mbuf);
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }
  GWEN_Buffer_free(mbuf);

  fn=(AH_JOBPLUGIN_NEWFN)p;
  assert(fn);
  jp=fn(hp);
  if (!jp) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in plugin: No instance created");
    GWEN_LibLoader_CloseLibrary(ll);
    GWEN_LibLoader_free(ll);
    return 0;
  }

  /* store libloader */
  AH_JobPlugin_SetLibLoader(jp, ll);

  return jp;
}


int AH_Provider_LoadJobPlugins(AH_PROVIDER *hp, const char *path){
  GWEN_PLUGIN_DESCRIPTION_LIST2 *l;

  l=GWEN_LoadPluginDescrs(path);
  if (l) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;
    GWEN_PLUGIN_DESCRIPTION *pd;

    it=GWEN_PluginDescription_List2_First(l);
    assert(it);
    pd=GWEN_PluginDescription_List2Iterator_Data(it);
    assert(pd);
    while(pd) {
      AH_JOBPLUGIN *jp;

      jp=AH_Provider_LoadJobPlugin(hp,
                                   path,
                                   GWEN_PluginDescription_GetName(pd));
      if (jp) {
        if (AH_Provider_FindJobPlugin(hp, AH_JobPlugin_GetName(jp))) {
          DBG_INFO(AQHBCI_LOGDOMAIN,
                   "Plugin \"%s\" already loaded, skipping",
                   AH_JobPlugin_GetName(jp));
          AH_JobPlugin_free(jp);
        }
        else {
          DBG_NOTICE(AQHBCI_LOGDOMAIN,
                     "Adding job plugin \"%s\"",
                     AH_JobPlugin_GetName(jp));
          AH_JobPlugin_List_Add(jp, hp->jobPlugins);
        }
      }
      else {
        DBG_WARN(AQHBCI_LOGDOMAIN,
                 "Could not load job plugin \"%s\"",
                 GWEN_PluginDescription_GetName(pd));
      }
      pd=GWEN_PluginDescription_List2Iterator_Next(it);
    } /* while */

    GWEN_PluginDescription_List2Iterator_free(it);
    GWEN_PluginDescription_List2_freeAll(l);
  }
  return 0;
}



int AH_Provider_LoadAllJobPlugins(AB_PROVIDER *pro){
  AH_PROVIDER *hp;
  GWEN_BUFFER *pbuf;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  if (AH_Provider_LoadJobPlugins(hp, AQHBCI_PLUGINS AH_PATH_SEP "jobs")) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Error loading global job plugins");
  }

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (AB_Provider_GetUserDataDir(pro, pbuf)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not determine user data dir");
  }
  else {
    GWEN_Buffer_AppendString(pbuf,
                             AH_PATH_SEP "plugins"
                             AH_PATH_SEP AQHBCI_SO_EFFECTIVE_STR
                             AH_PATH_SEP "jobs");
    if (AH_Provider_LoadJobPlugins(hp,GWEN_Buffer_GetStart(pbuf))) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Error loading global job plugins");
    }
  }
  GWEN_Buffer_free(pbuf);

  return 0;
}



int AH_Provider_GetAccounts(AB_PROVIDER *pro, AB_USER *u,
                            int nounmount) {
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_MEDIUM *m;
  AH_JOB *job;
  AH_OUTBOX *ob;
  AB_ACCOUNT_LIST2 *accs;
  int rv;
  AH_PROVIDER *hp;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(u);
  m=AH_User_GetMedium(u);
  assert(m);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  job=AH_Job_UpdateBank_new(u);
  if (!job) {
    DBG_ERROR(0, "Job not supported, should not happen");
    return AB_ERROR_GENERIC;
  }
  AH_Job_AddSigner(job, AB_User_GetUserId(u));

  ob=AH_Outbox_new(h);
  AH_Outbox_AddJob(ob, job);

  rv=AH_Outbox_Execute(ob, 0, nounmount);
  if (rv) {
    DBG_ERROR(0, "Could not execute outbox.\n");
    return rv;
  }

  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job has errors");
    // TODO: show errors
    AH_Outbox_free(ob);
    return AB_ERROR_GENERIC;
  }
  else {
    rv=AH_Job_Commit(job);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not commit result.\n");
      AH_Outbox_free(ob);
      return rv;
    }
  }

  /* check whether we got some accounts */
  accs=AH_Job_UpdateBank_GetAccountList(job);
  assert(accs);
  if (AB_Account_List2_GetSize(accs)==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No accounts found");
    return AB_ERROR_NO_DATA;
  }

  AH_Outbox_free(ob);

  return 0;
}



int AH_Provider_GetSysId(AB_PROVIDER *pro, AB_USER *u,
                         int nounmount) {
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_MEDIUM *m;
  AH_JOB *job;
  AH_OUTBOX *ob;
  int rv;
  AH_PROVIDER *hp;
  const char *s;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(u);
  m=AH_User_GetMedium(u);
  assert(m);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  job=AH_Job_GetSysId_new(u);
  if (!job) {
    DBG_ERROR(0, "Job not supported, should not happen");
    return AB_ERROR_GENERIC;
  }
  AH_Job_AddSigner(job, AB_User_GetUserId(u));

  ob=AH_Outbox_new(h);
  AH_Outbox_AddJob(ob, job);

  rv=AH_Outbox_Execute(ob, 0, nounmount);
  if (rv) {
    DBG_ERROR(0, "Could not execute outbox.\n");
    return rv;
  }

  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job has errors");
    // TODO: show errors
    AH_Outbox_free(ob);
    return AB_ERROR_GENERIC;
  }
  else {
    rv=AH_Job_Commit(job);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not commit result.\n");
      AH_Outbox_free(ob);
      return rv;
    }
  }

  s=AH_Job_GetSysId_GetSysId(job);
  if (!s) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No system id");
    AH_Outbox_free(ob);
    return AB_ERROR_NO_DATA;
  }

  rv=AH_Medium_SelectContext(m, AH_User_GetContextIdx(u));
  if (rv) {
    DBG_ERROR(0, "Could not select user");
    AH_Outbox_free(ob);
    return rv;
  }

  AH_User_SetSystemId(u, s);

  AH_Outbox_free(ob);

  return 0;
}



int AH_Provider_GetServerKeys(AB_PROVIDER *pro, AB_USER *u, int nounmount) {
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_MEDIUM *m;
  AH_JOB *job;
  AH_OUTBOX *ob;
  int rv;
  AH_PROVIDER *hp;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(u);
  m=AH_User_GetMedium(u);
  assert(m);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  job=AH_Job_GetKeys_new(u);
  if (!job) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported, should not happen");
    return AB_ERROR_GENERIC;
  }

  ob=AH_Outbox_new(h);
  AH_Outbox_AddJob(ob, job);

  rv=AH_Outbox_Execute(ob, 0, 1);
  if (rv) {
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Could not execute outbox."));
    if (!nounmount)
      AH_Medium_Unmount(m, 1);
    return rv;
  }

  if (AH_Job_GetKeys_GetCryptKey(job)==0) {
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("No crypt key received."));
    if (!nounmount)
      AH_Medium_Unmount(m, 1);
    return AB_ERROR_GENERIC;
  }

  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(0, "Job has errors");
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Job contains errors."));
    AH_Outbox_free(ob);
    if (!nounmount)
      AH_Medium_Unmount(m, 1);
    return AB_ERROR_GENERIC;
  }
  else {
    rv=AH_Job_Commit(job);
    if (rv) {
      DBG_ERROR(0, "Could not commit result.\n");
      AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                             I18N("Could not commit result"));
      AH_Outbox_free(ob);
      if (!nounmount)
        AH_Medium_Unmount(m, 1);
      return rv;
    }
  }

  if (AH_Job_GetKeys_GetSignKey(job)==0) {
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelNotice,
                           I18N("Bank does not use a sign key."));
  }

  AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelNotice,
                         I18N("Keys saved"));

  AH_Outbox_free(ob);

  if (!nounmount)
    AH_Medium_Unmount(m, 1);

  return 0;
}



int AH_Provider_SendUserKeys(AB_PROVIDER *pro, AB_USER *u, int nounmount) {
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_MEDIUM *m;
  AH_JOB *job;
  AH_OUTBOX *ob;
  int rv;
  AH_PROVIDER *hp;
  GWEN_CRYPTKEY *signKey;
  GWEN_CRYPTKEY *cryptKey;
  int mounted=0;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(u);
  m=AH_User_GetMedium(u);
  assert(m);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  /* mount medium if necessary */
  if (!AH_Medium_IsMounted(m)) {
    rv=AH_Medium_Mount(m);
    if (rv) {
      DBG_ERROR(0, "Could not mount medium");
      AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                             I18N("Could not mount medium"));
      return rv;
    }
    mounted=1;
  }

  /* select user's context */
  rv=AH_Medium_SelectContext(m, AH_User_GetContextIdx(u));
  if (rv) {
    DBG_ERROR(0, "Could not select user");
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Could not select context"));
    if (!nounmount && mounted)
      AH_Medium_Unmount(m, 1);
    return rv;
  }

  /* get keys */
  signKey=AH_Medium_GetLocalPubSignKey(m);
  cryptKey=AH_Medium_GetLocalPubCryptKey(m);
  if (!signKey || !cryptKey) {
    DBG_ERROR(0, "Either sign- or crypt key missing");
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Either sign- or crypt key missing"));
    GWEN_CryptKey_free(signKey);
    GWEN_CryptKey_free(cryptKey);
    if (!nounmount && mounted)
      AH_Medium_Unmount(m, 1);
    return AB_ERROR_NOT_FOUND;
  }

  /* create job */
  job=AH_Job_SendKeys_new(u, cryptKey, signKey);
  AH_Job_AddSigner(job, AB_User_GetUserId(u));
  GWEN_CryptKey_free(signKey);
  GWEN_CryptKey_free(cryptKey);
  if (!job) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported, should not happen");
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Job not supported, should not happen"));
    if (!nounmount && mounted)
      AH_Medium_Unmount(m, 1);
    return AB_ERROR_GENERIC;
  }

  /* enqueue job */
  ob=AH_Outbox_new(h);
  AH_Outbox_AddJob(ob, job);

  /* execute queue */
  rv=AH_Outbox_Execute(ob, 0, nounmount);
  if (rv) {
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Could not execute outbox."));
    AH_Outbox_free(ob);
    if (!nounmount && mounted)
      AH_Medium_Unmount(m, 1);
    return rv;
  }

  /* check result */
  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(0, "Job has errors");
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Job contains errors."));
    AH_Outbox_free(ob);
    if (!nounmount && mounted)
      AH_Medium_Unmount(m, 1);
    return AB_ERROR_GENERIC;
  }
  else {
    rv=AH_Job_Commit(job);
    if (rv) {
      DBG_ERROR(0, "Could not commit result.\n");
      AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                             I18N("Could not commit result"));
      AH_Outbox_free(ob);
      if (!nounmount && mounted)
        AH_Medium_Unmount(m, 1);
      return rv;
    }
  }

  AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelNotice,
                         I18N("Keys sent"));

  AH_Outbox_free(ob);
  if (!nounmount && mounted)
    AH_Medium_Unmount(m, 1);

  return 0;
}



int AH_Provider_GetCert(AB_PROVIDER *pro, AB_USER *u, int nounmount) {
  AB_BANKING *ab;
  AH_HBCI *h;
  int rv;
  AH_PROVIDER *hp;
  int alwaysAskForCert;
  AH_DIALOG *dialog;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  dialog=AH_Dialog_new(u);
  assert(dialog);

  AH_HBCI_RemoveAllBankCerts(h, u);
  alwaysAskForCert=AB_Banking_GetAlwaysAskForCert(ab);
  AB_Banking_SetAlwaysAskForCert(ab, 1);

  rv=AH_Dialog_Connect(dialog, 30);
  AH_Dialog_Disconnect(dialog, 2);
  AH_Dialog_free(dialog);
  AB_Banking_SetAlwaysAskForCert(ab, alwaysAskForCert);
  if (rv) {
    DBG_ERROR(0, "Could not connect to server (%d)", rv);
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Could not connect to server"));
    return rv;
  }

  return 0;
}



int AH_Provider_GetIniLetterTxt(AB_PROVIDER *pro,
                                AB_USER *u,
                                int useBankKey,
                                GWEN_BUFFER *lbuf,
                                int nounmount) {
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_MEDIUM *m;
  GWEN_CRYPTKEY *key;
  GWEN_DB_NODE *dbKey;
  const void *p;
  unsigned int l;
  GWEN_BUFFER *bbuf;
  GWEN_BUFFER *keybuf;
  int i;
  GWEN_TIME *ti;
  char numbuf[32];
  char hashbuffer[21];
  int rv;
  AH_PROVIDER *hp;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  m=AH_User_GetMedium(u);
  assert(m);

  rv=AH_Medium_Mount(m);
  if (rv) {
    DBG_ERROR(0, "Could not mount medium (%d)", rv);
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Could not mount medium"));
    return rv;
  }

  rv=AH_Medium_SelectContext(m, AH_User_GetContextIdx(u));
  if (rv) {
    DBG_ERROR(0, "Could not select context %d (%d)",
              AH_User_GetContextIdx(u), rv);
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Could not select context"));
  }

  if (useBankKey) {
    key=AH_Medium_GetPubSignKey(m);
    if (!key)
      key=AH_Medium_GetPubCryptKey(m);
    if (!key) {
      if (!nounmount)
        AH_Medium_Unmount(m, 1);
      DBG_ERROR(0, "Server keys missing, please get them first");
      AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                             I18N("Server keys missing, "
                                  "please get them first"));
      return AB_ERROR_NOT_FOUND;
    }
  }
  else {
    GWEN_CRYPTKEY *cryptKey;
    GWEN_CRYPTKEY *signKey;

    signKey=AH_Medium_GetLocalPubSignKey(m);
    cryptKey=AH_Medium_GetLocalPubCryptKey(m);
    if (!signKey || !cryptKey) {
      if (!nounmount)
        AH_Medium_Unmount(m, 1);
      DBG_ERROR(0, "Keys missing, please create them first");
      AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                             I18N("Keys missing, please create them first"));
      return AB_ERROR_NOT_FOUND;
    }
    key=signKey;
    GWEN_CryptKey_free(cryptKey);
  }

  dbKey=GWEN_DB_Group_new("keydata");
  if (GWEN_CryptKey_toDb(key, dbKey, 1)) {
    GWEN_DB_Group_free(dbKey);
    GWEN_CryptKey_free(key);
    DBG_ERROR(0, "Bad key.");
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Bad key"));
    return AB_ERROR_BAD_DATA;
  }
  GWEN_CryptKey_free(key); key=0;

  keybuf=GWEN_Buffer_new(0, 257, 0, 1);

  /* prelude */
  GWEN_Buffer_AppendString(lbuf,
                           I18N("\n\n\nINI-Letter\n\n"));
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Date           : "));
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, I18N("YYYY/MM/DD"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Time           : "));
  GWEN_Time_toString(ti, I18N("hh:mm:ss"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");

  if (useBankKey) {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Bank           : "));
    GWEN_Buffer_AppendString(lbuf, AB_User_GetBankCode(u));
  }
  else {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("User           : "));
    GWEN_Buffer_AppendString(lbuf, AB_User_GetUserId(u));
  }

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Key number     : "));
  snprintf(numbuf, sizeof(numbuf), "%d",
           GWEN_DB_GetIntValue(dbKey, "number", 0, 0));
  GWEN_Buffer_AppendString(lbuf, numbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");

  GWEN_Buffer_AppendString(lbuf,
                           I18N("Key version    : "));
  snprintf(numbuf, sizeof(numbuf), "%d",
           GWEN_DB_GetIntValue(dbKey, "version", 0, 0));
  GWEN_Buffer_AppendString(lbuf, numbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Customer system: "));
    GWEN_Buffer_AppendString(lbuf, AH_HBCI_GetProductName(h));
    GWEN_Buffer_AppendString(lbuf, "\n");
  }

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Public key for electronic signature"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");

  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Exponent"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");

  /* exponent */
  p=GWEN_DB_GetBinValue(dbKey,
                        "data/e",
                        0,
                        0,0,
                        &l);
  if (!p || !l) {
    GWEN_DB_Group_free(dbKey);
    DBG_ERROR(0, "Bad key.");
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Bad key"));
    return AB_ERROR_BAD_DATA;
  }

  bbuf=GWEN_Buffer_new(0, 97, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  if (l<96)
    GWEN_Buffer_FillLeftWithBytes(bbuf, 0, 96-l);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<6; i++) {
    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
      DBG_ERROR(0, "Error converting to hex??");
      abort();
    }
    p+=16;
    GWEN_Buffer_AppendString(lbuf, "\n");
  }

  GWEN_Buffer_FillWithBytes(keybuf, 0, 128-l);
  GWEN_Buffer_AppendBuffer(keybuf, bbuf);
  GWEN_Buffer_free(bbuf);

  /* modulus */
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Modulus"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");
  p=GWEN_DB_GetBinValue(dbKey,
                        "data/n",
                        0,
                        0,0,
                        &l);
  if (!p || !l) {
    GWEN_DB_Group_free(dbKey);
    DBG_ERROR(0, "Bad key.");
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Bad key"));
    return AB_ERROR_BAD_DATA;
  }

  bbuf=GWEN_Buffer_new(0, 97, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  if (l<96)
    GWEN_Buffer_FillLeftWithBytes(bbuf, 0, 96-l);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<6; i++) {
    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
      DBG_ERROR(0, "Error converting to hex??");
      abort();
    }
    p+=16;
    GWEN_Buffer_AppendString(lbuf, "\n");
  }

  GWEN_Buffer_FillWithBytes(keybuf, 0, 128-l);
  GWEN_Buffer_AppendBuffer(keybuf, bbuf);
  GWEN_Buffer_free(bbuf);

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Hash"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");
  l=20;
  if (GWEN_MD_Hash("RMD160",
                   GWEN_Buffer_GetStart(keybuf),
                   GWEN_Buffer_GetUsedBytes(keybuf),
                   hashbuffer,
                   &l)) {
    DBG_ERROR(0, "Could not hash");
    abort();
  }
  GWEN_Buffer_free(keybuf);

  GWEN_Buffer_AppendString(lbuf, "  ");
  if (GWEN_Text_ToHexBuffer(hashbuffer, 20, lbuf, 2, ' ', 0)) {
    DBG_ERROR(0, "Error converting to hex??");
    abort();
  }
  GWEN_Buffer_AppendString(lbuf, "\n");

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "\n\n");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("I confirm that I found the above key "
                                  "for my electronic signature.\n"));
    GWEN_Buffer_AppendString(lbuf, "\n\n");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("____________________________  "
                                  "____________________________\n"
                                  "Place, date                   "
                                  "Signature\n"));
  }

  return 0;
}



int AH_Provider_GetIniLetterHtml(AB_PROVIDER *pro,
                                 AB_USER *u,
                                 int useBankKey,
                                 GWEN_BUFFER *lbuf,
                                 int nounmount) {
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_MEDIUM *m;
  GWEN_CRYPTKEY *key;
  GWEN_DB_NODE *dbKey;
  const void *p;
  unsigned int l;
  GWEN_BUFFER *bbuf;
  GWEN_BUFFER *keybuf;
  int i;
  GWEN_TIME *ti;
  char numbuf[32];
  char hashbuffer[21];
  int rv;
  AH_PROVIDER *hp;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  m=AH_User_GetMedium(u);
  assert(m);

  rv=AH_Medium_Mount(m);
  if (rv) {
    DBG_ERROR(0, "Could not mount medium (%d)", rv);
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Could not mount medium"));
    return rv;
  }

  rv=AH_Medium_SelectContext(m, AH_User_GetContextIdx(u));
  if (rv) {
    DBG_ERROR(0, "Could not select context %d (%d)",
              AH_User_GetContextIdx(u), rv);
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Could not select context"));
  }

  if (useBankKey) {
    key=AH_Medium_GetPubSignKey(m);
    if (!key)
      key=AH_Medium_GetPubCryptKey(m);
    if (!key) {
      if (!nounmount)
        AH_Medium_Unmount(m, 1);
      DBG_ERROR(0, "Server keys missing, please get them first");
      AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                             I18N("Server keys missing, "
                                  "please get them first"));
      return AB_ERROR_NOT_FOUND;
    }
  }
  else {
    GWEN_CRYPTKEY *cryptKey;
    GWEN_CRYPTKEY *signKey;

    signKey=AH_Medium_GetLocalPubSignKey(m);
    cryptKey=AH_Medium_GetLocalPubCryptKey(m);
    if (!signKey || !cryptKey) {
      if (!nounmount)
        AH_Medium_Unmount(m, 1);
      DBG_ERROR(0, "Keys missing, please create them first");
      AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                             I18N("Keys missing, please create them first"));
      return AB_ERROR_NOT_FOUND;
    }
    key=signKey;
    GWEN_CryptKey_free(cryptKey);
  }

  dbKey=GWEN_DB_Group_new("keydata");
  if (GWEN_CryptKey_toDb(key, dbKey, 1)) {
    GWEN_DB_Group_free(dbKey);
    GWEN_CryptKey_free(key);
    DBG_ERROR(0, "Bad key.");
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Bad key"));
    return AB_ERROR_BAD_DATA;
  }
  GWEN_CryptKey_free(key); key=0;

  keybuf=GWEN_Buffer_new(0, 257, 0, 1);

  /* prelude */
  GWEN_Buffer_AppendString(lbuf, "<h3>");
  GWEN_Buffer_AppendString(lbuf, I18N("INI-Letter"));
  GWEN_Buffer_AppendString(lbuf, "</h3>\n");
  GWEN_Buffer_AppendString(lbuf, "<table>\n");
  GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
  GWEN_Buffer_AppendString(lbuf, I18N("Date"));
  GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, I18N("YYYY/MM/DD"), lbuf);
  GWEN_Buffer_AppendString(lbuf, I18N("Time"));
  GWEN_Time_toString(ti, I18N("hh:mm:ss"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");

  if (useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("Bank"));
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, AB_User_GetBankCode(u));
    GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");
  }
  else {
    GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("User"));
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, AB_User_GetUserId(u));
    GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");
  }

  GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
  GWEN_Buffer_AppendString(lbuf, I18N("Key number"));
  GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
  snprintf(numbuf, sizeof(numbuf), "%d",
           GWEN_DB_GetIntValue(dbKey, "number", 0, 0));
  GWEN_Buffer_AppendString(lbuf, numbuf);
  GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");

  GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
  GWEN_Buffer_AppendString(lbuf, I18N("Key version"));
  GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
  snprintf(numbuf, sizeof(numbuf), "%d",
           GWEN_DB_GetIntValue(dbKey, "version", 0, 0));
  GWEN_Buffer_AppendString(lbuf, numbuf);
  GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("Customer system"));
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, AH_HBCI_GetProductName(h));
    GWEN_Buffer_AppendString(lbuf, "</td></tr>\n");
  }
  GWEN_Buffer_AppendString(lbuf, "</table>\n");

  GWEN_Buffer_AppendString(lbuf, "<h3>");
  GWEN_Buffer_AppendString(lbuf, I18N("Public key for electronic signature"));
  GWEN_Buffer_AppendString(lbuf, "</h3>\n");

  GWEN_Buffer_AppendString(lbuf, "<h4>");
  GWEN_Buffer_AppendString(lbuf, I18N("Exponent"));
  GWEN_Buffer_AppendString(lbuf, "</h4>\n");

  /* exponent */
  p=GWEN_DB_GetBinValue(dbKey,
                        "data/e",
                        0,
                        0,0,
                        &l);
  if (!p || !l) {
    GWEN_DB_Group_free(dbKey);
    DBG_ERROR(0, "Bad key.");
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Bad key"));
    return AB_ERROR_BAD_DATA;
  }

  GWEN_Buffer_AppendString(lbuf, "<font face=fixed>\n");
  bbuf=GWEN_Buffer_new(0, 97, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  if (l<96)
    GWEN_Buffer_FillLeftWithBytes(bbuf, 0, 96-l);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<6; i++) {
    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
      DBG_ERROR(0, "Error converting to hex??");
      abort();
    }
    p+=16;
    GWEN_Buffer_AppendString(lbuf, "\n");
  }

  GWEN_Buffer_FillWithBytes(keybuf, 0, 128-l);
  GWEN_Buffer_AppendBuffer(keybuf, bbuf);
  GWEN_Buffer_free(bbuf);
  GWEN_Buffer_AppendString(lbuf, "</font>\n");
  GWEN_Buffer_AppendString(lbuf, "<br>\n");

  /* modulus */
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "<h4>");
  GWEN_Buffer_AppendString(lbuf, I18N("Modulus"));
  GWEN_Buffer_AppendString(lbuf, "</h4>\n");
  p=GWEN_DB_GetBinValue(dbKey,
                        "data/n",
                        0,
                        0,0,
                        &l);
  if (!p || !l) {
    GWEN_DB_Group_free(dbKey);
    DBG_ERROR(0, "Bad key.");
    AB_Banking_ProgressLog(ab, 0, AB_Banking_LogLevelError,
                           I18N("Bad key"));
    return AB_ERROR_BAD_DATA;
  }

  GWEN_Buffer_AppendString(lbuf, "<font face=fixed>\n");
  bbuf=GWEN_Buffer_new(0, 97, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  if (l<96)
    GWEN_Buffer_FillLeftWithBytes(bbuf, 0, 96-l);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<6; i++) {
    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
      DBG_ERROR(0, "Error converting to hex??");
      abort();
    }
    p+=16;
    GWEN_Buffer_AppendString(lbuf, "\n");
  }

  GWEN_Buffer_FillWithBytes(keybuf, 0, 128-l);
  GWEN_Buffer_AppendBuffer(keybuf, bbuf);
  GWEN_Buffer_free(bbuf);
  GWEN_Buffer_AppendString(lbuf, "</font>\n");
  GWEN_Buffer_AppendString(lbuf, "<br>\n");

  GWEN_Buffer_AppendString(lbuf, "<h4>");
  GWEN_Buffer_AppendString(lbuf, I18N("Hash"));
  GWEN_Buffer_AppendString(lbuf, "</h4>\n");
  GWEN_Buffer_AppendString(lbuf, "<font face=fixed>\n");
  l=20;
  if (GWEN_MD_Hash("RMD160",
                   GWEN_Buffer_GetStart(keybuf),
                   GWEN_Buffer_GetUsedBytes(keybuf),
                   hashbuffer,
                   &l)) {
    DBG_ERROR(0, "Could not hash");
    abort();
  }
  GWEN_Buffer_free(keybuf);

  GWEN_Buffer_AppendString(lbuf, "  ");
  if (GWEN_Text_ToHexBuffer(hashbuffer, 20, lbuf, 2, ' ', 0)) {
    DBG_ERROR(0, "Error converting to hex??");
    abort();
  }
  GWEN_Buffer_AppendString(lbuf, "</font>\n");
  GWEN_Buffer_AppendString(lbuf, "<br>\n");

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "<br><br>\n");
    GWEN_Buffer_AppendString(lbuf,
                             I18N("I confirm that I found the above key "
                                  "for my electronic signature.\n"));
    GWEN_Buffer_AppendString(lbuf, "<br><br>\n");
    GWEN_Buffer_AppendString(lbuf, "<table>\n");
    GWEN_Buffer_AppendString(lbuf, "<tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, "____________________________  ");
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, "____________________________  ");
    GWEN_Buffer_AppendString(lbuf, "</td></tr><tr><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("Place, date"));
    GWEN_Buffer_AppendString(lbuf, "</td><td>\n");
    GWEN_Buffer_AppendString(lbuf, I18N("Signature"));
    GWEN_Buffer_AppendString(lbuf, "</td></tr></table>\n");
    GWEN_Buffer_AppendString(lbuf, "<br>\n");
  }

  return 0;
}









