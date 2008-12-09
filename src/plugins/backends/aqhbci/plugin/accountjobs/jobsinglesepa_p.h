/***************************************************************************
    begin       : Sat Nov 29 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSINGLESEPA_P_H
#define AH_JOBSINGLESEPA_P_H


#include "jobsinglesepa_l.h"
#include <gwenhywfar/db.h>


typedef struct AH_JOB_SINGLESEPA AH_JOB_SINGLESEPA;
struct AH_JOB_SINGLESEPA {
  AB_JOB_TYPE jobType;
  char *fiid;
  char *oldFiid;
};
static void GWENHYWFAR_CB AH_Job_SingleSepa_FreeData(void *bp, void *p);
static int AH_Job_SingleSepa_Process(AH_JOB *j,
				     AB_IMEXPORTER_CONTEXT *ctx,
				     uint32_t guiid);
static int AH_Job_SingleSepa_Exchange(AH_JOB *j, AB_JOB *bj,
				      AH_JOB_EXCHANGE_MODE m,
				      AB_IMEXPORTER_CONTEXT *ctx,
				      uint32_t guiid);


static int AH_Job_SingleSepa__ValidateTransfer(AB_JOB *bj,
					       AH_JOB *mj,
					       AB_TRANSACTION *t);

static AH_JOB *AH_Job_SingleSepaBase_new(AB_USER *u,
					 AB_ACCOUNT *account,
					 AB_JOB_TYPE jobType);

#endif /* AH_JOBSINGLETRANSFER_P_H */


