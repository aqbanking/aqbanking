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

#ifndef AO_PROVIDER_P_H
#define AO_PROVIDER_P_H


#define AO_PROVIDER_CONNECT_TIMEOUT 15
#define AO_PROVIDER_SEND_TIMEOUT    10
#define AO_PROVIDER_RECV_TIMEOUT    60

#include "queues_l.h"
#include "context_l.h"
#include <aqofxconnect/provider.h>
#include <aqbanking/transaction.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/netlayer.h>



struct AO_PROVIDER {
  GWEN_DB_NODE *dbConfig;

  int connectTimeout;
  int sendTimeout;
  int recvTimeout;
  GWEN_TYPE_UINT32 lastJobId;

  AO_QUEUE *queue;
  AB_JOB_LIST2 *bankingJobs;
};

void AO_Provider_FreeData(void *bp, void *p);

int AO_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AO_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AO_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
int AO_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
int AO_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx);
int AO_Provider_ResetQueue(AB_PROVIDER *pro);


int AO_Provider_EncodeJob(AB_PROVIDER *pro,
                          AO_CONTEXT *ctx,
                          char **pData);

void AO_Provider_AddBankCertFolder(AB_PROVIDER *pro,
                                   const AB_USER *u,
                                   GWEN_BUFFER *nbuf);

int AO_Provider_SendMessage(AB_PROVIDER *pro,
                            AB_USER *u,
                            GWEN_NETLAYER *nl,
                            const char *p,
                            unsigned int plen);

int AO_Provider_SendAndReceive(AB_PROVIDER *pro,
                               AB_USER *u,
                               const char *p,
                               unsigned int plen,
                               GWEN_BUFFER **rbuf);

int AO_Provider_RequestStatements(AB_PROVIDER *pro, AB_JOB *j,
                                  AB_IMEXPORTER_CONTEXT *ictx);

int AO_Provider_DistributeContext(AB_PROVIDER *pro,
                                  AB_JOB *refJob,
                                  AB_IMEXPORTER_CONTEXT *ictx);


int AO_Provider_ExecUserQueue(AB_PROVIDER *pro, AO_USERQUEUE *uq);
int AO_Provider_ExecQueue(AB_PROVIDER *pro);

int AO_Provider_CountDoneJobs(AB_JOB_LIST2 *jl);
AB_JOB *AO_Provider_FindJobById(AB_JOB_LIST2 *jl, GWEN_TYPE_UINT32 jid);




#endif

