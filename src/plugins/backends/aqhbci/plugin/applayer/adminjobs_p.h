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


#ifndef AH_ADMINJOBS_P_H
#define AH_ADMINJOBS_P_H

#include "adminjobs_l.h"


/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetKeys
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

typedef struct AH_JOB_GETKEYS AH_JOB_GETKEYS;

struct AH_JOB_GETKEYS {
  GWEN_CRYPTKEY *signKey;
  GWEN_CRYPTKEY *cryptKey;
};

static void AH_Job_GetKeys_FreeData(void *bp, void *p);
static int AH_Job_GetKeys_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_GetKeys_Commit(AH_JOB *j);




/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_SendKeys
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


/* no data needed */

static int AH_Job_SendKeys_PrepareKey(AH_JOB *j,
                                      GWEN_DB_NODE *dbKey,
                                      const GWEN_CRYPTKEY *key);



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetSysId
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

typedef struct AH_JOB_GETSYSID AH_JOB_GETSYSID;

struct AH_JOB_GETSYSID {
  char *sysId;
};

static void AH_Job_GetSysId_FreeData(void *bp, void *p);
static int AH_Job_GetSysId_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_GetSysId_NextMsg(AH_JOB *j);
static int AH_Job_GetSysId_ExtractSysId(AH_JOB *j);




/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_UpdateBank
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

typedef struct AH_JOB_UPDATEBANK AH_JOB_UPDATEBANK;
struct AH_JOB_UPDATEBANK {
  AB_ACCOUNT_LIST2 *accountList;
  int scanned;
};
static void AH_Job_UpdateBank_FreeData(void *bp, void *p);

static int AH_Job_UpdateBank_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);




/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_TestVersion
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */
typedef struct AH_JOB_TESTVERSION AH_JOB_TESTVERSION;
struct AH_JOB_TESTVERSION {
  AH_JOB_TESTVERSION_RESULT versionSupported;
};
static void AH_Job_TestVersion_FreeData(void *bp, void *p);
static int AH_Job_TestVersion_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);




/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetStatus
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */
typedef struct AH_JOB_GETSTATUS AH_JOB_GETSTATUS;
struct AH_JOB_GETSTATUS {
  AH_RESULT_LIST *results;
  GWEN_TIME *fromDate;
  GWEN_TIME *toDate;
};
static void AH_Job_GetStatus_FreeData(void *bp, void *p);
static int AH_Job_GetStatus_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_GetStatus_Exchange(AH_JOB *j, AB_JOB *bj,
                                     AH_JOB_EXCHANGE_MODE m);



#endif /* AH_ADMINJOBS_P_H */



