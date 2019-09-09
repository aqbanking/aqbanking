/***************************************************************************
    begin       : Mon Sep 09 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "jobgetbankinfo_p.h"

#include "aqhbci/aqhbci_l.h"



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int _process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static void GWENHYWFAR_CB _freeData(void *bp, void *p);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */




GWEN_INHERIT(AH_JOB, AH_JOB_GETBANKINFO)




AH_JOB *AH_Job_GetBankInfo_new(AB_PROVIDER *pro, AB_USER *u, int withHkTan)
{
  AH_JOB *j;
  GWEN_DB_NODE *args;
  AH_JOB_GETBANKINFO *jd;

  assert(u);
  if (withHkTan)
    j=AH_Job_new("JobGetBankInfoWithTan", pro, u, 0, 0);
  else
  j=AH_Job_new("JobGetBankInfo", pro, u, 0, 0);
  if (!j) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "JobGetBankInfo not supported, should not happen");
    return 0;
  }

  GWEN_NEW_OBJECT(AH_JOB_GETBANKINFO, jd);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_GETBANKINFO, j, jd, _freeData)
  AH_Job_SetProcessFn(j, _process);

  /* set arguments */
  args=AH_Job_GetArguments(j);
  assert(args);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "open/prepare/bpdversion", 0);
  GWEN_DB_SetIntValue(args, GWEN_DB_FLAGS_OVERWRITE_VARS, "open/prepare/updversion", 0);

  DBG_INFO(AQHBCI_LOGDOMAIN, "JobGetBankInfo created");
  return j;
}



void GWENHYWFAR_CB _freeData(void *bp, void *p)
{
  AH_JOB_GETBANKINFO *jd;

  jd=(AH_JOB_GETBANKINFO *)p;
  GWEN_FREE_OBJECT(jd);
}



int _process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx)
{
  AH_JOB_GETBANKINFO *jd;
  GWEN_DB_NODE *dbResponses;
  AB_USER *u;
  AB_BANKING *ab;
  GWEN_DB_NODE *dbCurr;
  int tanMaxVersion=0;

  assert(j);
  jd=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_GETBANKINFO, j);
  assert(jd);

  if (jd->scanned)
    return 0;

  jd->scanned=1;

  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  u=AH_Job_GetUser(j);
  assert(u);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  dbCurr=GWEN_DB_FindFirstGroup(dbResponses, "HITANS");
  while (dbCurr) {
    GWEN_DB_NODE *dbTanData;

    dbTanData=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data/HITANS");
    if (dbTanData) {
      int tanSegVersion;

      tanSegVersion=GWEN_DB_GetIntValue(dbTanData, "head/version", 0, 0);
      if (tanSegVersion>tanMaxVersion)
        tanMaxVersion=tanSegVersion;

      DBG_INFO(AQHBCI_LOGDOMAIN, "Found an TAN segment definition (version %d)", tanSegVersion);
    }
    dbCurr=GWEN_DB_FindNextGroup(dbCurr, "HITANS");
  }

  if (tanMaxVersion<1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No HITANS segments found in server response.");
  }

  return 0;
}




