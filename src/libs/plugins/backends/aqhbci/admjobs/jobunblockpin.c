/***************************************************************************
    begin       : Mon Mar 16 2020
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobunblockpin_l.h"



AH_JOB *AH_Job_UnblockPin_new(AB_PROVIDER *pro, AB_USER *u)
{
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  assert(u);
  j=AH_Job_new("JobUnblockPin", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobUnblockPin not supported, should not happen");
    return NULL;
  }

  /* set arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobUnblockPin created");
  return j;
}


