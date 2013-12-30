/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOB_P_H
#define AQBANKING_JOB_P_H


#include "job_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/list2.h>


struct AB_JOB {
  GWEN_INHERIT_ELEMENT(AB_JOB)
  GWEN_LIST_ELEMENT(AB_JOB)
  AB_ACCOUNT *account;
  AB_JOB_STATUS status;
  char *resultText;
  AB_JOB_TYPE jobType;
  int availability;
  uint32_t jobId;
  uint32_t usage;
  char *createdBy;
  uint32_t idForProvider;
  GWEN_DB_NODE *dbData;
  GWEN_TIME *lastStatusChange;
  char *usedTan;

  AB_TRANSACTION *transaction;
  AB_TRANSACTION_LIMITS *limits;
};
static AB_JOB *AB_Job__clearAll_cb(AB_JOB *j, void *userData);



#endif /* AQBANKING_JOB_P_H */
