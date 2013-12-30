/***************************************************************************
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
#include "aqebics_l.h"
#include "user_l.h"
#include "account_l.h"
#include "msg/xml.h"
#include "msg/keys.h"
#include "dialogs/dlg_edituser_l.h"
#include "dialogs/dlg_newkeyfile_l.h"

#include <aqbanking/provider_be.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/account_be.h>
#include <aqbanking/job_be.h>
#include <aqbanking/jobgetbalance_be.h>
#include <aqbanking/jobgettransactions_be.h>
#include <aqbanking/jobsingletransfer_be.h>
#include <aqbanking/jobsingledebitnote_be.h>
#include <aqbanking/value.h>
#include <aqbanking/httpsession.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/ct.h>
#include <gwenhywfar/gui.h>

#include <time.h>


GWEN_INHERIT(AB_PROVIDER, EBC_PROVIDER)



AB_PROVIDER *EBC_Provider_new(AB_BANKING *ab){
  AB_PROVIDER *pro;
  EBC_PROVIDER *dp;

  pro=AB_Provider_new(ab, "aqebics");
  GWEN_NEW_OBJECT(EBC_PROVIDER, dp);
  GWEN_INHERIT_SETDATA(AB_PROVIDER, EBC_PROVIDER, pro, dp,
                       EBC_Provider_FreeData);
  dp->bankingJobs=AB_Job_List2_new();
  dp->queue=EBC_Queue_new();

  AB_Provider_SetInitFn(pro, EBC_Provider_Init);
  AB_Provider_SetFiniFn(pro, EBC_Provider_Fini);
  AB_Provider_SetUpdateJobFn(pro, EBC_Provider_UpdateJob);
  AB_Provider_SetAddJobFn(pro, EBC_Provider_AddJob);
  AB_Provider_SetExecuteFn(pro, EBC_Provider_Execute);
  AB_Provider_SetResetQueueFn(pro, EBC_Provider_ResetQueue);
  AB_Provider_SetExtendUserFn(pro, EBC_Provider_ExtendUser);
  AB_Provider_SetExtendAccountFn(pro, EBC_Provider_ExtendAccount);

  AB_Provider_SetGetEditUserDialogFn(pro, EBC_Provider_GetEditUserDialog);
  AB_Provider_AddFlags(pro, AB_PROVIDER_FLAGS_HAS_EDITUSER_DIALOG);

  AB_Provider_SetGetNewUserDialogFn(pro, EBC_Provider_GetNewUserDialog);
  AB_Provider_AddFlags(pro, AB_PROVIDER_FLAGS_HAS_NEWUSER_DIALOG);


  return pro;
}



void GWENHYWFAR_CB EBC_Provider_FreeData(GWEN_UNUSED void *bp, void *p) {
  EBC_PROVIDER *dp;

  dp=(EBC_PROVIDER*)p;
  assert(dp);

  EBC_Queue_free(dp->queue);
  AB_Job_List2_free(dp->bankingJobs);

  GWEN_FREE_OBJECT(dp);
}



int EBC_Provider_GetConnectTimeout(const AB_PROVIDER *pro) {
  EBC_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  return dp->connectTimeout;
}



int EBC_Provider_GetTransferTimeout(const AB_PROVIDER *pro) {
  EBC_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  return dp->transferTimeout;
}



int EBC_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData) {
  EBC_PROVIDER *dp;
  const char *logLevelName;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  if (!GWEN_Logger_IsOpen(AQEBICS_LOGDOMAIN)) {
    GWEN_Logger_Open(AQEBICS_LOGDOMAIN,
		     "aqebics", 0,
		     GWEN_LoggerType_Console,
		     GWEN_LoggerFacility_User);
  }

  logLevelName=getenv("AQEBICS_LOGLEVEL");
  if (logLevelName) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(logLevelName);
    if (ll!=GWEN_LoggerLevel_Unknown) {
      GWEN_Logger_SetLevel(AQEBICS_LOGDOMAIN, ll);
      DBG_WARN(AQEBICS_LOGDOMAIN,
               "Overriding loglevel for AqEBICS with \"%s\"",
               logLevelName);
    }
    else {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Unknown loglevel \"%s\"",
                logLevelName);
    }
  }

  DBG_INFO(AQEBICS_LOGDOMAIN, "Please remember to purchase a license if you want to use the EBICS backend.");


  if (1) {
    GWEN_STRINGLIST *sl=GWEN_PathManager_GetPaths(AB_PM_LIBNAME,
						  AB_PM_LOCALEDIR);
    const char *localedir=GWEN_StringList_FirstString(sl);
    int rv;

    rv=GWEN_I18N_BindTextDomain_Dir(PACKAGE, localedir);
    if (rv) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not bind textdomain (%d)", rv);
    }
    else {
      rv=GWEN_I18N_BindTextDomain_Codeset(PACKAGE, "UTF-8");
      if (rv) {
	DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not set codeset (%d)", rv);
      }
    }

    GWEN_StringList_free(sl);
  }

  DBG_NOTICE(AQEBICS_LOGDOMAIN, "Initializing AqEBICS backend");
  dp->connectTimeout=GWEN_DB_GetIntValue(dbData, "connectTimeout", 0,
                                        EBC_DEFAULT_CONNECT_TIMEOUT);
  dp->transferTimeout=GWEN_DB_GetIntValue(dbData, "transferTimeout", 0,
                                          EBC_DEFAULT_TRANSFER_TIMEOUT);

  return 0;
}



int EBC_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData) {
  EBC_PROVIDER *dp;
  uint32_t currentVersion;

  DBG_NOTICE(AQEBICS_LOGDOMAIN, "Deinitializing AqEBICS backend");

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  currentVersion=
    (AQEBICS_VERSION_MAJOR<<24) |
    (AQEBICS_VERSION_MINOR<<16) |
    (AQEBICS_VERSION_PATCHLEVEL<<8) |
    AQEBICS_VERSION_BUILD;

  /* save configuration */
  DBG_NOTICE(AQEBICS_LOGDOMAIN, "Setting version %08x",
             currentVersion);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "lastVersion", currentVersion);

  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "connectTimeout", dp->connectTimeout);
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "transferTimeout", dp->transferTimeout);

  return 0;
}



int EBC_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j) {
  EBC_PROVIDER *dp;
  AB_ACCOUNT *a;
  AB_USER *u;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  a=AB_Job_GetAccount(j);
  assert(a);

  u=AB_Account_GetFirstUser(a);
  if (u==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No user assigned to this account.");
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("No user assigned to this account."));
    GWEN_Gui_ShowError(I18N("Setup Error"),
		       I18N("No user assigned to this account. Please assign one in the online banking setup dialog "
			    "for this account.\n"));
    return GWEN_ERROR_INTERNAL;
  }

  switch(AB_Job_GetType(j)) {
  case AB_Job_TypeTransfer: {
    AB_TRANSACTION_LIMITS *lim;

    lim=AB_TransactionLimits_new();
    AB_TransactionLimits_SetMaxLenPurpose(lim, 27);
    AB_TransactionLimits_SetMaxLenRemoteName(lim, 27);
    AB_TransactionLimits_SetMaxLinesRemoteName(lim, 1);
    AB_TransactionLimits_SetMaxLinesPurpose(lim, 2);

    AB_TransactionLimits_AddValuesTextKey(lim, "51", 0);
    AB_Job_SetFieldLimits(j, lim);
    AB_TransactionLimits_free(lim);

    break;
  }

  case AB_Job_TypeDebitNote: {
    AB_TRANSACTION_LIMITS *lim;

    lim=AB_TransactionLimits_new();
    AB_TransactionLimits_SetMaxLenPurpose(lim, 27);
    AB_TransactionLimits_SetMaxLenRemoteName(lim, 27);
    AB_TransactionLimits_SetMaxLinesRemoteName(lim, 1);
    AB_TransactionLimits_SetMaxLinesPurpose(lim, 2);

    AB_TransactionLimits_AddValuesTextKey(lim, "05", 0);
    AB_Job_SetFieldLimits(j, lim);
    AB_TransactionLimits_free(lim);

    break;
  }

  case AB_Job_TypeGetTransactions:
    break;

  case AB_Job_TypeGetBalance:
  default:
    DBG_INFO(AQEBICS_LOGDOMAIN,
	     "Job not yet supported (%d)",
	     AB_Job_GetType(j));
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */
  return 0;
}



int EBC_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j) {
  EBC_PROVIDER *dp;
  AB_ACCOUNT *a;
  AB_USER *u;
  EBC_USERQUEUE *uq;
  int doAdd=1;
  GWEN_DB_NODE *dbJob;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  a=AB_Job_GetAccount(j);
  assert(a);

  u=AB_Account_GetFirstUser(a);
  if (u==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No user assigned to account.");
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 I18N("No user assigned to account."));
    GWEN_Gui_ShowError(I18N("Setup Error"),
		       I18N("No user assigned to this account. Please assign one in the online banking setup dialog "
			    "for this account.\n"));
    return GWEN_ERROR_INTERNAL;
  }

  dbJob=AB_Job_GetProviderData(j, pro);
  assert(dbJob);

  switch(AB_Job_GetType(j)) {
  case AB_Job_TypeGetTransactions:
  case AB_Job_TypeTransfer:
  case AB_Job_TypeDebitNote:
    break;
  case AB_Job_TypeGetBalance:
  default:
    DBG_INFO(AQEBICS_LOGDOMAIN,
	     "Job not yet supported (%d)",
	     AB_Job_GetType(j));
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */

  uq=EBC_Queue_GetUserQueue(dp->queue, u);
  assert(uq);

  if (AB_Job_GetType(j)==AB_Job_TypeGetTransactions) {
    AB_JOB *firstJob;

    firstJob=EBC_Queue_FindFirstJobLikeThis(dp->queue, u, j);
    if (firstJob) {
      GWEN_DB_NODE *dbCurrJob;

      /* this job is just a copy of the firstJob, reference it */
      dbCurrJob=AB_Job_GetProviderData(j, pro);
      assert(dbCurrJob);

      GWEN_DB_SetIntValue(dbCurrJob,
			  GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "refJob",
			  AB_Job_GetJobId(firstJob));
      /* don't add to queues */
      doAdd=0;
    }
  }

  if (doAdd) {
    /* only add to queue if needed */
    EBC_UserQueue_AddJob(uq, j);
  }

  /* always add to linear list */
  AB_Job_List2_PushBack(dp->bankingJobs, j);
  return 0;
}



GWEN_DIALOG *EBC_Provider_GetEditUserDialog(AB_PROVIDER *pro, AB_USER *u) {
  EBC_PROVIDER *xp;
  GWEN_DIALOG *dlg;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(xp);

  dlg=EBC_EditUserDialog_new(AB_Provider_GetBanking(pro), u, 1);
  if (dlg==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



GWEN_DIALOG *EBC_Provider_GetNewUserDialog(AB_PROVIDER *pro, int i) {
  EBC_PROVIDER *xp;
  GWEN_DIALOG *dlg;

  assert(pro);
  xp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(xp);

  dlg=EBC_NewKeyFileDialog_new(AB_Provider_GetBanking(pro));
  if (dlg==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (no dialog)");
    return NULL;
  }

  return dlg;
}



int EBC_Provider_CountDoneJobs(AB_JOB_LIST2 *jl){
  AB_JOB_LIST2_ITERATOR *jit;
  int cnt=0;

  jit=AB_Job_List2_First(jl);
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while(uj) {
      AB_JOB_STATUS js;

      js=AB_Job_GetStatus(uj);
      if (js==AB_Job_StatusFinished ||
	  js==AB_Job_StatusError)
	cnt++;
      uj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  return cnt;
}



int EBC_Provider_ExecContext_STA(AB_PROVIDER *pro,
				 AB_IMEXPORTER_CONTEXT *ctx,
				 GWEN_UNUSED AB_USER *u,
				 GWEN_UNUSED AB_ACCOUNT *a,
				 GWEN_HTTP_SESSION *sess,
				 EBC_CONTEXT *ectx){
  EBC_PROVIDER *dp;
  int errors=0;
  int oks=0;
  AB_JOB_LIST2_ITERATOR *jit;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  jit=AB_Job_List2_First(EBC_Context_GetJobs(ectx));
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while(uj) {
      int rv;

      /* exchange STA request */
      rv=EBC_Provider_XchgStaRequest(pro, sess,
				     AB_JobGetTransactions_GetFromTime(uj),
				     AB_JobGetTransactions_GetToTime(uj),
				     ctx);
      if (!rv) {
	oks++;
        AB_Job_SetStatus(uj, AB_Job_StatusFinished);
      }
      else {
        if (rv==GWEN_ERROR_NO_DATA)
	  AB_Job_SetStatus(uj, AB_Job_StatusFinished);
	else {
	  AB_Job_SetStatus(uj, AB_Job_StatusError);
	  if (rv==GWEN_ERROR_USER_ABORTED) {
	    DBG_INFO(AQEBICS_LOGDOMAIN, "User aborted");
	    AB_Job_List2Iterator_free(jit);
	    return rv;
	  }
	  errors++;
	}
      }
      rv=GWEN_Gui_ProgressAdvance(0, EBC_Provider_CountDoneJobs(dp->bankingJobs));
      if (rv==GWEN_ERROR_USER_ABORTED) {
	DBG_INFO(AQEBICS_LOGDOMAIN, "User aborted");
	AB_Job_List2Iterator_free(jit);
	return rv;
      }
      uj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  return 0;
}



void EBC_Provider_SetJobListStatus(AB_JOB_LIST2 *jl, AB_JOB_STATUS js) {
  AB_JOB_LIST2_ITERATOR *jit;

  jit=AB_Job_List2_First(jl);
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while(uj) {
      AB_Job_SetStatus(uj, js);
      uj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }
}



int EBC_Provider_ExecContext__IZV(AB_PROVIDER *pro,
				  AB_IMEXPORTER_CONTEXT *ctx,
				  AB_USER *u,
				  AB_ACCOUNT *a,
				  GWEN_HTTP_SESSION *sess,
				  EBC_CONTEXT *ectx){
  EBC_PROVIDER *dp;
  AB_JOB_LIST2_ITERATOR *jit;
  AB_JOB_STATUS js;
  AB_IMEXPORTER_CONTEXT *exCtx;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GWEN_BUFFER *bufDtaus;
  GWEN_TIME *currentTime;
  GWEN_BUFFER *logbuf;
  int rv;
  const char *profileName=NULL;
  const char *s;
  const char *rqType;
  uint32_t groupId=0;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* prepare CTX log */
  logbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(logbuf, "BEGIN");
  currentTime=GWEN_CurrentTime();
  GWEN_Time_toString(currentTime, I18N("YYYY/MM/DD-hh:mm:ss"), logbuf);
  GWEN_Time_free(currentTime);
  GWEN_Buffer_AppendString(logbuf, "\t");
  GWEN_Buffer_AppendString(logbuf, I18N("National Mass Transfer"));
  GWEN_Buffer_AppendString(logbuf, "\n");
  GWEN_Buffer_AppendString(logbuf, "\t");
  GWEN_Buffer_AppendString(logbuf, I18N("Transfer type: "));
  GWEN_Buffer_AppendString(logbuf, "\t");

  switch(EBC_Context_GetJobType(ectx)) {
  case AB_Job_TypeTransfer:
    if (!profileName)
      profileName="transfer";
    GWEN_Buffer_AppendString(logbuf, I18N("Transfer"));
    break;
  case AB_Job_TypeDebitNote:
    if (!profileName)
      profileName="debitnote";
    GWEN_Buffer_AppendString(logbuf, I18N("Debit Note"));
    break;
  default:
    GWEN_Buffer_AppendString(logbuf, I18N("unknown"));
    break;
  }
  GWEN_Buffer_AppendString(logbuf, "\n");

  GWEN_Buffer_AppendString(logbuf, "\t");
  GWEN_Buffer_AppendString(logbuf, I18N("Account: "));
  GWEN_Buffer_AppendString(logbuf, "\t");
  GWEN_Buffer_AppendString(logbuf, AB_Account_GetBankCode(a));
  GWEN_Buffer_AppendString(logbuf, " / ");
  GWEN_Buffer_AppendString(logbuf, AB_Account_GetAccountNumber(a));
  GWEN_Buffer_AppendString(logbuf, "\n");
  /* add a tab-less line to start a new table */
  GWEN_Buffer_AppendString(logbuf, "Transactions\n");

  DBG_INFO(AQEBICS_LOGDOMAIN, "Sampling transactions from jobs");
  exCtx=AB_ImExporterContext_new();
  ai=AB_ImExporterAccountInfo_new();
  AB_ImExporterAccountInfo_FillFromAccount(ai, a);

  jit=AB_Job_List2_First(EBC_Context_GetJobs(ectx));
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while(uj) {
      AB_TRANSACTION *t;
      const GWEN_STRINGLIST *sl;
      const char *s;
      const AB_VALUE *v;

      switch(EBC_Context_GetJobType(ectx)) {
      case AB_Job_TypeTransfer:
      case AB_Job_TypeDebitNote:
	t=AB_Job_GetTransaction(uj);
        break;
      default:
        t=NULL;
      }
      assert(t);

      if (groupId==0)
	/* take id from first job of the created DTAUS doc */
	groupId=AB_Job_GetJobId(uj);
      AB_Transaction_SetGroupId(t, groupId);

      AB_ImExporterAccountInfo_AddTransaction(ai, AB_Transaction_dup(t));
      sl=AB_Transaction_GetRemoteName(t);
      s=NULL;
      if (sl)
	s=GWEN_StringList_FirstString(sl);
      if (!s)
	s=I18N("unknown");
      GWEN_Buffer_AppendString(logbuf, s);
      GWEN_Buffer_AppendString(logbuf, "\t");
      s=AB_Transaction_GetRemoteBankCode(t);
      if (!s)
	s="????????";
      GWEN_Buffer_AppendString(logbuf, s);
      GWEN_Buffer_AppendString(logbuf, "\t");
      s=AB_Transaction_GetRemoteAccountNumber(t);
      if (!s)
	s="??????????";
      GWEN_Buffer_AppendString(logbuf, s);
      GWEN_Buffer_AppendString(logbuf, "\t");
      sl=AB_Transaction_GetPurpose(t);
      s=NULL;
      if (sl)
	s=GWEN_StringList_FirstString(sl);
      if (!s)
	s="";
      GWEN_Buffer_AppendString(logbuf, s);
      GWEN_Buffer_AppendString(logbuf, "\t");
      v=AB_Transaction_GetValue(t);
      if (v)
	AB_Value_toHumanReadableString(v, logbuf, 2);
      else
	GWEN_Buffer_AppendString(logbuf, "0,00 EUR");
      GWEN_Buffer_AppendString(logbuf, "\n");

      uj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }
  AB_ImExporterContext_AddAccountInfo(exCtx, ai);

  GWEN_Buffer_AppendString(logbuf, I18N("Results:\n"));

  /* export as DTAUS to bufDtaus */
  bufDtaus=GWEN_Buffer_new(0, 1024, 0, 1);

  DBG_INFO(AQEBICS_LOGDOMAIN, "Exporting transactions to DTAUS[default]");
  rv=AB_Banking_ExportToBuffer(AB_Provider_GetBanking(pro),
			       exCtx,
			       "dtaus",
			       profileName,
			       bufDtaus);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufDtaus);
    EBC_Provider_SetJobListStatus(EBC_Context_GetJobs(ectx),
				  AB_Job_StatusError);
    GWEN_Buffer_AppendString(logbuf, "\t");
    GWEN_Buffer_AppendString(logbuf, I18N("Error while exporting to DTAUS\n"));
    GWEN_Buffer_AppendString(logbuf, "END\n");

    AB_ImExporterContext_AddLog(ctx, GWEN_Buffer_GetStart(logbuf));
    GWEN_Buffer_free(logbuf);
    return rv;
  }

  GWEN_Buffer_AppendString(logbuf, "\t");
  GWEN_Buffer_AppendString(logbuf, I18N("Exporting to DTAUS: ok\n"));

  /* exchange upload request */
  DBG_INFO(AQEBICS_LOGDOMAIN, "Uploading.");
  AB_HttpSession_ClearLog(sess);

  if (EBC_Context_GetJobType(ectx)==AB_Job_TypeDebitNote) {
    if (EBC_User_GetFlags(u) & EBC_USER_FLAGS_USE_IZL)
      rqType="IZL";
    else
      rqType="IZV";
  }
  else
    rqType="IZV";
  rv=EBC_Provider_XchgUploadRequest(pro, sess, u, rqType,
				    (const uint8_t*)GWEN_Buffer_GetStart(bufDtaus),
				    GWEN_Buffer_GetUsedBytes(bufDtaus));
  if (rv<0 || rv>=300)
    js=AB_Job_StatusError;
  else
    js=AB_Job_StatusFinished;

  s=AB_HttpSession_GetLog(sess);
  if (s)
    GWEN_Buffer_AppendString(logbuf, s);
  GWEN_Buffer_AppendString(logbuf, "END\n");

  AB_ImExporterContext_AddLog(ctx, GWEN_Buffer_GetStart(logbuf));
  GWEN_Buffer_free(logbuf);

  EBC_Provider_SetJobListStatus(EBC_Context_GetJobs(ectx), js);

  DBG_INFO(AQEBICS_LOGDOMAIN, "Done");
  return 0;
}



int EBC_Provider_ExecContext_IZV(AB_PROVIDER *pro,
				 AB_IMEXPORTER_CONTEXT *ctx,
				 AB_USER *u,
				 AB_ACCOUNT *a,
				 GWEN_HTTP_SESSION *sess,
				 EBC_CONTEXT *ectx){
  EBC_PROVIDER *dp;
  AB_JOB_LIST2_ITERATOR *jit;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  DBG_INFO(AQEBICS_LOGDOMAIN, "Executing IZV request");
  rv=EBC_Provider_ExecContext__IZV(pro, ctx, u, a, sess, ectx);

  jit=AB_Job_List2_First(EBC_Context_GetJobs(ectx));
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while(uj) {
      const AB_TRANSACTION *ot;

      switch(EBC_Context_GetJobType(ectx)) {
      case AB_Job_TypeTransfer:
      case AB_Job_TypeDebitNote:
	ot=AB_Job_GetTransaction(uj);
        break;
      default:
        ot=NULL;
      }

      assert(ot);
      if (ot) {
	AB_TRANSACTION *t;
	AB_TRANSACTION_STATUS tStatus;

	switch(AB_Job_GetStatus(uj)) {
	case AB_Job_StatusFinished: tStatus=AB_Transaction_StatusAccepted; break;
	case AB_Job_StatusPending:  tStatus=AB_Transaction_StatusPending; break;
	default:                    tStatus=AB_Transaction_StatusRejected; break;
	}

	t=AB_Transaction_dup(ot);
	AB_Transaction_SetStatus(t, tStatus);
	AB_ImExporterContext_AddTransfer(ctx, t);
      }

      uj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  return rv;
}



int EBC_Provider_ExecContext(AB_PROVIDER *pro,
			     AB_IMEXPORTER_CONTEXT *ctx,
			     AB_USER *u,
                             AB_ACCOUNT *a,
			     GWEN_HTTP_SESSION *sess,
			     EBC_CONTEXT *ectx){
  EBC_PROVIDER *dp;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  switch(EBC_Context_GetJobType(ectx)) {
  case AB_Job_TypeGetTransactions:
    rv=EBC_Provider_ExecContext_STA(pro, ctx, u, a, sess, ectx);
    break;
  case AB_Job_TypeTransfer:
  case AB_Job_TypeDebitNote:
    rv=EBC_Provider_ExecContext_IZV(pro, ctx, u, a, sess, ectx);
    break;
  case AB_Job_TypeEuTransfer:
  case AB_Job_TypeInternalTransfer:
  default:
      rv=GWEN_ERROR_NOT_IMPLEMENTED;
    break;
  }

  return rv;
}



int EBC_Provider_ExecAccountQueue(AB_PROVIDER *pro,
				  AB_IMEXPORTER_CONTEXT *ctx,
				  AB_USER *u,
				  GWEN_HTTP_SESSION *sess,
				  EBC_ACCOUNTQUEUE *aq){
  EBC_PROVIDER *dp;
  int errors=0;
  int oks=0;
  AB_ACCOUNT *a;
  EBC_CONTEXT_LIST *cl;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  a=EBC_AccountQueue_GetAccount(aq);

  cl=EBC_AccountQueue_GetContextList(aq);
  if (cl) {
    EBC_CONTEXT *ectx;

    ectx=EBC_Context_List_First(cl);
    while(ectx) {
      int rv;

      rv=EBC_Provider_ExecContext(pro, ctx, u, a, sess, ectx);
      if (!rv)
	oks++;
      else
	errors++;

      if (rv==GWEN_ERROR_USER_ABORTED) {
	DBG_INFO(AQEBICS_LOGDOMAIN, "User aborted");
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Error,
			     I18N("User aborted"));
	return rv;
      }

      ectx=EBC_Context_List_Next(ectx);
    }
  }

  return 0;
}



int EBC_Provider_ExecUserQueue(AB_PROVIDER *pro,
			       AB_IMEXPORTER_CONTEXT *ctx,
			       EBC_USERQUEUE *uq){
  EBC_PROVIDER *dp;
  int errors=0;
  int oks=0;
  GWEN_HTTP_SESSION *sess;
  int rv;
  EBC_ACCOUNTQUEUE_LIST *al;
  AB_BANKING *ab;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  /* create and open session */
  sess=EBC_Dialog_new(pro,
		      EBC_UserQueue_GetUser(uq));
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not open session");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  al=EBC_UserQueue_GetAccountQueues(uq);
  if (al) {
    EBC_ACCOUNTQUEUE *aq;

    aq=EBC_AccountQueue_List_First(al);
    while(aq) {
      AB_ACCOUNT *a;

      a=EBC_AccountQueue_GetAccount(aq);
      assert(a);

      /* lock account */
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Info,
			   I18N("Locking account"));
      rv=AB_Banking_BeginExclUseAccount(ab, a);
      if (rv<0) {
        DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Error,
			     I18N("Could not lock account"));
      }
      else {
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Info,
			     I18N("Executing account queue"));
	rv=EBC_Provider_ExecAccountQueue(pro, ctx,
					 EBC_UserQueue_GetUser(uq),
					 sess, aq);
	if (!rv)
	  oks++;
	else {
	  errors++;
	  if (rv==GWEN_ERROR_USER_ABORTED) {
	    DBG_INFO(AQEBICS_LOGDOMAIN, "User aborted");
	    AB_Banking_EndExclUseAccount(ab, a, 1); /* abandon */
	    GWEN_HttpSession_Fini(sess);
	    GWEN_HttpSession_free(sess);
	    return rv;
	  }
	  else {
	    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
	  }
	}

        /* unlock account */
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Info,
			     I18N("Unlocking account"));
	rv=AB_Banking_EndExclUseAccount(ab, a, 0);
	if (rv<0) {
	  DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
	  GWEN_Gui_ProgressLog(0,
			       GWEN_LoggerLevel_Error,
			       I18N("Could not unlock account"));
	}
      }

      aq=EBC_AccountQueue_List_Next(aq);
    }
  }

  /* close and destroy session */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  return 0;
}



AB_JOB *EBC_Provider_FindJobById(AB_JOB_LIST2 *jl, uint32_t jid) {
  AB_JOB_LIST2_ITERATOR *jit;

  jit=AB_Job_List2_First(jl);
  if (jit) {
    AB_JOB *j;

    j=AB_Job_List2Iterator_Data(jit);
    assert(j);
    while(j) {
      if (AB_Job_GetJobId(j)==jid) {
	AB_Job_List2Iterator_free(jit);
        return j;
      }
      j=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  return 0;
}



int EBC_Provider_ExecQueue(AB_PROVIDER *pro,
			   AB_IMEXPORTER_CONTEXT *ctx) {
  EBC_PROVIDER *dp;
  EBC_USERQUEUE_LIST *uql;
  int errors=0;
  int oks=0;
  int rv;
  AB_BANKING *ab;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  uql=EBC_Queue_GetUserQueues(dp->queue);
  if (uql) {
    EBC_USERQUEUE *uq;

    uq=EBC_UserQueue_List_First(uql);
    while(uq) {
      AB_USER *u;

      u=EBC_UserQueue_GetUser(uq);
      assert(u);
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Info,
			   I18N("Locking user"));
      rv=AB_Banking_BeginExclUseUser(ab, u);
      if (rv) {
	DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
	errors++;
      }
      else {
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Info,
			     I18N("Executing user queue"));
	rv=EBC_Provider_ExecUserQueue(pro, ctx, uq);
	if (rv)
	  errors++;
	else
	  oks++;
	if (rv==GWEN_ERROR_USER_ABORTED) {
	  DBG_INFO(AQEBICS_LOGDOMAIN, "User aborted");
	  GWEN_Gui_ProgressLog(0,
			       GWEN_LoggerLevel_Info,
			       I18N("Unlocking user"));
	  AB_Banking_EndExclUseUser(ab, u, 1);
	  return rv;
	}
	rv=AB_Banking_EndExclUseUser(ab, u, 0);
	if (rv<0) {
	  DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
	}
      }
      uq=EBC_UserQueue_List_Next(uq);
    } /* while */
  }

  return 0;
}



int EBC_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx) {
  EBC_PROVIDER *dp;
  int oks=0;
  int errors=0;
  AB_JOB_LIST2_ITERATOR *jit;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  rv=EBC_Provider_ExecQueue(pro, ctx);
  if (!rv)
    oks++;
  else {
    errors++;
    if (rv==GWEN_ERROR_USER_ABORTED) {
      EBC_Queue_Clear(dp->queue);
      AB_Job_List2_Clear(dp->bankingJobs);
      return rv;
    }
  }

  /* set results in referencing jobs, too */
  jit=AB_Job_List2_First(dp->bankingJobs);
  if (jit) {
    AB_JOB *uj;

    uj=AB_Job_List2Iterator_Data(jit);
    assert(uj);
    while(uj) {
      if (AB_Job_GetStatus(uj)==AB_Job_StatusSent) {
	AB_JOB *rj;
	uint32_t rjid;

	rj=uj;
        /* find referenced job (if any) */
	do {
	  GWEN_DB_NODE *dbT;

	  dbT=AB_Job_GetProviderData(rj, pro);
	  assert(dbT);
	  rjid=GWEN_DB_GetIntValue(dbT, "refJob", 0, 0);
	  if (rjid) {
	    rj=EBC_Provider_FindJobById(dp->bankingJobs, rjid);
	  }
	} while(rjid && rj);

	if (rj && rj!=uj) {
	  /* found referenced job, copy status and result text */
	  DBG_INFO(AQEBICS_LOGDOMAIN,
		    "Copying status from referenced job");
	  AB_Job_SetStatus(uj, AB_Job_GetStatus(rj));
	  AB_Job_SetResultText(uj, AB_Job_GetResultText(rj));
	}
	if (AB_Job_GetStatus(uj)==AB_Job_StatusSent)
	  AB_Job_SetStatus(uj, AB_Job_StatusFinished);
      }
      uj=AB_Job_List2Iterator_Next(jit);
    } /* while */
    AB_Job_List2Iterator_free(jit);
  }

  rv=AB_Banking_ExecutionProgress(AB_Provider_GetBanking(pro));
  if (rv==GWEN_ERROR_USER_ABORTED) {
    DBG_INFO(AQEBICS_LOGDOMAIN,
             "User aborted");
    return rv;
  }

  EBC_Queue_Clear(dp->queue);
  AB_Job_List2_Clear(dp->bankingJobs);

  if (!oks && errors) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Not a single job succeeded");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



int EBC_Provider_ResetQueue(AB_PROVIDER *pro) {
  EBC_PROVIDER *dp;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  AB_Job_List2_Clear(dp->bankingJobs);

  return 0;
}



int EBC_Provider_ExtendUser(AB_PROVIDER *pro, AB_USER *u,
			    AB_PROVIDER_EXTEND_MODE em,
			    GWEN_DB_NODE *db) {
  EBC_User_Extend(u, pro, em, db);
  return 0;
}



int EBC_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
			       AB_PROVIDER_EXTEND_MODE em,
			       GWEN_DB_NODE *db) {
  EBC_Account_Extend(a, pro, em, db);
  return 0;
}



int EBC_Provider_MountToken(AB_PROVIDER *pro,
			    AB_USER *u,
			    GWEN_CRYPT_TOKEN **pCt,
			    const GWEN_CRYPT_TOKEN_CONTEXT **pCtx) {
  EBC_PROVIDER *dp;
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* get crypt token of signer */
  rv=AB_Banking_GetCryptToken(AB_Provider_GetBanking(pro),
			      EBC_User_GetTokenType(u),
			      EBC_User_GetTokenName(u),
			      &ct);
  if (rv) {
    DBG_INFO(AQEBICS_LOGDOMAIN,
	     "Could not get crypt token for user \"%s\" (%d)",
	     AB_User_GetUserId(u), rv);
    return rv;
  }

  /* make sure the right flags are set */
  DBG_INFO(AQEBICS_LOGDOMAIN, "Adding mode \"direct sign\" to CryptToken");
  GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_DIRECT_SIGN);

  /* open CryptToken if necessary */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    rv=GWEN_Crypt_Token_Open(ct, 0, 0);
    if (rv) {
      DBG_INFO(AQEBICS_LOGDOMAIN,
	       "Could not open crypt token for user \"%s\" (%d)",
	       AB_User_GetUserId(u), rv);
      return rv;
    }
  }

  /* get context and key info */
  ctx=GWEN_Crypt_Token_GetContext(ct,
				  EBC_User_GetTokenContextId(u),
				  0);
  if (ctx==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN,
	     "Context %d not found on crypt token [%s:%s]",
	     EBC_User_GetTokenContextId(u),
	     GWEN_Crypt_Token_GetTypeName(ct),
	     GWEN_Crypt_Token_GetTokenName(ct));
    return GWEN_ERROR_NOT_FOUND;
  }

  *pCt=ct;
  *pCtx=ctx;

  return 0;
}



int EBC_Provider_GenerateNonce(GWEN_UNUSED AB_PROVIDER *pro, GWEN_BUFFER *buf) {
  int rv;
  uint8_t rbuf[16];

  GWEN_Crypt_Random(2, rbuf, sizeof(rbuf));
  rv=GWEN_Text_ToHexBuffer((const char*)rbuf, sizeof(rbuf), buf, 0, 0, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
              "Could not convert NONCE to hex (%d)", rv);
    return rv;
  }

  DBG_DEBUG(AQEBICS_LOGDOMAIN,
	    "Generated NONCE [%s]",
	    GWEN_Buffer_GetStart(buf));

  return 0;
}



int EBC_Provider_GenerateTimeStamp(GWEN_UNUSED AB_PROVIDER *pro,
				   AB_USER *u,
				   GWEN_BUFFER *buf) {
  char timestamp[40];
  time_t ti;
  struct tm *t;

  ti=time(0);
  /*
  if (EBC_User_GetFlags(u) & EBC_USER_FLAGS_TIMESTAMP_FIX1) {
  */
    t=gmtime(&ti);
    snprintf(timestamp, sizeof(timestamp)-1,
	     "%04d-%02d-%02dT%02d:%02d:%02d.000Z",
	     t->tm_year+1900,
	     t->tm_mon+1,
	     t->tm_mday,
	     t->tm_hour,
	     t->tm_min,
	     t->tm_sec);
    timestamp[sizeof(timestamp)-1]=0;
    DBG_DEBUG(AQEBICS_LOGDOMAIN,
	      "Generated timestamp [%s]",
	      timestamp);
    GWEN_Buffer_AppendString(buf, timestamp);
  /*
  }
  else {
    int thzone;

    t=localtime(&ti);
    thzone=-timezone/60;
    if (t->tm_isdst>0)
      thzone+=60;
    snprintf(timestamp, sizeof(timestamp)-1,
	     "%04d-%02d-%02dT%02d:%02d:%02d.000%+03d:%02d",
	     t->tm_year+1900,
	     t->tm_mon+1,
	     t->tm_mday,
	     t->tm_hour,
	     t->tm_min,
	     t->tm_sec,
	     (int)(thzone/60),
	     abs(thzone%60));
    timestamp[sizeof(timestamp)-1]=0;
    DBG_DEBUG(AQEBICS_LOGDOMAIN,
	      "Generated timestamp [%s] (tz=%d, daylight=%d)",
	      timestamp, (int)timezone, t->tm_isdst);
    GWEN_Buffer_AppendString(buf, timestamp);
  }
  */

  return 0;
}



int EBC_Provider_Generate_OrderId(AB_PROVIDER *pro, GWEN_BUFFER *buf) {
  uint32_t id;
  char rbuf[4];
  char c;
  uint32_t j;

  GWEN_Buffer_AllocRoom(buf, 4);
  id=AB_Banking_GetUniqueId(AB_Provider_GetBanking(pro));
  if (id==0)
    return GWEN_ERROR_IO;

  rbuf[3]=id%36;
  j=id/36;
  rbuf[2]=j%36;
  j/=36;
  rbuf[1]=j%36;
  j/=36;
  rbuf[0]=j%26;

  c=rbuf[0];
  c+='A';
  GWEN_Buffer_AppendByte(buf, c);

  c=rbuf[1];
  if (c<10) c+='0';
  else c+='A'-10;
  GWEN_Buffer_AppendByte(buf, c);

  c=rbuf[2];
  if (c<10) c+='0';
  else c+='A'-10;
  GWEN_Buffer_AppendByte(buf, c);

  c=rbuf[3];
  if (c<10) c+='0';
  else c+='A'-10;
  GWEN_Buffer_AppendByte(buf, c);

  return 0;
}



GWEN_LOGGER_LEVEL EBC_Provider_ResultCodeToLogLevel(GWEN_UNUSED AB_PROVIDER *pro,
						    const char *s) {
  if (strlen(s)!=6) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Invalid error code [%s]", s);
    return GWEN_LoggerLevel_Error;
  }
  else {
    uint8_t c;
    GWEN_LOGGER_LEVEL lev;

    c=s[1]-'0';
    switch(c) {
    case 0:  lev=GWEN_LoggerLevel_Info; break;
    case 1:  lev=GWEN_LoggerLevel_Notice; break;
    case 3:  lev=GWEN_LoggerLevel_Warning; break;
    case 6:
    case 9:
    default: lev=GWEN_LoggerLevel_Error; break;
    }

    return lev;
  }
}



const char *EBC_Provider_TechnicalCodeToString(const char *s) {
  unsigned int code;

  if (sscanf(s, "%u", &code)!=1)
    return NULL;

  switch(code) {
  case 0    : return I18N("Ok");
  case 11000: return I18N("Download postproces done");
  case 11001: return I18N("Download postproces skipped");
  case 11101: return I18N("TX segment number underrun");
  case 31001: return I18N("Order params ignored");
  case 61001: return I18N("Authentication failed");
  case 61002: return I18N("Invalid request");
  case 61099: return I18N("Internal error");
  case 61101: return I18N("TX recovery sync");
  case 91002: return I18N("Invalid user or invalid user state");
  case 91003: return I18N("User unknown");
  case 91004: return I18N("Invalid user state");
  case 91005: return I18N("Invalid order type");
  case 91006: return I18N("Unsupported order type");
  case 91007: return I18N("Distributed signature authorisation failed");
  case 91008: return I18N("Bank pubkey update required");
  case 91009: return I18N("Segment size exceeded");
  case 91010: return I18N("Invalid XML");
  case 91101: return I18N("TX unknown transaction id");
  case 91102: return I18N("TX abort");
  case 91103: return I18N("TX message replay");
  case 91104: return I18N("TX segment number exceeded");
  case 91112: return I18N("Invalid order params");
  case 91113: return I18N("Invalid request content");
  case 91117: return I18N("Max order data size exceeded");
  case 91118: return I18N("Max segments exceeded");
  case 91119: return I18N("Max transactions exceeded");
  case 91120: return I18N("Partner id mismatch");
  case 91121: return I18N("Incompatible order attribute");
  default:    return NULL;
  }
}



const char *EBC_Provider_BankCodeToString(const char *s) {
  unsigned int code;

  if (sscanf(s, "%u", &code)!=1)
    return NULL;

  switch(code) {
  case 0    : return I18N("Ok");
  case 11301: return I18N("No online checks");
  case 91001: return I18N("Download signed only");
  case 91002: return I18N("Download unsigned only");
  case 90003: return I18N("Authorisation failed");
  case 90004: return I18N("Invalid order data format");
  case 90005: return I18N("No download data available");
  case 90006: return I18N("Unsupported request for order instance");
  case 91105: return I18N("Recovery not supported");
  case 91111: return I18N("Invalid signature file format");
  case 91114: return I18N("Order id unknown");
  case 91115: return I18N("Order id already exists");
  case 91116: return I18N("Processing error");
  case 91201: return I18N("Keymgmt unsupported version of signature");
  case 91202: return I18N("Keymgmt unsupported version of authentication");
  case 91203: return I18N("Keymgmt unsupported version of encryption");
  case 91204: return I18N("Keymgmt keylength error in signature");
  case 91205: return I18N("Keymgmt keylength error in authentication");
  case 91206: return I18N("Keymgmt keylength error in encryption");
  case 91207: return I18N("Keymgmt no X509 support");
  case 91301: return I18N("Signature verification failed");
  case 91302: return I18N("Account authorisation failed");
  case 91303: return I18N("Amount check failed");
  case 91304: return I18N("Signer unknown");
  case 91305: return I18N("Invalid signer state");
  case 91306: return I18N("Duplicate signature");
  default:    return NULL;
  }
}



void EBC_Provider_LogRequestResults(AB_PROVIDER *pro,
				    EB_MSG *mRsp,
				    GWEN_BUFFER *logbuf) {
  const char *tcode;
  const char *bcode;
  const char *s;
  GWEN_BUFFER *tbuf;

  tcode=EB_Msg_GetCharValue(mRsp, "header/mutable/ReturnCode", NULL);
  bcode=EB_Msg_GetCharValue(mRsp, "body/ReturnCode", NULL);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (tcode) {
    GWEN_Buffer_AppendString(tbuf, I18N("EBICS (Technical Code):"));
    GWEN_Buffer_AppendString(tbuf, " ");
    GWEN_Buffer_AppendString(tbuf, tcode);
    s=EBC_Provider_TechnicalCodeToString(tcode);
    if (s) {
      GWEN_Buffer_AppendString(tbuf, " [");
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "]");
    }
    if (logbuf) {
      GWEN_Buffer_AppendString(logbuf, "\t");
      GWEN_Buffer_AppendBuffer(logbuf, tbuf);
    }
    GWEN_Gui_ProgressLog(0,
			 EBC_Provider_ResultCodeToLogLevel(pro, tcode),
			 GWEN_Buffer_GetStart(tbuf));
    DBG_INFO(AQEBICS_LOGDOMAIN, "%s", GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);
  }

  s=EB_Msg_GetCharValue(mRsp, "header/mutable/ReportText", NULL);
  if (s) {
    GWEN_Buffer_AppendString(tbuf, I18N("EBICS (Technical Report):"));
    GWEN_Buffer_AppendString(tbuf, " ");
    GWEN_Buffer_AppendString(tbuf, s);
    if (logbuf) {
      GWEN_Buffer_AppendString(logbuf, "\t");
      GWEN_Buffer_AppendBuffer(logbuf, tbuf);
    }
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Notice,
			 GWEN_Buffer_GetStart(tbuf));
    DBG_INFO(AQEBICS_LOGDOMAIN, "%s", GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);
  }

  s=EB_Msg_GetCharValue(mRsp, "body/ReturnCode", NULL);
  if (bcode) {
    GWEN_Buffer_AppendString(tbuf, I18N("EBICS (Bank Code):"));
    GWEN_Buffer_AppendString(tbuf, " ");
    GWEN_Buffer_AppendString(tbuf, bcode);

    s=EBC_Provider_BankCodeToString(bcode);
    if (s) {
      GWEN_Buffer_AppendString(tbuf, " [");
      GWEN_Buffer_AppendString(tbuf, s);
      GWEN_Buffer_AppendString(tbuf, "]");
    }

    if (logbuf) {
      GWEN_Buffer_AppendString(logbuf, "\t");
      GWEN_Buffer_AppendBuffer(logbuf, tbuf);
    }

    GWEN_Gui_ProgressLog(0,
			 EBC_Provider_ResultCodeToLogLevel(pro, bcode),
			 GWEN_Buffer_GetStart(tbuf));
    DBG_INFO(AQEBICS_LOGDOMAIN, "%s", GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_Reset(tbuf);
  }

  GWEN_Buffer_free(tbuf);
}



int EBC_Provider_AddBankPubKeyDigests(AB_PROVIDER *pro, AB_USER *u, xmlNodePtr node) {
  EBC_PROVIDER *dp;
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  GWEN_BUFFER *hbuf;
  xmlNodePtr nodeX = NULL;
  const char *s;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* get key id for server auth key */
  keyId=GWEN_Crypt_Token_Context_GetAuthVerifyKeyId(ctx);
  ki=GWEN_Crypt_Token_GetKeyInfo(ct,
				 keyId,
				 0xffffffff,
				 0);
  if (ki==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN,
	     "Keyinfo %04x not found on crypt token [%s:%s]",
	     keyId,
	     GWEN_Crypt_Token_GetTypeName(ct),
	     GWEN_Crypt_Token_GetTokenName(ct));
    GWEN_Crypt_Token_Close(ct, 0, 0);
    return GWEN_ERROR_NOT_FOUND;
  }

  s=EBC_User_GetAuthVersion(u);
  DBG_ERROR(0, "Auth Version: %s\n", s);
  if (! (s && *s))
    s="X001";
  if (strcasecmp(s, "X001")==0) {
    hbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=EB_Key_Info_BuildHashSha1(ki, hbuf, 1);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(hbuf);
      GWEN_Crypt_Token_Close(ct, 0, 0);
      return rv;
    }
    nodeX=xmlNewTextChild(node, NULL,
			  BAD_CAST "Authentication",
			  BAD_CAST GWEN_Buffer_GetStart(hbuf));
    GWEN_Buffer_free(hbuf);
    assert(nodeX);
    xmlNewProp(nodeX,
	       BAD_CAST "Version",
	       BAD_CAST "X001");
    xmlNewProp(nodeX,
	       BAD_CAST "Algorithm",
	       BAD_CAST "http://www.w3.org/2000/09/xmldsig#sha1");
  }
  else if (strcasecmp(s, "X002")==0) {
    hbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=EB_Key_Info_BuildHashSha256(ki, hbuf, 1);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(hbuf);
      GWEN_Crypt_Token_Close(ct, 0, 0);
      return rv;
    }
    nodeX=xmlNewTextChild(node, NULL,
			  BAD_CAST "Authentication",
			  BAD_CAST GWEN_Buffer_GetStart(hbuf));
    GWEN_Buffer_free(hbuf);
    assert(nodeX);
    xmlNewProp(nodeX,
	       BAD_CAST "Version",
	       BAD_CAST "X002");
    xmlNewProp(nodeX,
	       BAD_CAST "Algorithm",
	       BAD_CAST "http://www.w3.org/2001/04/xmlenc#sha256");
  }
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Unsupported auth version [%s]", s);
    GWEN_Crypt_Token_Close(ct, 0, 0);
    return GWEN_ERROR_INTERNAL;
  }

  /* get key id for server crypt key */
  keyId=GWEN_Crypt_Token_Context_GetEncipherKeyId(ctx);
  ki=GWEN_Crypt_Token_GetKeyInfo(ct,
				 keyId,
				 0xffffffff,
				 0);
  if (ki==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN,
	     "Keyinfo %04x not found on crypt token [%s:%s]",
	     keyId,
	     GWEN_Crypt_Token_GetTypeName(ct),
	     GWEN_Crypt_Token_GetTokenName(ct));
    GWEN_Crypt_Token_Close(ct, 0, 0);
    return GWEN_ERROR_NOT_FOUND;
  }

  s=EBC_User_GetCryptVersion(u);
  if (! (s && *s))
    s="E001";
  if (strcasecmp(s, "E001")==0) {
    hbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=EB_Key_Info_BuildHashSha1(ki, hbuf, 1);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(hbuf);
      GWEN_Crypt_Token_Close(ct, 0, 0);
      return rv;
    }
    nodeX=xmlNewTextChild(node, NULL,
			  BAD_CAST "Encryption",
			  BAD_CAST GWEN_Buffer_GetStart(hbuf));
    GWEN_Buffer_free(hbuf);
    assert(nodeX);
    xmlNewProp(nodeX,
	       BAD_CAST "Version",
	       BAD_CAST "E001");
    xmlNewProp(nodeX,
	       BAD_CAST "Algorithm",
	       BAD_CAST "http://www.w3.org/2000/09/xmldsig#sha1");
  }
  else if (strcasecmp(s, "E002")==0) {
    hbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=EB_Key_Info_BuildHashSha256(ki, hbuf, 1);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(hbuf);
      GWEN_Crypt_Token_Close(ct, 0, 0);
      return rv;
    }
    nodeX=xmlNewTextChild(node, NULL,
			  BAD_CAST "Encryption",
			  BAD_CAST GWEN_Buffer_GetStart(hbuf));
    GWEN_Buffer_free(hbuf);
    assert(nodeX);
    xmlNewProp(nodeX,
	       BAD_CAST "Version",
	       BAD_CAST "E002");
    xmlNewProp(nodeX,
	       BAD_CAST "Algorithm",
	       BAD_CAST "http://www.w3.org/2001/04/xmlenc#sha256");
  }
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Unsupported crypt version [%s]", s);
    GWEN_Crypt_Token_Close(ct, 0, 0);
    return GWEN_ERROR_INTERNAL;
  }

  return 0;
}



int EBC_Provider_FillDataEncryptionInfoNode(AB_PROVIDER *pro, AB_USER *u,
					    const GWEN_CRYPT_KEY *skey,
					    xmlNodePtr node) {
  EBC_PROVIDER *dp;
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki;
  uint32_t keyId;
  GWEN_BUFFER *hbuf;
  xmlNodePtr nodeX = NULL;
  const char *s;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* get key id for server crypt key */
  keyId=GWEN_Crypt_Token_Context_GetEncipherKeyId(ctx);
  ki=GWEN_Crypt_Token_GetKeyInfo(ct,
				 keyId,
				 0xffffffff,
				 0);
  if (ki==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN,
	     "Keyinfo %04x not found on crypt token [%s:%s]",
	     keyId,
	     GWEN_Crypt_Token_GetTypeName(ct),
	     GWEN_Crypt_Token_GetTokenName(ct));
    GWEN_Crypt_Token_Close(ct, 0, 0);
    return GWEN_ERROR_NOT_FOUND;
  }

  hbuf=GWEN_Buffer_new(0, 256, 0, 1);

  s=EBC_User_GetCryptVersion(u);
  if (!(s && *s))
    s="E001";
  if (strcasecmp(s, "E001")==0) {
    rv=EB_Key_Info_BuildHashSha1(ki, hbuf, 1);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(hbuf);
      GWEN_Crypt_Token_Close(ct, 0, 0);
      return rv;
    }
    nodeX=xmlNewTextChild(node, NULL,
			  BAD_CAST "EncryptionPubKeyDigest",
			  BAD_CAST GWEN_Buffer_GetStart(hbuf));
    GWEN_Buffer_free(hbuf);
    assert(nodeX);

    xmlNewProp(nodeX,
	       BAD_CAST "Version",
	       BAD_CAST "E001");
    xmlNewProp(nodeX,
	       BAD_CAST "Algorithm",
	       BAD_CAST "http://www.w3.org/2000/09/xmldsig#sha1");
  }
  else if (strcasecmp(s, "E002")==0) {
    rv=EB_Key_Info_BuildHashSha256(ki, hbuf, 1);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(hbuf);
      GWEN_Crypt_Token_Close(ct, 0, 0);
      return rv;
    }
    nodeX=xmlNewTextChild(node, NULL,
			  BAD_CAST "EncryptionPubKeyDigest",
			  BAD_CAST GWEN_Buffer_GetStart(hbuf));
    GWEN_Buffer_free(hbuf);
    assert(nodeX);

    xmlNewProp(nodeX,
	       BAD_CAST "Version",
	       BAD_CAST "E002");
    xmlNewProp(nodeX,
	       BAD_CAST "Algorithm",
	       BAD_CAST "http://www.w3.org/2001/04/xmlenc#sha256");
  }


  /* add encrypted transactio key */
  hbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=EBC_Provider_EncryptKey(pro, u, skey, hbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_Crypt_Token_Close(ct, 0, 0);
    return rv;
  }

  nodeX=xmlNewTextChild(node, NULL,
			BAD_CAST "TransactionKey",
			BAD_CAST GWEN_Buffer_GetStart(hbuf));
  GWEN_Buffer_free(hbuf);
  assert(nodeX);

  return 0;
}



int EBC_Provider_GetCert(AB_PROVIDER *pro, AB_USER *u) {
  GWEN_HTTP_SESSION *sess;
  int rv;
  AB_BANKING *ab;

  sess=EBC_Dialog_new(pro, u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  /* create and open session */
  sess=EBC_Dialog_new(pro, u);
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Could not open session");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* try to connect */
  rv=GWEN_HttpSession_ConnectionTest(sess);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "Could not connect to server");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  return 0;
}



#include "p_sign_x001.c"
#include "p_sign_x002.c"
#include "p_sign.c"
#include "p_decipher.c"
#include "p_encipher_e001.c"
#include "p_encipher_e002.c"
#include "p_encipher.c"
#include "p_eu_a004.c"
#include "p_eu_a005.c"
#include "p_eu.c"
#include "p_tools.c"
#include "r_ini_h002.c"
#include "r_ini_h003.c"
#include "r_ini.c"
#include "r_hia_h002.c"
#include "r_hia_h003.c"
#include "r_hia.c"
#include "r_hpb_h002.c"
#include "r_hpb_h003.c"
#include "r_hpb.c"
#include "r_download_h002.c"
#include "r_download_h003.c"
#include "r_download.c"
#include "r_hpd.c"
#include "r_hkd.c"
#include "r_htd.c"
#include "r_sta.c"
#include "r_upload_h002.c"
#include "r_upload_h003.c"
#include "r_upload.c"
#include "r_pub_h002.c"
#include "r_pub_h003.c"
#include "r_pub.c"


