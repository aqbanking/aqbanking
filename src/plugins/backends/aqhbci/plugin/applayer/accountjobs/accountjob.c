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


#include "accountjob_p.h"
#include "aqhbci_l.h"
#include "job_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/gwentime.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_INHERIT(AH_JOB, AH_ACCOUNTJOB);


/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_AccountJob_new(const char *name,
                          AH_CUSTOMER *cu,
                          AH_ACCOUNT *account){
  AH_ACCOUNTJOB *aj;
  AH_BANK *b;
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  assert(name);
  assert(cu);
  assert(account);
  j=AH_Job_new(name, cu, AH_Account_GetAccountId(account));
  if (!j)
    return 0;

  GWEN_NEW_OBJECT(AH_ACCOUNTJOB, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_ACCOUNTJOB, j, aj, AH_AccountJob_FreeData);
  aj->account=account;
  AH_Account_Attach(aj->account);

  /* set some known arguments */
  b=AH_Account_GetBank(account);
  assert(b);
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                       "accountId",
                       AH_Account_GetAccountId(account));
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                       "bankCode",
                       AH_Account_GetBankId(account));
  GWEN_DB_SetIntValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
                      "country",
                      AH_Bank_GetCountry(b));

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
AH_ACCOUNT *AH_AccountJob_GetAccount(const AH_JOB *j){
  AH_ACCOUNTJOB *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_ACCOUNTJOB, j);
  assert(aj);

  return aj->account;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_AccountJob_FreeData(void *bp, void *p) {
  AH_ACCOUNTJOB *aj;

  aj=(AH_ACCOUNTJOB*)p;
  AH_Account_free(aj->account);
  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_AccountJob_AddCurrentTime(GWEN_BUFFER *buf) {
  GWEN_TIME *t;
  int hours, mins, secs;
  char numbuf[16];

  t=GWEN_CurrentTime();
  assert(t);
  if (GWEN_Time_GetBrokenDownTime(t, &hours, &mins, &secs)) {
    GWEN_Time_free(t);
    return -1;
  }
  snprintf(numbuf, sizeof(numbuf), "%02d%02d%02d", hours, mins, secs);
  GWEN_Buffer_AppendString(buf, numbuf);
  GWEN_Time_free(t);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_AccountJob_AddCurrentDate(GWEN_BUFFER *buf) {
  GWEN_TIME *t;
  int year, month, day;
  char numbuf[16];

  t=GWEN_CurrentTime();
  assert(t);
  if (GWEN_Time_GetBrokenDownDate(t, &day, &month, &year)) {
    GWEN_Time_free(t);
    return -1;
  }
  snprintf(numbuf, sizeof(numbuf), "%04d%02d%02d", year, month, day);
  GWEN_Buffer_AppendString(buf, numbuf);
  GWEN_Time_free(t);
  return 0;
}









