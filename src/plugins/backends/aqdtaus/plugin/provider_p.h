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

#include "provider_l.h"
#include "job_l.h"
#include <aqbanking/transaction.h>
#include <gwenhywfar/waitcallback.h>


typedef struct AD_PROVIDER AD_PROVIDER;
struct AD_PROVIDER {
  AD_JOB_LIST *myJobs;
  AB_JOB_LIST2 *bankingJobs;
  GWEN_DB_NODE *dbConfig;
  GWEN_TYPE_UINT32 lastJobId;
};

static void GWENHYWFAR_CB AD_Provider_FreeData(void *bp, void *p);

static int AD_Provider_AddTransfer(AB_PROVIDER *pro,
                                   AB_ACCOUNT *acc,
                                   const AB_TRANSACTION *t,
                                   GWEN_TYPE_UINT32 *jid);

static int AD_Provider_AddDebitNote(AB_PROVIDER *pro,
                                    AB_ACCOUNT *acc,
                                    const AB_TRANSACTION *t,
                                    GWEN_TYPE_UINT32 *jid);

static AD_JOB *AD_Provider_FindMyJob(AB_PROVIDER *pro, GWEN_TYPE_UINT32 jid);

static int AD_Provider_ExecCommand(AB_PROVIDER *pro, const char *cmd);

static int AD_Provider_CheckEmptyDir(const char *path);

static int AD_Provider__WriteDTAUS(AB_PROVIDER *pro,
                                   const char *path,
                                   GWEN_BUFFER *buf);

static int AD_Provider_WriteDTAUS(AB_PROVIDER *pro,
                                  AB_ACCOUNT *da,
                                  GWEN_BUFFER *buf);
static int AD_Provider_SaveJob(AB_PROVIDER *pro, AD_JOB *dj, GWEN_BUFFER *data);


static int AD_Provider_MountDisc(AB_PROVIDER *pro, AB_ACCOUNT *da);
static int AD_Provider_UnmountDisc(AB_PROVIDER *pro, AB_ACCOUNT *da);

static int AD_Provider_ProcessJob(AB_PROVIDER *pro, AD_JOB *dj);

static int AD_Provider_PrintBegleitZettel(AB_PROVIDER *pro,
                                          AD_JOB *dj,
                                          GWEN_DB_NODE *dbTransfers);

static int AD_Provider__Execute(AB_PROVIDER *pro);


static int AD_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int AD_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData);
static int AD_Provider_UpdateJob(AB_PROVIDER *pro, AB_JOB *j);
static int AD_Provider_AddJob(AB_PROVIDER *pro, AB_JOB *j);
static int AD_Provider_Execute(AB_PROVIDER *pro, AB_IMEXPORTER_CONTEXT *ctx);
static int AD_Provider_ResetQueue(AB_PROVIDER *pro);
static int AD_Provider_ExtendAccount(AB_PROVIDER *pro, AB_ACCOUNT *a,
                                     AB_PROVIDER_EXTEND_MODE em);


#endif

