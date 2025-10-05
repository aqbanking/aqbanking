/***************************************************************************
    begin       : Fri Oct 3 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobvpa.h"

#include "aqhbci/joblayer/job_crypt.h"



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */



/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */

AH_JOB *AH_Job_VPA_new(AB_PROVIDER *pro, AB_USER *u, int jobVersion, const uint8_t *ptrVopId, unsigned int lenVopId)
{
  AH_JOB *j;
  GWEN_DB_NODE *dbArgs;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Looking for VPA job in version %d", jobVersion);
  j=AH_Job_new("JobVpa", pro, u, 0, jobVersion);
  if (!j) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "VPA job in version %d not found", jobVersion);
    return NULL;
  }

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);

  if (ptrVopId && lenVopId) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Setting VOPID");
    GWEN_DB_SetBinValue(dbArgs, GWEN_DB_FLAGS_DEFAULT, "vopId", ptrVopId, lenVopId);
  }
  else {
    DBG_ERROR(NULL, "No VOPID");
  }

  return j;
}




