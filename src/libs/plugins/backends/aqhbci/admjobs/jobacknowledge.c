/***************************************************************************
    begin       : Sat Mar 20 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobacknowledge_l.h"



AH_JOB *AH_Job_Acknowledge_new(AB_PROVIDER *pro,
                               AB_USER *u,
                               const uint8_t *ptrAckCode,
                               uint32_t lenAckCode)
{
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  assert(u);
  j=AH_Job_new("JobAcknowledge", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobAcknowledge not supported, should not happen");
    return NULL;
  }

  /* set arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  GWEN_DB_SetBinValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "ackCode", ptrAckCode, lenAckCode);

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobAcknowledge created");
  return j;
}


