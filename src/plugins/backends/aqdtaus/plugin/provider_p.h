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

#ifndef AD_PROVIDER_P_H
#define AD_PROVIDER_P_H

#define AD_DTAUS_WCB_GENERIC "AD_DTAUS_WCB_GENERIC"
#define AD_OUTBOX_EXECUTE_WCB_ID "AD_OUTBOX_EXECUTE_WCB_ID"


#include <aqdtaus/provider.h>
#include <aqdtaus/job.h>
#include <aqbanking/transaction.h>
#include <gwenhywfar/waitcallback.h>


struct AD_PROVIDER {
  AB_ACCOUNT_LIST2 *accounts;
  AD_JOB_LIST *myJobs;
  AB_JOB_LIST2 *bankingJobs;
  GWEN_DB_NODE *dbConfig;
  GWEN_TYPE_UINT32 lastJobId;
};

void AD_Provider_FreeData(void *bp, void *p);

int AD_Provider_AddTransfer(AB_PROVIDER *pro,
                            AB_ACCOUNT *acc,
                            const AB_TRANSACTION *t,
                            GWEN_TYPE_UINT32 *jid);

int AD_Provider_AddDebitNote(AB_PROVIDER *pro,
                             AB_ACCOUNT *acc,
                             const AB_TRANSACTION *t,
                             GWEN_TYPE_UINT32 *jid);

AB_ACCOUNT *AD_Provider_FindMyAccount(AB_PROVIDER *pro,
                                      AB_ACCOUNT *acc);
AD_JOB *AD_Provider_FindMyJob(AB_PROVIDER *pro, GWEN_TYPE_UINT32 jid);

int AD_Provider_ExecCommand(AB_PROVIDER *pro, const char *cmd);

int AD_Provider_CheckEmptyDir(const char *path);

int AD_Provider__WriteDTAUS(AB_PROVIDER *pro,
                            const char *path,
                            GWEN_BUFFER *buf);

int AD_Provider_WriteDTAUS(AB_PROVIDER *pro,
                           AB_ACCOUNT *da,
                           GWEN_BUFFER *buf);
int AD_Provider_SaveJob(AB_PROVIDER *pro, AD_JOB *dj, GWEN_BUFFER *data);


int AD_Provider_MountDisc(AB_PROVIDER *pro, AB_ACCOUNT *da);
int AD_Provider_UnmountDisc(AB_PROVIDER *pro, AB_ACCOUNT *da);

int AD_Provider_ProcessJob(AB_PROVIDER *pro, AD_JOB *dj);

int AD_Provider_PrintBegleitZettel(AB_PROVIDER *pro,
                                   AD_JOB *dj,
                                   GWEN_DB_NODE *dbTransfers);

int AD_Provider__Execute(AB_PROVIDER *pro);


int AD_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AD_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AD_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
int AD_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
int AD_Provider_Execute(AB_PROVIDER *pro);
int AD_Provider_ResetQueue(AB_PROVIDER *pro);
AB_ACCOUNT_LIST2 *AD_Provider_GetAccountList(AB_PROVIDER *pro);
int AD_Provider_UpdateAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);


#endif

