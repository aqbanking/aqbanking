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


#include "jobgetsepainfo_p.h"

#include "aqhbci/joblayer/job_crypt.h"



GWEN_INHERIT(AH_JOB, AH_JOB_GETACCSEPAINFO)



AH_JOB *AH_Job_GetAccountSepaInfo_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *acc)
{
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;
  AH_JOB_GETACCSEPAINFO *jd;
  const char *s;

  assert(u);
  j=AH_Job_new("JobGetAccountSepaInfo", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobGetAccountSepaInfo not supported, should not happen");
    return 0;
  }

  GWEN_NEW_OBJECT(AH_JOB_GETACCSEPAINFO, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETACCSEPAINFO, j, jd,
                       AH_Job_GetAccountSepaInfo_FreeData)
  AH_Job_SetProcessFn(j, AH_Job_GetAccountSepaInfo_Process);

  jd->account=acc;

  /* set arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  s=AB_Account_GetAccountNumber(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "accountId", s);

  s=AB_Account_GetSubAccountId(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "accountSubId", s);

  s=AB_Account_GetBankCode(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "bankCode", s);

  GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "country", 280);

  s=AB_Account_GetIban(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "iban", s);

  s=AB_Account_GetBic(jd->account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "bic", s);


  DBG_INFO(AQHBCI_LOGDOMAIN, "JobGetAccountSepaInfo created");
  return j;
}



void GWENHYWFAR_CB AH_Job_GetAccountSepaInfo_FreeData(void *bp, void *p)
{
  AH_JOB_GETACCSEPAINFO *jd;

  jd=(AH_JOB_GETACCSEPAINFO *)p;
  GWEN_FREE_OBJECT(jd);
}



int AH_Job_GetAccountSepaInfo_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_GETACCSEPAINFO *jd;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;
  AB_USER *u;
  AB_BANKING *ab;
  AB_PROVIDER *pro;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETACCSEPAINFO, j);
  assert(jd);

  if (jd->scanned)
    return 0;

  jd->scanned=1;

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  u=AH_Job_GetUser(j);
  assert(u);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  pro=AH_Job_GetProvider(j);
  assert(pro);

  /* search for "GetAccountSepaInfoResponse" */
  dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
  while (dbCurr) {
    GWEN_DB_NODE *dbXA;
    int rv;

    rv=AH_Job_CheckEncryption(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
    rv=AH_Job_CheckSignature(j, dbCurr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }

    dbXA=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/GetAccountSepaInfoResponse");
    if (dbXA) {
      /* check if we have a structure with an "account" group" */
      GWEN_DB_NODE *dbAccount=GWEN_DB_FindFirstGroup(dbXA, "account");
      if (dbAccount) {
        const char *accountId;
        const char *bankCode;
        const char *accountSuffix;
        const char *sSepa;

        /* there is account info, are there multiple accounts returned? */
        if (NULL!=GWEN_DB_FindNextGroup(dbAccount, "account")) {
          const char *reqAccountId;
          const char *reqBankCode;
          const char *reqAccountSuffix;

          /* yes, multiple accounts, find the one we requested */
          DBG_INFO(AQHBCI_LOGDOMAIN, "Multiple accounts returned in GetAccountSepaInfoResponse");
          /* GWEN_DB_Dump(dbXA, 2); */

          reqAccountId=AB_Account_GetAccountNumber(jd->account);
          reqBankCode=AB_Account_GetBankCode(jd->account);
          reqAccountSuffix=AB_Account_GetSubAccountId(jd->account);
          if (reqAccountSuffix==NULL)
            reqAccountSuffix="";

          while (dbAccount) {
            accountId=GWEN_DB_GetCharValue(dbAccount, "accountId", 0, 0);
            accountSuffix=GWEN_DB_GetCharValue(dbAccount, "accountsubid", 0, 0);
            if (accountSuffix==NULL)
              accountSuffix="";
            bankCode=GWEN_DB_GetCharValue(dbAccount, "bankCode", 0, 0);
            sSepa=GWEN_DB_GetCharValue(dbAccount, "sepa", 0, "n");

            DBG_DEBUG(AQHBCI_LOGDOMAIN,
                      "- checking this account: bc=%s, an=%s, as=%s, sepa=%s (searching bc=%s, an=%s, as=%s)",
                      bankCode, accountId, accountSuffix, sSepa,
                      reqBankCode, reqAccountId, reqAccountSuffix);
            if (
              (bankCode && reqBankCode && 0==strcasecmp(bankCode, reqBankCode)) &&
              (accountId && reqAccountId && 0==strcasecmp(accountId, reqAccountId)) &&
              (accountSuffix && reqAccountSuffix && 0==strcasecmp(accountSuffix, reqAccountSuffix)) &&
              (strcasecmp(sSepa, "j")==0)
            ) {
              /* matching account found, break */
              DBG_INFO(AQHBCI_LOGDOMAIN, "Found matching account in GetAccountSepaInfoResponse");
              break;
            }
            dbAccount=GWEN_DB_FindNextGroup(dbAccount, "account");
          } /* while */
        }

        if (dbAccount) {
          const char *iban;
          const char *bic;
          int useWithSepa=0;

          /* account data found */
          DBG_INFO(AQHBCI_LOGDOMAIN, "Found a GetAccountSepaInfoResponse segment");
          accountId=GWEN_DB_GetCharValue(dbAccount, "accountId", 0, 0);
          accountSuffix=GWEN_DB_GetCharValue(dbAccount, "accountsubid", 0, 0);
          bankCode=GWEN_DB_GetCharValue(dbAccount, "bankCode", 0, 0);
          sSepa=GWEN_DB_GetCharValue(dbAccount, "sepa", 0, "n");
          if (strcasecmp(sSepa, "j")==0)
            useWithSepa=1;

          iban=GWEN_DB_GetCharValue(dbAccount, "iban", 0, 0);
          bic=GWEN_DB_GetCharValue(dbAccount, "bic", 0, 0);

          rv=AB_Provider_BeginExclUseAccount(pro, jd->account);
          if (rv<0) {
            DBG_ERROR(AQHBCI_LOGDOMAIN, "Unable to lock account");
          }
          else {
            if (accountSuffix)
              AB_Account_SetSubAccountId(jd->account, accountSuffix);

            if (useWithSepa) {
              DBG_INFO(AQHBCI_LOGDOMAIN, "SEPA available with this account");
              AH_Account_AddFlags(jd->account, AH_BANK_FLAGS_SEPA); /* we have a sub id (even if emtpy), set flag */
            }
            else {
              DBG_INFO(AQHBCI_LOGDOMAIN, "SEPA not available with this account");
              AH_Account_SubFlags(jd->account, AH_BANK_FLAGS_SEPA); /* we have a sub id (even if emtpy), set flag */
            }

            if (iban && *iban) {
              if (bic && *bic) {
                DBG_NOTICE(AQHBCI_LOGDOMAIN, "Setting IBAN and BIC: %s/%s", iban, bic);
                AB_Account_SetIban(jd->account, iban);
                AB_Account_SetBic(jd->account, bic);
              }
              else {
                DBG_NOTICE(AQHBCI_LOGDOMAIN, "Setting IBAN (no BIC): %s", iban);
                AB_Account_SetIban(jd->account, iban);
              }
            }
            else {
              DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing information in account: BLZ=[%s], Kto=[%s], IBAN=[%s], BIC=[%s]",
                        bankCode?bankCode:"",
                        accountId?accountId:"",
                        iban?iban:"",
                        bic?bic:"");
            }
            AB_Provider_EndExclUseAccount(pro, jd->account, 0);
          }
        }
      } /* if dbAccount */
      else {
        const char *accountId;
        const char *bankCode;
        const char *accountSuffix;
        const char *sSepa;
        const char *iban;
        const char *bic;
        int useWithSepa=0;

        /* account data found */
        DBG_INFO(AQHBCI_LOGDOMAIN, "Found a GetAccountSepaInfoResponse segment");
        accountId=GWEN_DB_GetCharValue(dbXA, "accountid", 0, 0);
        accountSuffix=GWEN_DB_GetCharValue(dbXA, "accountsubid", 0, 0);
        bankCode=GWEN_DB_GetCharValue(dbXA, "bankcode", 0, 0);
        sSepa=GWEN_DB_GetCharValue(dbXA, "sepa", 0, "n");
        if (strcasecmp(sSepa, "j")==0)
          useWithSepa=1;

        iban=GWEN_DB_GetCharValue(dbXA, "iban", 0, 0);
        bic=GWEN_DB_GetCharValue(dbXA, "bic", 0, 0);

        DBG_INFO(AQHBCI_LOGDOMAIN, "Update Account");
        rv=AB_Provider_BeginExclUseAccount(pro, jd->account);
        if (rv<0) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Unable to lock account");
        }
        else {
          if (accountSuffix)
            AB_Account_SetSubAccountId(jd->account, accountSuffix);

          if (useWithSepa) {
            DBG_INFO(AQHBCI_LOGDOMAIN, "SEPA available with this account");
            AH_Account_AddFlags(jd->account, AH_BANK_FLAGS_SEPA); /* we have SEPA info, set flag */
          }
          else {
            DBG_INFO(AQHBCI_LOGDOMAIN, "SEPA not available with this account");
            AH_Account_SubFlags(jd->account, AH_BANK_FLAGS_SEPA);
          }

          if (iban && *iban) {
            if (bic && *bic) {
              DBG_NOTICE(AQHBCI_LOGDOMAIN, "Setting IBAN and BIC: %s/%s", iban, bic);
              AB_Account_SetIban(jd->account, iban);
              AB_Account_SetBic(jd->account, bic);
            }
            else {
              DBG_NOTICE(AQHBCI_LOGDOMAIN, "Setting IBAN (no BIC): %s", iban);
              AB_Account_SetIban(jd->account, iban);
            }
          }
          else {
            DBG_ERROR(AQHBCI_LOGDOMAIN, "Missing information in account: BLZ=[%s], Kto=[%s], IBAN=[%s], BIC=[%s]",
                      bankCode?bankCode:"",
                      accountId?accountId:"",
                      iban?iban:"",
                      bic?bic:"");
          }
          AB_Provider_EndExclUseAccount(pro, jd->account, 0);

          DBG_INFO(AQHBCI_LOGDOMAIN, "Write AccountSpec");
          rv=AB_Provider_WriteAccountSpecForAccount(pro, jd->account, 0);
          if (rv<0) {
            DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
          }
        }
      } /* if (dbAccount) */
    } /* if (dbXA) */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while dbCurr */

  return 0;
}




