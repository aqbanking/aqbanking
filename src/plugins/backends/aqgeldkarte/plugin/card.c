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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "card_p.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(AG_CARD, AG_Card)




AG_CARD *AG_Card_new(AB_ACCOUNT *acc){
  AG_CARD *dj;

  assert(acc);
  GWEN_NEW_OBJECT(AG_CARD, dj);
  GWEN_LIST_INIT(AG_CARD, dj);
  dj->bankingJobs=AB_Job_List2_new();
  dj->account=acc;
  return dj;
}



void AG_Card_free(AG_CARD *dj){
  if (dj) {
    GWEN_LIST_FINI(AG_CARD, dj);
    AB_Job_List2_free(dj->bankingJobs);
    GWEN_FREE_OBJECT(dj);
  }
}



AB_ACCOUNT *AG_Card_GetAccount(const AG_CARD *dj){
  assert(dj);
  return dj->account;
}



AB_JOB_LIST2 *AG_Card_GetBankingJobs(const AG_CARD *dj) {
  assert(dj);
  return dj->bankingJobs;
}



void AG_Card_AddJob(AG_CARD *dj, AB_JOB *bj) {
  assert(dj);
  assert(bj);
  AB_Job_List2_PushBack(dj->bankingJobs, bj);
}




