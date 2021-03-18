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


#include "accountjob_p.h"
#include "aqhbci_l.h"
#include "job_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/gui.h>

#include <assert.h>



GWEN_INHERIT(AH_JOB, AH_ACCOUNTJOB);


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _getJobVersionToUse(const char *name, AB_USER *u, const AB_ACCOUNT *account);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AH_JOB *AH_AccountJob_new(const char *name,
                          AB_PROVIDER *pro,
                          AB_USER *u,
                          AB_ACCOUNT *account)
{
  AH_ACCOUNTJOB *aj;
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;
  const char *s;
  int jobVersion=0;

  assert(name);
  assert(u);
  assert(account);

  /* this might later be removed, since all accounts should now have a suffix reported by the bank,
   * and if an account doesn't, it is just not needed anymore */
  jobVersion=_getJobVersionToUse(name, u, account);
  if (jobVersion<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return NULL;
  }

  j=AH_Job_new(name, pro, u, account, jobVersion);
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_ACCOUNTJOB, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_ACCOUNTJOB, j, aj, AH_AccountJob_FreeData);
  aj->account=account;

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  s=AB_Account_GetAccountNumber(account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "accountId", s);

  s=AB_Account_GetSubAccountId(account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "accountSubId", s);

  s=AB_Account_GetBankCode(account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "bankCode", s);

  GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                      "country", 280);

  /* new for SEPA jobs */
  s=AB_Account_GetIban(account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "iban", s);

  s=AB_Account_GetBic(account);
  if (s && *s)
    GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "bic", s);

  return j;
}



AB_ACCOUNT *AH_AccountJob_GetAccount(const AH_JOB *j)
{
  AH_ACCOUNTJOB *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_ACCOUNTJOB, j);
  assert(aj);

  return aj->account;
}



void GWENHYWFAR_CB AH_AccountJob_FreeData(void *bp, void *p)
{
  AH_ACCOUNTJOB *aj;

  aj=(AH_ACCOUNTJOB *)p;
  GWEN_FREE_OBJECT(aj);
}



int AH_AccountJob_IsAccountJob(const AH_JOB *j)
{
  return GWEN_INHERIT_ISOFTYPE(AH_JOB, AH_ACCOUNTJOB, j);
}



int _getJobVersionToUse(const char *name, AB_USER *u, const AB_ACCOUNT *account)
{
  int jobVersion=0;

  if (!(AH_Account_GetFlags(account) & AH_BANK_FLAGS_KTV2)) {
    int maxVer=0;

    /* no account suffix, so we try to determine the highest usable
     * version of the job which still doesn't need the suffix
     */
    DBG_NOTICE(AQHBCI_LOGDOMAIN,
               "No account suffix [%u], determining highest possible job version for \"%s\" (%08x)",
               AB_Account_GetUniqueId(account), name, AH_Account_GetFlags(account));
    if (strcasecmp(name, "JobGetTransactions")==0)
      maxVer=4;
    else if (strcasecmp(name, "JobGetBalance")==0)
      maxVer=4;
    else if (strcasecmp(name, "JobSingleTransfer")==0)
      maxVer=3;
    else if (strcasecmp(name, "JobSingleDebitNote")==0)
      maxVer=3;
    else if (strcasecmp(name, "JobInternalTransfer")==0 ||
             strcasecmp(name, "JobLoadCellPhone")==0)
      /* this job needs a suffix, so if there is none you don't get it */
      maxVer=-1;
    else if (strcasecmp(name, "JobGetDatedTransfers")==0)
      maxVer=1;
    else if (strcasecmp(name, "JobCreateDatedTransfer")==0)
      maxVer=2;
    else if (strcasecmp(name, "JobModifyDatedTransfer")==0)
      maxVer=2;
    else if (strcasecmp(name, "JobDeleteDatedTransfer")==0)
      maxVer=1;
    else if (strcasecmp(name, "JobCreateStandingOrder")==0)
      maxVer=2;
    else if (strcasecmp(name, "JobModifyStandingOrder")==0)
      maxVer=2;
    else if (strcasecmp(name, "JobDeleteStandingOrder")==0)
      maxVer=1;
    if (maxVer==-1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "This job needs an account suffix, but your bank didn't provide one. "
                "Therefore this job is not supported with your account.");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("This job needs an account suffix, but your bank did not provide one. "
                                "Therefore this job is not supported with your account.\n"
                                "Setting a higher HBCI version in the user settings might fix "
                                "the problem."));
      return GWEN_ERROR_GENERIC;
    }
    if (maxVer>0) {
      jobVersion=AH_Job_GetMaxVersionUpUntil(name, u, maxVer);
      if (jobVersion<1) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No job [%s] below version %d, falling back to 0", name, maxVer);
        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Warning,
                              "No version for job [%s] up to %d found, falling back to 0", name, maxVer);
        jobVersion=0;
      }
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Reducing version of job [%s] to %d", name, jobVersion);
      }
    }
  }

  return jobVersion;
}




