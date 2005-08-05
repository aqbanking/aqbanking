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
#include "wcb_l.h"
#include "hbci_l.h"
#include "outbox_l.h"
#include <aqbanking/account_be.h>
#include <aqbanking/provider_be.h>
#include <aqbanking/job_be.h>
#include <aqhbci/jobgetbalance.h>
#include <aqhbci/jobgettransactions.h>
#include <aqhbci/jobgetstandingorders.h>
#include <aqhbci/jobgetdatedxfers.h>
#include <aqhbci/jobsingletransfer.h>
#include <aqhbci/jobmultitransfer.h>
#include <aqhbci/jobeutransfer.h>
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
  AB_Provider_SetGetAccountListFn(pro, AH_Provider_GetAccountList);
  AB_Provider_SetUpdateAccountFn(pro, AH_Provider_UpdateAccount);
  AB_Provider_SetAddAccountFn(pro, AH_Provider_AddAccount);

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
#ifdef HAVE_I18N
  const char *s;
#endif
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

#ifdef HAVE_I18N
  setlocale(LC_ALL,"");
  s=bindtextdomain(PACKAGE,  LOCALEDIR);
  if (s) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Locale bound.");
    bind_textdomain_codeset(PACKAGE, "UTF-8");
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error binding locale");
  }
#endif

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



AH_ACCOUNT *AH_Provider__FindMyAccount(AB_PROVIDER *pro, AB_ACCOUNT *a) {
  AH_PROVIDER *hp;
  AH_ACCOUNT *ma;
  AH_BANK *mb;
  GWEN_DB_NODE *db;
  int country;
  const char *accountId;
  const char *bankCode;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  db=AB_Account_GetProviderData(a);
  assert(db);
  country=GWEN_DB_GetIntValue(db, "country", 0, 280);
  accountId=GWEN_DB_GetCharValue(db, "accountId", 0,
                                 AB_Account_GetAccountNumber(a));
  assert(accountId);
  bankCode=GWEN_DB_GetCharValue(db, "serverBankCode", 0,
                                AB_Account_GetBankCode(a));
  assert(bankCode);

  mb=AH_HBCI_FindBank(hp->hbci, country, bankCode);
  if (!mb) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Bank not found");
    return 0;
  }

  ma=AH_Bank_FindAccount(mb, accountId);
  if (!ma) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Account not found");
    return 0;
  }

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "country",
                      AH_Bank_GetCountry(mb));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "serverBankCode",
                       AH_Bank_GetBankId(mb));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "accountId",
                       AH_Account_GetAccountId(ma));

  return ma;
}



int AH_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j){
  AH_PROVIDER *hp;
  GWEN_DB_NODE *dbAccount;
  GWEN_DB_NODE *dbJob;
  AH_JOB *mj;
  AH_ACCOUNT *ma;
  AH_BANK *mb;
  AH_CUSTOMER *mcu;
  const GWEN_STRINGLIST *custs;
  int rv;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  dbAccount=AB_Account_GetProviderData(AB_Job_GetAccount(j));
  assert(dbAccount);

  dbJob=AB_Job_GetProviderData(j, pro);
  assert(dbJob);

  ma=AH_Provider__FindMyAccount(pro, AB_Job_GetAccount(j));
  if (!ma) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Account for this job not found");
    return AB_ERROR_INVALID;
  }


  mb=AH_Account_GetBank(ma);
  assert(mb);

  /* determine customer to use */
  custs=AH_Account_GetCustomers(ma);
  if (!custs) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No customers noted for account \"%d/%s/%s\"",
              AH_Bank_GetCountry(mb),
              AH_Account_GetBankId(ma),
              AH_Account_GetAccountId(ma));
    mcu = 0;
  }
  else {
    const char *lcustid;

    lcustid=GWEN_DB_GetCharValue(dbAccount, "customer", 0, 0);
    if (!lcustid) {
      GWEN_STRINGLISTENTRY *se;

      se=GWEN_StringList_FirstEntry(custs);
      if (se)
        lcustid=GWEN_StringListEntry_Data(se);
    }
    if (!lcustid) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No customers noted for account \"%d/%s/%s\"",
                AH_Bank_GetCountry(mb),
                AH_Account_GetBankId(ma),
                AH_Account_GetAccountId(ma));
      mcu = 0;
    }
    else {
      mcu=AH_HBCI_FindCustomer(hp->hbci,
                               AH_Bank_GetCountry(mb),
                               AH_Bank_GetBankId(mb),
                               "*",
                               lcustid);
      if (!mcu) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Customer \"%d/%s/%s\" not found",
                  AH_Bank_GetCountry(mb),
                  AH_Bank_GetBankId(mb),
                  lcustid);
        return AB_ERROR_NOT_AVAILABLE;
      }
    }
  }
  if (!mcu) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No customer for this account");
    return AB_ERROR_NOT_AVAILABLE;
  }

  mj=0;
  switch(AB_Job_GetType(j)) {

  case AB_Job_TypeGetBalance:
    mj=AH_Job_GetBalance_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeGetTransactions:
    mj=AH_Job_GetTransactions_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeGetStandingOrders:
    mj=AH_Job_GetStandingOrders_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeGetDatedTransfers:
    mj=AH_Job_GetDatedTransfers_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeTransfer:
    mj=0;
    if (!AH_Customer_GetPreferSingleTransfer(mcu)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Customer prefers multi jobs");
      /* create new job */
      mj=AH_Job_MultiTransfer_new(mcu, ma);
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
      mj=AH_Job_SingleTransfer_new(mcu, ma);
      if (!mj) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
	return AB_ERROR_NOT_AVAILABLE;
      }
    }
    break;

  case AB_Job_TypeDebitNote:
    mj=0;
    if (!AH_Customer_GetPreferSingleTransfer(mcu)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Customer prefers multi jobs");
      /* create new job */
      mj=AH_Job_MultiDebitNote_new(mcu, ma);
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
      mj=AH_Job_SingleDebitNote_new(mcu, ma);
      if (!mj) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
	return AB_ERROR_NOT_AVAILABLE;
      }
    }
    break;

  case AB_Job_TypeEuTransfer:
    mj=AH_Job_EuTransfer_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeCreateStandingOrder:
    mj=AH_Job_CreateStandingOrder_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeModifyStandingOrder:
    mj=AH_Job_ModifyStandingOrder_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeDeleteStandingOrder:
    mj=AH_Job_DeleteStandingOrder_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeCreateDatedTransfer:
    mj=AH_Job_CreateDatedTransfer_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeModifyDatedTransfer:
    mj=AH_Job_ModifyDatedTransfer_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeDeleteDatedTransfer:
    mj=AH_Job_DeleteDatedTransfer_new(mcu, ma);
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
  AH_ACCOUNT *ma;
  AH_BANK *mb;
  AH_CUSTOMER *mcu;
  const GWEN_STRINGLIST *custs;
  int rv;
  int sigs;
  int jobIsNew=1;
  AB_JOB_STATUS jst;

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

  ma=AH_Provider__FindMyAccount(pro, AB_Job_GetAccount(j));
  if (!ma) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Account for this job not found");
    return AB_ERROR_INVALID;
  }

  mb=AH_Account_GetBank(ma);
  assert(mb);

  /* determine customer to use */
  custs=AH_Account_GetCustomers(ma);
  if (!custs) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No customers noted for account \"%d/%s/%s\"",
              AH_Bank_GetCountry(mb),
              AH_Account_GetBankId(ma),
              AH_Account_GetAccountId(ma));
    mcu=0;
  }
  else {
    const char *lcustid;

    lcustid=GWEN_DB_GetCharValue(dbAccount, "customer", 0, 0);
    if (!lcustid) {
      GWEN_STRINGLISTENTRY *se;

      /* find a customer from the account's list */
      se=GWEN_StringList_FirstEntry(custs);
      while(se) {
        const char *s;

        s=GWEN_StringListEntry_Data(se);
        mcu=AH_HBCI_FindCustomer(hp->hbci,
                                 AH_Bank_GetCountry(mb),
                                 AH_Bank_GetBankId(mb),
                                 "*",
                                 s);
        if (mcu) {
          lcustid=s;
          break;
        }
        se=GWEN_StringListEntry_Next(se);
      } /* while */
    }

    if (!lcustid) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "No customers noted for account \"%d/%s/%s\"",
                AH_Bank_GetCountry(mb),
                AH_Account_GetBankId(ma),
                AH_Account_GetAccountId(ma));
      mcu=0;
    }
    else {
      mcu=AH_HBCI_FindCustomer(hp->hbci,
                               AH_Bank_GetCountry(mb),
                               AH_Bank_GetBankId(mb),
                               "*",
                               lcustid);
      if (!mcu) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Customer \"%d/%s/%s\" not found",
                  AH_Bank_GetCountry(mb),
                  AH_Bank_GetBankId(mb),
                  lcustid);
        return AB_ERROR_NOT_AVAILABLE;
      }
    }
  }

  if (!mcu) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No customer for this account");
    return AB_ERROR_NOT_AVAILABLE;
  }

  mj=0;
  switch(AB_Job_GetType(j)) {

  case AB_Job_TypeGetBalance:
    mj=AH_Job_GetBalance_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeGetTransactions:
    mj=AH_Job_GetTransactions_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeGetStandingOrders:
    mj=AH_Job_GetStandingOrders_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeGetDatedTransfers:
    mj=AH_Job_GetDatedTransfers_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeTransfer:
    mj=0;
    if (!AH_Customer_GetPreferSingleTransfer(mcu)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Customer prefers multi jobs");
      mj=AH_Outbox_FindTransferJob(hp->outbox, mcu, ma, 1);
      if (mj) {
	/* simply add transfer to existing job */
        jobIsNew=0;
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No matching job found");
      }
      else {
	/* create new job */
	mj=AH_Job_MultiTransfer_new(mcu, ma);
	if (!mj) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Multi-job not supported with this account, "
		    "using single-job");
	}
      }
    }
    if (!mj) {
      mj=AH_Job_SingleTransfer_new(mcu, ma);
      if (!mj) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
	return AB_ERROR_NOT_AVAILABLE;
      }
    }
    break;

  case AB_Job_TypeDebitNote:
    mj=0;
    if (!AH_Customer_GetPreferSingleTransfer(mcu)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Customer prefers multi jobs");
      mj=AH_Outbox_FindTransferJob(hp->outbox, mcu, ma, 0);
      if (mj) {
	/* simply add transfer to existing job */
        jobIsNew=0;
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No matching job found");
      }
      else {
	/* create new job */
	mj=AH_Job_MultiDebitNote_new(mcu, ma);
	if (!mj) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Multi-job not supported with this account, "
		    "using single-job");
	}
      }
    }
    if (!mj) {
      mj=AH_Job_SingleDebitNote_new(mcu, ma);
      if (!mj) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
	return AB_ERROR_NOT_AVAILABLE;
      }
    }
    break;

  case AB_Job_TypeEuTransfer:
    mj=AH_Job_EuTransfer_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeCreateStandingOrder:
    mj=AH_Job_CreateStandingOrder_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeModifyStandingOrder:
    mj=AH_Job_ModifyStandingOrder_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeDeleteStandingOrder:
    mj=AH_Job_DeleteStandingOrder_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeCreateDatedTransfer:
    mj=AH_Job_CreateDatedTransfer_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeModifyDatedTransfer:
    mj=AH_Job_ModifyDatedTransfer_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Job_TypeDeleteDatedTransfer:
    mj=AH_Job_DeleteDatedTransfer_new(mcu, ma);
    if (!mj) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return AB_ERROR_NOT_AVAILABLE;
    }
    break;

  default:
    mj=AH_Provider__GetPluginJob(hp, AB_Job_GetType(j), mcu, ma);
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
      AH_Job_AddSigner(mj, // AH_Customer_GetCustomerId(mcu));
		       AH_User_GetUserId(AH_Customer_GetUser(mcu)));
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



int AH_Provider_Execute(AB_PROVIDER *pro){
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

      mj=AH_Provider__FindMyJob(mjl, AB_Job_GetIdForProvider(bj));
      assert(mj);

      beData=AB_Job_GetProviderData(bj, pro);
      assert(beData);

      /* store used TAN (if any) */
      s=AH_Job_GetUsedTan(mj);
      if (s)
        GWEN_DB_SetCharValue(beData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "usedTan", s);

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
          }
        }
        else {
          /* job is ok */
          if (AB_Job_GetStatus(bj)==AB_Job_StatusSent) {
            AB_Job_SetStatus(bj, AB_Job_StatusFinished);
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



AB_ACCOUNT_LIST2 *AH_Provider_GetAccountList(AB_PROVIDER *pro){
  AH_PROVIDER *hp;
  AB_ACCOUNT_LIST2 *lba;
  AH_ACCOUNT_LIST2 *lma;
  AH_ACCOUNT_LIST2_ITERATOR *ait;
  AB_ACCOUNT *ba;
  AH_ACCOUNT *ma;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  lma=AH_HBCI_GetAccounts(hp->hbci, 0, "*", "*");
  if (!lma) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No accounts");
    return 0;
  }

  lba=AB_Account_List2_new();
  ait=AH_Account_List2_First(lma);
  assert(ait);
  ma=AH_Account_List2Iterator_Data(ait);
  assert(ma);
  while(ma) {
    int rv;

    ba=AB_Account_new(AB_Provider_GetBanking(pro),
                      pro, 0);
    rv=AH_Provider__FillAccount(pro, ba, ma);
    if (rv) {
      AB_Account_free(ba);
    }
    else {
      /* add job */
      AB_Account_List2_PushBack(lba, ba);
    }
    ma=AH_Account_List2Iterator_Next(ait);
  } /* while */
  AH_Account_List2Iterator_free(ait);
  AH_Account_List2_free(lma);

  return lba;
}



int AH_Provider__FillAccount(AB_PROVIDER *pro,
                             AB_ACCOUNT *a,
                             AH_ACCOUNT *ma){
  AH_PROVIDER *hp;
  GWEN_DB_NODE *dbAccount;
  AH_BANK *mb;
  const char *p;

  assert(pro);
  assert(ma);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  dbAccount=AB_Account_GetProviderData(a);
  assert(dbAccount);

  mb=AH_Account_GetBank(ma);
  assert(mb);

  AB_Account_SetAccountNumber(a, AH_Account_GetAccountId(ma));
  AB_Account_SetBankCode(a, AH_Account_GetBankId(ma));
  p=AH_Account_GetAccountName(ma);
  if (p)
    AB_Account_SetAccountName(a, p);
  p=AH_Bank_GetBankName(mb);
  if (p)
    AB_Account_SetBankName(a, p);

  p=AH_Account_GetOwnerName(ma);
  if (p)
    AB_Account_SetOwnerName(a, p);

  p=AH_Bank_GetBankId(mb);
  GWEN_DB_SetCharValue(dbAccount, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "serverBankCode", p);

  return 0;
}



int AH_Provider_UpdateAccount(AB_PROVIDER *pro, AB_ACCOUNT *a){
  AH_PROVIDER *hp;
  GWEN_DB_NODE *dbAccount;
  AH_ACCOUNT *ma;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  dbAccount=AB_Account_GetProviderData(a);
  assert(dbAccount);

  ma=AH_Provider__FindMyAccount(pro, a);
  if (!ma) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Account not found");
    return AB_ERROR_NOT_AVAILABLE;
  }

  return AH_Provider__FillAccount(pro, a, ma);
}



int AH_Provider_AddAccount(AB_PROVIDER *pro, AB_ACCOUNT *a){
  AH_PROVIDER *hp;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  return AB_ERROR_NOT_SUPPORTED;
}



AH_HBCI *AH_Provider_GetHbci(AB_PROVIDER *pro){
  AH_PROVIDER *hp;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  return hp->hbci;
}



AH_JOB *AH_Provider__GetPluginJob(AH_PROVIDER *hp,
                                  AB_JOB_TYPE jt,
                                  AH_CUSTOMER *mcu,
                                  AH_ACCOUNT *ma){
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





