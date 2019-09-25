/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/*
 * This file is included by job.c
 */




/* ========================================================================
 * Virtual Functions
 * ======================================================================== */



int AH_Job_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{

  assert(j);
  assert(j->usage);

  AH_Job_SampleResults(j);

  if (j->processFn)
    return j->processFn(j, ctx);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No processFn set");
    return AH_Job_DefaultProcessHandler(j);
  }
}



int AH_Job_Commit(AH_JOB *j, int doLock)
{
  assert(j);
  assert(j->usage);
  if (j->commitFn)
    return j->commitFn(j, doLock);
  else {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No commitFn set");
    return AH_Job_DefaultCommitHandler(j, doLock);
  }
}



int AH_Job_Prepare(AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  if (j->prepareFn)
    return j->prepareFn(j);
  else {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No prepareFn set");
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int AH_Job_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod)
{
  assert(j);
  assert(j->usage);
  if (j->addChallengeParamsFn)
    return j->addChallengeParamsFn(j, hkTanVer, dbMethod);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No addChallengeParamsFn set");
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int AH_Job_GetLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits)
{
  assert(j);
  assert(j->usage);
  if (j->getLimitsFn)
    return j->getLimitsFn(j, pLimits);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No getLimitsFn set");
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int AH_Job_HandleCommand(AH_JOB *j, const AB_TRANSACTION *t)
{
  assert(j);
  assert(j->usage);
  if (j->handleCommandFn)
    return j->handleCommandFn(j, t);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No handleCommandFn set");
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int AH_Job_HandleResults(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  assert(j);
  assert(j->usage);
  if (j->handleResultsFn)
    return j->handleResultsFn(j, ctx);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No handleResultsFn set in job \"%s\"", (j->name)?(j->name):"(unnamed)");
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}





/* ========================================================================
 * Setters for Virtual Functions
 * ======================================================================== */



void AH_Job_SetProcessFn(AH_JOB *j, AH_JOB_PROCESS_FN f)
{
  assert(j);
  assert(j->usage);
  j->processFn=f;
}



void AH_Job_SetCommitFn(AH_JOB *j, AH_JOB_COMMIT_FN f)
{
  assert(j);
  assert(j->usage);
  j->commitFn=f;
}



void AH_Job_SetNextMsgFn(AH_JOB *j, AH_JOB_NEXTMSG_FN f)
{
  assert(j);
  assert(j->usage);
  j->nextMsgFn=f;
}



void AH_Job_SetPrepareFn(AH_JOB *j, AH_JOB_PREPARE_FN f)
{
  assert(j);
  assert(j->usage);
  j->prepareFn=f;
}



void AH_Job_SetAddChallengeParamsFn(AH_JOB *j, AH_JOB_ADDCHALLENGEPARAMS_FN f)
{
  assert(j);
  assert(j->usage);
  j->addChallengeParamsFn=f;
}



void AH_Job_SetGetLimitsFn(AH_JOB *j, AH_JOB_GETLIMITS_FN f)
{
  assert(j);
  assert(j->usage);
  j->getLimitsFn=f;
}



void AH_Job_SetHandleCommandFn(AH_JOB *j, AH_JOB_HANDLECOMMAND_FN f)
{
  assert(j);
  assert(j->usage);
  j->handleCommandFn=f;
}



void AH_Job_SetHandleResultsFn(AH_JOB *j, AH_JOB_HANDLERESULTS_FN f)
{
  assert(j);
  assert(j->usage);
  j->handleResultsFn=f;
}





/* ========================================================================
 * Defaults or Offers for Virtual Functions
 * ======================================================================== */



int AH_Job_DefaultProcessHandler(AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  if (j->flags & AH_JOB_FLAGS_PROCESSED) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Already processed job \"%s\"", j->name);
    return 0;
  }
  return 0;
}



int AH_Job_DefaultCommitHandler(AH_JOB *j, int doLock)
{
  int rv;

  assert(j);
  assert(j->usage);
  if (j->flags & AH_JOB_FLAGS_COMMITTED) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Already committed job \"%s\"", j->name);
    return 0;
  }
  rv=AH_Job_CommitSystemData(j, doLock);
  j->flags|=AH_JOB_FLAGS_COMMITTED;
  return rv;
}



int AH_Job_GetLimits_EmptyLimits(AH_JOB *j, AB_TRANSACTION_LIMITS **pLimits)
{
  AB_TRANSACTION_LIMITS *tl;

  tl=AB_TransactionLimits_new();
  AB_TransactionLimits_SetCommand(tl, AH_Job_GetSupportedCommand(j));
  *pLimits=tl;
  return 0;
}



int AH_Job_HandleCommand_Accept(AH_JOB *j, const AB_TRANSACTION *t)
{
  return 0;
}



int AH_Job_HandleResults_Empty(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_RESULT_LIST *rl;
  AH_RESULT *r;
  AB_TRANSACTION_STATUS tStatus;

  assert(j);
  assert(j->usage);

  rl=AH_Job_GetSegResults(j);
  assert(rl);

  r=AH_Result_List_First(rl);
  if (!r) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No segment results");
    tStatus=AB_Transaction_StatusError;
  }
  else {
    int has10=0;
    int has20=0;

    while (r) {
      int rcode;

      rcode=AH_Result_GetCode(r);
      if (rcode>=10 && rcode<=19) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Has10: %d (%s)", rcode, AH_Result_GetText(r));
        has10=1;
      }
      else if (rcode>=20 && rcode <=29) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Has20: %d (%s)", rcode, AH_Result_GetText(r));
        has20=1;
      }
      else {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Other: %d (%s)", rcode, AH_Result_GetText(r));
      }
      r=AH_Result_List_Next(r);
    }

    if (has20)
      tStatus=AB_Transaction_StatusAccepted;
    else if (has10)
      tStatus=AB_Transaction_StatusPending;
    else
      tStatus=AB_Transaction_StatusRejected;
  }

  AH_Job_SetStatusOnCommands(j, tStatus);
  return 0;
}





