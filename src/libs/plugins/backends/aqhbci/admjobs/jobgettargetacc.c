/***************************************************************************
 begin       : Tue Oct 12 2021
 copyright   : (C) 2021 by Stefan Bayer, Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "jobgettargetacc_p.h"

#include "aqhbci/joblayer/job_crypt.h"
#include "aqhbci/banking/provider_job.h"
#include "aqhbci/ajobs//accountjob_l.h"

#include <aqbanking/backendsupport/account.h>
#include <aqbanking/types/refaccount.h>
#include <aqbanking/types/transactionlimits.h>

GWEN_INHERIT(AH_JOB, AH_JOB_GETTARGETACC)


static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static int _cbProcess(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static AB_REFERENCE_ACCOUNT *_getOrCreateReferenceAccount(AB_ACCOUNT_SPEC *as, GWEN_DB_NODE *dbTargetAccount);
static int _createTransactionLimits(const AH_JOB *j, AB_ACCOUNT_SPEC *as);



AH_JOB *AH_Job_GetTargetAccount_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *acc)
{
  AH_JOB *j;
  AH_JOB_GETTARGETACC *jd;

  assert(u);
  j=AH_AccountJob_new("JobGetAccountTargetAccount", pro, u, acc);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobGetAccountTargetAccount not supported, should not happen");
    return NULL;
  }

  GWEN_NEW_OBJECT(AH_JOB_GETTARGETACC, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETTARGETACC, j, jd, _freeData)
  AH_Job_SetProcessFn(j, _cbProcess);

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobGetAccountTargetAccount created");
  return j;
}



void GWENHYWFAR_CB _freeData(void *bp, void *p)
{
  AH_JOB_GETTARGETACC *jd;

  jd = (AH_JOB_GETTARGETACC *) p;
  GWEN_FREE_OBJECT(jd);
}



int _cbProcess(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_GETTARGETACC *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  AB_BANKING *ab;
  AB_PROVIDER *pro;

  assert(j);
  jd = GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETTARGETACC, j);
  assert(jd);

  if (jd->scanned)
    return 0;

  jd->scanned = 1;

  dbResponses = AH_Job_GetResponses(j);
  assert(dbResponses);

  ab = AH_Job_GetBankingApi(j);
  assert(ab);

  pro = AH_Job_GetProvider(j);
  assert(pro);

  /* search for "GetAccountTargetAccountResponse" */
  dbCurr = GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    GWEN_DB_NODE *dbXA;
    int rv;

    rv = AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv = AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    dbXA = GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/GetAccountTargetAccountResponse");
    if (dbXA) {
      uint32_t uniqueAccountId;
      AB_ACCOUNT_SPEC *as;
      GWEN_DB_NODE *dbTargetAccount;

      /* check if we have a structure with an "account" group" */
      uniqueAccountId = AB_Account_GetUniqueId(AH_AccountJob_GetAccount(j));
      rv = AB_Banking_GetAccountSpecByUniqueId(ab, uniqueAccountId, &as);
      if (rv<0) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "No account spec for account, SNH!");
	return GWEN_ERROR_INTERNAL;
      }

      dbTargetAccount = GWEN_DB_FindFirstGroup(dbXA, "targetAccount");
      while (dbTargetAccount) {
	AB_REFERENCE_ACCOUNT *ra;

	ra=_getOrCreateReferenceAccount(as, dbTargetAccount);
	if (ra==NULL) {
	  DBG_INFO(AQHBCI_LOGDOMAIN, "Error getting reference account from response segment");
	  return GWEN_ERROR_GENERIC;
	}

        /* check if the internal transfer is already part of the transaction limits, if not, add them */
	if (AB_AccountSpec_GetTransactionLimitsForCommand(as, AB_Transaction_CommandSepaInternalTransfer)==NULL) {
	  rv=_createTransactionLimits(j, as);
	  if (rv<0) {
	    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	    return rv;
	  }
	}

	dbTargetAccount = GWEN_DB_FindNextGroup(dbTargetAccount, "targetAccount");
      } /* while (dbTargetAccount) */

      rv = AB_Banking_WriteAccountSpec(AB_Provider_GetBanking(pro), as);
      AB_AccountSpec_free(as);
    } /* if (dbXA) */
    dbCurr = GWEN_DB_GetNextGroup(dbCurr);
  } /* while dbCurr */

  return 0;
}



AB_REFERENCE_ACCOUNT *_getOrCreateReferenceAccount(AB_ACCOUNT_SPEC *as, GWEN_DB_NODE *dbTargetAccount)
{
  const char *reqIban;
  const char *reqBic;
  const char *reqAccountId;
  const char *reqAccountSubId;
  const char *reqOwnerName;
  const char *reqAddtlOwnerName;
  const char *reqAccDescription;
  const char *reqCountry;
  const char *reqBankCode;
  int reqAccountType;
  AB_REFERENCE_ACCOUNT_LIST *refAccountList;
  AB_REFERENCE_ACCOUNT *refAccount=NULL;

  /* get unique id of the target account */
  reqIban = GWEN_DB_GetCharValue(dbTargetAccount, "account/iban", 0, NULL);
  reqBic = GWEN_DB_GetCharValue(dbTargetAccount, "account/bic", 0, NULL);
  reqAccountId = GWEN_DB_GetCharValue(dbTargetAccount, "account/accountid", 0, NULL);
  reqAccountSubId = GWEN_DB_GetCharValue(dbTargetAccount, "account/accountsubid", 0, NULL);
  reqCountry = GWEN_DB_GetCharValue(dbTargetAccount, "account/country", 0, NULL);
  reqBankCode = GWEN_DB_GetCharValue(dbTargetAccount, "account/bankcode", 0, NULL);
  reqOwnerName = GWEN_DB_GetCharValue(dbTargetAccount, "rcvName1", 0, NULL);
  if (!(reqOwnerName && *reqOwnerName))
    reqOwnerName=AB_AccountSpec_GetOwnerName(as);
  reqAddtlOwnerName = GWEN_DB_GetCharValue(dbTargetAccount, "rcvName2", 0, NULL);
  reqAccountType = GWEN_DB_GetIntValue(dbTargetAccount, "accType", 0, 0);
  reqAccDescription = GWEN_DB_GetCharValue(dbTargetAccount, "accDescription", 0, NULL);
  refAccountList = AB_AccountSpec_GetRefAccountList(as);
  refAccount=refAccountList?AB_ReferenceAccount_List_FindFirst(refAccountList,
							       reqIban, reqBic,
							       reqAccountId, reqAccountSubId,
							       reqCountry, reqBankCode,
							       "*", reqAccDescription):NULL;
  if (refAccount == NULL) {
    AB_REFERENCE_ACCOUNT *refAccount;

    refAccount = AB_ReferenceAccount_new();
    AB_ReferenceAccount_SetIban(refAccount, reqIban);
    AB_ReferenceAccount_SetBic(refAccount, reqBic);
    AB_ReferenceAccount_SetAccountNumber(refAccount, reqAccountId);
    AB_ReferenceAccount_SetSubAccountNumber(refAccount, reqAccountSubId);
    AB_ReferenceAccount_SetCountry(refAccount, reqCountry);
    AB_ReferenceAccount_SetBankCode(refAccount, reqBankCode);
    AB_ReferenceAccount_SetOwnerName(refAccount, reqOwnerName);
    AB_ReferenceAccount_SetOwnerName2(refAccount, reqAddtlOwnerName);
    AB_ReferenceAccount_SetAccountName(refAccount, reqAccDescription);
    AB_ReferenceAccount_SetAccountType(refAccount, reqAccountType);
    AB_AccountSpec_AddReferenceAccount(as, refAccount);
  }

  return refAccount;
}



int _createTransactionLimits(const AH_JOB *j, AB_ACCOUNT_SPEC *as)
{
  AB_PROVIDER *pro;
  AB_USER *u;
  AB_TRANSACTION_LIMITS *limits;
  AH_JOB *tmpJob=NULL;
  AB_TRANSACTION_LIMITS_LIST *tll;
  int rv;

  u=AH_Job_GetUser(j);
  pro=AH_Job_GetProvider(j);

  tll=AB_AccountSpec_GetTransactionLimitsList(as);
  DBG_INFO(AQHBCI_LOGDOMAIN,
	   "Creating transaction limits for job \"%s\"",
	   AB_Transaction_Command_toString(AB_Transaction_CommandSepaInternalTransfer));
  DBG_INFO(AQHBCI_LOGDOMAIN, "- creating job");
  rv = AH_Provider_CreateHbciJob(pro, u, AH_AccountJob_GetAccount(j), AB_Transaction_CommandSepaInternalTransfer, &tmpJob);
  if (rv < 0) {
    if (rv == GWEN_ERROR_NOT_AVAILABLE) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job \"%s\" is not available",
		 AB_Transaction_Command_toString(AB_Transaction_CommandSepaInternalTransfer));
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "- getting limits");
    rv = AH_Job_GetLimits(tmpJob, &limits);
    if (rv < 0) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
	       "Error getting limits for job \"%s\": %d",
	       AB_Transaction_Command_toString(AB_Transaction_CommandSepaInternalTransfer), rv);
      AH_Job_free(tmpJob);
      return rv;
    }
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "- adding limits");
    AB_TransactionLimits_List_Add(limits, tll);
    AH_Job_free(tmpJob);
  }

  return 0;
}


