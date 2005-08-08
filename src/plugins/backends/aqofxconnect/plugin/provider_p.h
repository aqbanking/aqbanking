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
#include <aqofxconnect/bank.h>
#include <aqbanking/transaction.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/netconnectionhttp.h>



struct AO_PROVIDER {
  AO_BANK_LIST *banks;
  GWEN_DB_NODE *dbConfig;

  int connectTimeout;
  int sendTimeout;
  int recvTimeout;
  GWEN_TYPE_UINT32 lastJobId;

  AO_BANKQUEUE_LIST *bankQueues;
  AB_JOB_LIST2 *bankingJobs;

  GWEN_TYPE_UINT32 libId;
};

void AO_Provider_FreeData(void *bp, void *p);

int AO_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AO_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
int AO_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
int AO_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
int AO_Provider_Execute(AB_PROVIDER *pro);
int AO_Provider_ResetQueue(AB_PROVIDER *pro);
AB_ACCOUNT_LIST2 *AO_Provider_GetAccountList(AB_PROVIDER *pro);
int AO_Provider_UpdateAccount(AB_PROVIDER *pro, AB_ACCOUNT *a);

AB_ACCOUNT *AO_Provider_FindMyAccountByAccount(AB_PROVIDER *pro,
                                               AB_ACCOUNT *ba);

int AO_Provider_EncodeJob(AB_PROVIDER *pro,
                          AO_CONTEXT *ctx,
                          char **pData);

int AO_Provider_ExtractHttpResponse(AB_PROVIDER *pro,
                                    GWEN_NETMSG *netmsg,
                                    GWEN_BUFFER *dbuf);
void AO_Provider_AddBankCertFolder(AB_PROVIDER *pro,
                                   const AO_BANK *b,
                                   GWEN_BUFFER *nbuf);

GWEN_NETCONNECTION *AO_Provider_CreateConnection(AB_PROVIDER *pro,
                                                 AO_USER *u);
int AO_Provider_SendMessage(AB_PROVIDER *pro,
                            AO_USER *u,
                            GWEN_NETCONNECTION *conn,
                            const char *p,
                            unsigned int plen);

int AO_Provider_SendAndReceive(AB_PROVIDER *pro,
                               AO_USER *u,
                               const char *p,
                               unsigned int plen,
                               GWEN_BUFFER *rbuf);

int AO_Provider_RequestStatements(AB_PROVIDER *pro, AB_JOB *j,
                                  AB_IMEXPORTER_CONTEXT *ictx);

int AO_Provider_DistributeContext(AB_PROVIDER *pro,
                                  AB_JOB *refJob,
                                  AB_IMEXPORTER_CONTEXT *ictx);


int AO_Provider_ExecUserQueue(AB_PROVIDER *pro, AO_USERQUEUE *uq);
int AO_Provider_ExecBankQueue(AB_PROVIDER *pro, AO_BANKQUEUE *bq);

int AO_Provider_CountDoneJobs(AB_JOB_LIST2 *jl);
AB_JOB *AO_Provider_FindJobById(AB_JOB_LIST2 *jl, GWEN_TYPE_UINT32 jid);




#endif

