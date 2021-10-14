/***************************************************************************
 begin       : Thu Jan 31 2019
 copyright   : (C) 2019 by Martin Preuss
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
#include <aqbanking/backendsupport/account.h>
#include <aqbanking/types/refaccount.h>
#include <aqbanking/types/transactionlimits.h>

GWEN_INHERIT(AH_JOB, AH_JOB_GETTARGETACC)

AH_JOB* AH_Job_GetTargetAccount_new(AB_PROVIDER *pro, AB_USER *u,
    AB_ACCOUNT *acc) {
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;
  AH_JOB_GETTARGETACC *jd;
  const char *s;

  assert(u);
  j = AH_Job_new("JobGetAccountTargetAccount", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
        "JobGetAccountTargetAccount not supported, should not happen");
    return 0;
  }

  GWEN_NEW_OBJECT(AH_JOB_GETTARGETACC, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETTARGETACC, j, jd,
      AH_Job_GetTargetAccount_FreeData)
  AH_Job_SetProcessFn(j, AH_Job_GetTargetAccount_Process);

  jd->account = acc;

  /* set arguments */
  dbArgs = AH_Job_GetArguments(j);
  assert(dbArgs);

  s = AB_Account_GetAccountNumber(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "accountId", s);

  s = AB_Account_GetSubAccountId(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "accountSubId", s);

  s = AB_Account_GetIban(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "iban", s);

  s = AB_Account_GetBic(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "bic", s);

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobGetAccountTargetAccount created");
  return j;
}

void GWENHYWFAR_CB AH_Job_GetTargetAccount_FreeData(void *bp, void *p) {
  AH_JOB_GETTARGETACC *jd;

  jd = (AH_JOB_GETTARGETACC*) p;
  GWEN_FREE_OBJECT(jd);
}

int AH_Job_GetTargetAccount_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_GETTARGETACC *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  AB_USER *u;
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

  u = AH_Job_GetUser(j);
  assert(u);

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

    dbXA = GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
        "data/GetAccountTargetAccountResponse");
    if (dbXA) {
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
      uint32_t uniqueAccountId;
      AB_ACCOUNT_SPEC *as;
      AB_REFERENCE_ACCOUNT_LIST *ral;
      AB_REFERENCE_ACCOUNT *ra;
      /* check if we have a structure with an "account" group" */
      GWEN_DB_NODE *dbTargetAccount = GWEN_DB_FindFirstGroup(dbXA,
          "targetAccount");
      uniqueAccountId = AB_Account_GetUniqueId(jd->account);
      rv = AB_Banking_GetAccountSpecByUniqueId(ab, uniqueAccountId, &as);

      while (dbTargetAccount) {
        /* get unique id of the target account */

        reqIban = GWEN_DB_GetCharValue(dbTargetAccount, "account/iban", 0, 0);
        reqBic = GWEN_DB_GetCharValue(dbTargetAccount, "account/bic", 0, 0);
        reqAccountId = GWEN_DB_GetCharValue(dbTargetAccount,
            "account/accountid", 0, 0);
        reqAccountSubId = GWEN_DB_GetCharValue(dbTargetAccount,
            "account/accountsubid", 0, 0);
        reqCountry = GWEN_DB_GetCharValue(dbTargetAccount, "account/country", 0,
            0);
        reqBankCode = GWEN_DB_GetCharValue(dbTargetAccount, "account/bankcode",
            0, 0);
        reqOwnerName = GWEN_DB_GetCharValue(dbTargetAccount, "rcvName1", 0, 0);
        reqAddtlOwnerName = GWEN_DB_GetCharValue(dbTargetAccount, "rcvName2", 0,
            0);
        reqAccountType = GWEN_DB_GetIntValue(dbTargetAccount, "accType", 0, 0);
        reqAccDescription = GWEN_DB_GetCharValue(dbTargetAccount,
            "accDescription", 0, 0);
        ral = AB_AccountSpec_GetRefAccountList(as);
        if (ral == NULL) {
          ra = NULL;
        } else {
          ra = AB_ReferenceAccount_List_FindFirst(ral, reqIban, reqBic,
              reqAccountId, reqAccountSubId, reqCountry, reqBankCode,
              reqOwnerName, reqAccDescription);
        }
        if (ra == NULL) {
          AB_REFERENCE_ACCOUNT *new_ra = AB_ReferenceAccount_new();
          AB_ReferenceAccount_SetIban(new_ra, reqIban);
          AB_ReferenceAccount_SetBic(new_ra, reqBic);
          AB_ReferenceAccount_SetAccountNumber(new_ra, reqAccountId);
          AB_ReferenceAccount_SetSubAccountNumber(new_ra, reqAccountSubId);
          AB_ReferenceAccount_SetCountry(new_ra, reqCountry);
          AB_ReferenceAccount_SetBankCode(new_ra, reqBankCode);
          AB_ReferenceAccount_SetOwnerName(new_ra, reqOwnerName);
          AB_ReferenceAccount_SetOwnerName2(new_ra, reqAddtlOwnerName);
          AB_ReferenceAccount_SetAccountName(new_ra, reqAccDescription);
          AB_ReferenceAccount_SetAccountType(new_ra, reqAccountType);
          AB_AccountSpec_AddReferenceAccount(as, new_ra);
        }

        /* check if the internal transfer is already part of the transaction limits, if not, add them */
        {
          AB_TRANSACTION_LIMITS *limits =
              AB_AccountSpec_GetTransactionLimitsForCommand(as,
                  AB_Transaction_CommandSepaInternalTransfer);
          if (limits == NULL) {
            AH_JOB *j = NULL;
            AB_TRANSACTION_LIMITS_LIST *tll =
                AB_AccountSpec_GetTransactionLimitsList(as);
            DBG_INFO(AQHBCI_LOGDOMAIN,
                "Creating transaction limits for job \"%s\"",
                AB_Transaction_Command_toString(
                    AB_Transaction_CommandSepaInternalTransfer));
            DBG_INFO(AQHBCI_LOGDOMAIN, "- creating job");
            rv = AH_Provider_CreateHbciJob(pro, u, jd->account,
                AB_Transaction_CommandSepaInternalTransfer, &j);
            if (rv < 0) {
              if (rv == GWEN_ERROR_NOT_AVAILABLE) {
                DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job \"%s\" is not available",
                    AB_Transaction_Command_toString(
                        AB_Transaction_CommandSepaInternalTransfer));
              } else {
                DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
                return rv;
              }
            } else {
              DBG_INFO(AQHBCI_LOGDOMAIN, "- getting limits");
              rv = AH_Job_GetLimits(j, &limits);
              if (rv < 0) {
                DBG_INFO(AQHBCI_LOGDOMAIN,
                    "Error getting limits for job \"%s\": %d",
                    AB_Transaction_Command_toString(
                        AB_Transaction_CommandSepaInternalTransfer), rv);
                AH_Job_free(j);
                return rv;
              }
              DBG_NOTICE(AQHBCI_LOGDOMAIN, "- adding limits");
              AB_TransactionLimits_List_Add(limits, tll);
              AH_Job_free(j);
            }
          }
        }
        dbTargetAccount = GWEN_DB_FindNextGroup(dbTargetAccount,
            "targetAccount");

      } /* while (dbTargetAccount) */

      rv = AB_Banking_WriteAccountSpec(AB_Provider_GetBanking(pro), as);
    } /* if (dbXA) */
    dbCurr = GWEN_DB_GetNextGroup(dbCurr);
  } /* while dbCurr */

  return 0;
}

