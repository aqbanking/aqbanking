/***************************************************************************
    begin       : Tue Jun 03 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/*
 * This file is included by provider.c
 */

#include "jobforeignxferwh_l.h"





int AH_Provider_SendDtazv(AB_PROVIDER *pro,
                          AB_USER *u,
                          AB_ACCOUNT *a,
                          AB_IMEXPORTER_CONTEXT *ctx,
                          const uint8_t *dataPtr,
                          uint32_t dataLen,
                          int withProgress, int nounmount, int doLock)
{
  AH_PROVIDER *hp;
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_JOB *job;
  AH_OUTBOX *ob;
  int rv;

  assert(pro);
  hp=GWEN_INHERIT_GETDATA(AB_PROVIDER, AH_PROVIDER, pro);
  assert(hp);

  assert(u);
  assert(a);

  /* gather all objects */
  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  job=AH_Job_ForeignTransferWH_new(pro, u, a);
  if (!job) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported, should not happen");
    return GWEN_ERROR_GENERIC;
  }

  rv=AH_Job_ForeignTransferWH_SetDtazv(job, dataPtr, dataLen);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(job);
    return rv;
  }

  AH_Job_AddSigner(job, AB_User_GetUserId(u));

  ob=AH_Outbox_new(pro);
  AH_Outbox_AddJob(ob, job);

  rv=AH_Outbox_Execute(ob, ctx, withProgress, 1, doLock);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not execute outbox.\n");
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    AH_Job_free(job);
    AH_Outbox_free(ob);
    return rv;
  }

  AH_Outbox_free(ob);

  if (AH_Job_HasErrors(job) || AH_Job_GetStatus(job)==AH_JobStatusError) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job has errors");
    // TODO: show errors
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_GENERIC;
  }

  AH_Job_free(job);

  if (!nounmount)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));

  return 0;
}



