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


#ifndef AQBANKING_PROVIDER_P_H
#define AQBANKING_PROVIDER_P_H

#include "provider_l.h"
#include <aqbanking/banking.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/plugin.h>



struct AB_PROVIDER {
  GWEN_INHERIT_ELEMENT(AB_PROVIDER)
  GWEN_LIST_ELEMENT(AB_PROVIDER)

  AB_BANKING *banking;
  char *name;
  char *escName;

  AB_PROVIDER_INIT_FN initFn;
  AB_PROVIDER_FINI_FN finiFn;
  AB_PROVIDER_UPDATEJOB_FN updateJobFn;
  AB_PROVIDER_ADDJOB_FN addJobFn;
  AB_PROVIDER_EXECUTE_FN executeFn;
  AB_PROVIDER_RESETQUEUE_FN resetQueueFn;

  AB_PROVIDER_EXTEND_USER_FN extendUserFn;
  AB_PROVIDER_EXTEND_ACCOUNT_FN extendAccountFn;

  AB_PROVIDER_UPDATE_FN updateFn;

  GWEN_PLUGIN *plugin;

  uint32_t usage;
  uint32_t flags;
  int isInit;
};



#endif /* AQBANKING_PROVIDER_P_H */
