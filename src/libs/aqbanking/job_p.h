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


#ifndef AQBANKING_JOB_P_H
#define AQBANKING_JOB_P_H


#include "job_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc2.h>


struct AB_JOB {
  GWEN_INHERIT_ELEMENT(AB_JOB);
  GWEN_LIST_ELEMENT(AB_JOB);
  AB_ACCOUNT *account;
  AB_JOB_STATUS status;
  char *resultText;
  AB_JOB_TYPE jobType;
  GWEN_DB_NODE *data;
  int availability;
  GWEN_TYPE_UINT32 usage;
};
AB_JOB *AB_Job__freeAll_cb(AB_JOB *j);




#endif /* AQBANKING_JOB_P_H */
