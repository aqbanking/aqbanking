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


#ifndef AQBANKING_BANKING_P_H
#define AQBANKING_BANKING_P_H


#include "banking_l.h"
#include "account_l.h"
#include "job_l.h"


struct AB_BANKING {
  GWEN_INHERIT_ELEMENT(AB_BANKING);
  char *appName;
  AB_JOB_LIST *enqueuedJobs;
  AB_ACCOUNT_LIST *accounts;
  GWEN_TYPE_UINT32 lastUniqueId;

  GWEN_STRINGLIST *activeProviders;

  char *configFile;

  GWEN_DB_NODE *data;

  AB_PROVIDER_LIST *providers;
  AB_PROVIDER_WIZARD_LIST *wizards;

  AB_BANKING_MESSAGEBOX_FN messageBoxFn;
  AB_BANKING_INPUTBOX_FN inputBoxFn;
  AB_BANKING_SHOWBOX_FN showBoxFn;
  AB_BANKING_HIDEBOX_FN hideBoxFn;
  AB_BANKING_PROGRESS_START_FN progressStartFn;
  AB_BANKING_PROGRESS_ADVANCE_FN progressAdvanceFn;
  AB_BANKING_PROGRESS_LOG_FN progressLogFn;
  AB_BANKING_PROGRESS_END_FN progressEndFn;


};



AB_PROVIDER *AB_Banking_FindProvider(AB_BANKING *ab, const char *name);
AB_PROVIDER_WIZARD *AB_Banking_FindWizard(AB_BANKING *ab,
                                          AB_PROVIDER *pro,
                                          const char *name);

int AB_Banking__MergeInAccount(AB_BANKING *ab, AB_ACCOUNT *a);



#endif /* AQBANKING_BANKING_P_H */
