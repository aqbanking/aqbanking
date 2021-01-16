/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_CBOX_P_H
#define AH_OUTBOX_CBOX_P_H


#include "jobqueue_l.h"
#include "outbox_l.h"
#include "cbox_itan.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>



/** Customer's outbox */
struct AH_OUTBOX_CBOX {
  GWEN_LIST_ELEMENT(AH_OUTBOX_CBOX);
  AH_OUTBOX *outbox;
  AB_PROVIDER *provider;
  AB_USER *user;
  AH_JOBQUEUE_LIST *todoQueues;

  AH_JOB_LIST *todoJobs;
  AH_JOB_LIST *finishedJobs;

  uint32_t usage;
};


#endif /* AH_OUTBOX_CBOX_P_H */





