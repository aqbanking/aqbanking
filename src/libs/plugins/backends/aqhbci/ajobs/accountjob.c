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
#include "aqhbci/aqhbci_l.h"
#include "aqhbci/joblayer/job_l.h"
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


/* none for now */


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

  assert(name);
  assert(u);
  assert(account);

  j=AH_Job_new(name, pro, u, account, 0);
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



