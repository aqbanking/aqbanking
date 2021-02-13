/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_OUTBOX_P_H
#define AH_OUTBOX_P_H


#include "aqhbci/applayer/outbox_l.h"

#include "aqhbci/joblayer/jobqueue_l.h"
#include "aqhbci/applayer/cbox.h"

#include <gwenhywfar/inherit.h>


struct AH_OUTBOX {
  GWEN_INHERIT_ELEMENT(AH_OUTBOX);
  AB_PROVIDER *provider;
  AH_OUTBOX_CBOX_LIST *userBoxes;
  AH_JOB_LIST *finishedJobs;
  AB_IMEXPORTER_CONTEXT *context;

  uint32_t usage;
};



#endif /* AH_OUTBOX_P_H */





