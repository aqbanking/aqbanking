/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBQUEUE_P_H
#define AH_JOBQUEUE_P_H

#include "jobqueue_l.h"
#include "job_l.h"
#include <gwenhywfar/stringlist.h>



struct AH_JOBQUEUE {
  GWEN_LIST_ELEMENT(AH_JOBQUEUE);

  AB_USER *user;
  GWEN_STRINGLIST *signers;
  uint32_t usage;
  AH_JOB_LIST *jobs;
  uint32_t flags;

  int secProfile;
  int secClass;

  char *usedTan;
  char *usedPin; /* used for checking bad pins in AH_JobQueue_DispatchMessage */

  AH_JOBQUEUE *referenceQueue;

  GWEN_DB_NODE *dbAllResponses;
};



#endif /* AH_JOBQUEUE_P_H */




