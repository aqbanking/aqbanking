/***************************************************************************
    begin       : Thu Jan 31 2019
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobgetsepainfo_l.h"

#include "aqhbci/joblayer/job_crypt.h"


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


AH_JOB *AH_Job_GetAccountSepaInfo_new(AB_PROVIDER *pro, AB_USER *u)
{
  AH_JOB *j;

  assert(u);
  j=AH_Job_new("JobGetAccountSepaInfo", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobGetAccountSepaInfo not supported, should not happen");
    return 0;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobGetAccountSepaInfo created");
  return j;
}



