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

#ifndef AD_JOB_P_H
#define AD_JOB_P_H

#include "job_l.h"


struct AD_JOB {
  GWEN_LIST_ELEMENT(AD_JOB);
  AB_ACCOUNT *account;
  AB_TRANSACTION_LIST2 *transactions;
  int isDebitNote;
  uint32_t jobId;

  int resultCode;
  char *resultText;
};



#endif
