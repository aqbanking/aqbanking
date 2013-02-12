/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2013 by Martin Preuss
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
  char *peerId;
  GWEN_CRYPT_TOKEN_KEYINFO *signKeyInfo;
  GWEN_CRYPT_TOKEN_KEYINFO *cryptKeyInfo;
  GWEN_CRYPT_TOKEN_KEYINFO *authKeyInfo;
};

static void GWENHYWFAR_CB AH_Job_GetKeys_FreeData(void *bp, void *p);
static int AH_Job_GetKeys_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);




/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_SendKeys
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


/* no data needed */

static int AH_Job_SendKeys_PrepareKey(AH_JOB *j,
                                      GWEN_DB_NODE *dbKey,
				      const GWEN_CRYPT_TOKEN_KEYINFO *ki,
                                      int kn);



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetSysId
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

typedef struct AH_JOB_GETSYSID AH_JOB_GETSYSID;

struct AH_JOB_GETSYSID {
  char *sysId;
};

static void GWENHYWFAR_CB AH_Job_GetSysId_FreeData(void *bp, void *p);
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
static void GWENHYWFAR_CB AH_Job_UpdateBank_FreeData(void *bp, void *p);

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
static void GWENHYWFAR_CB AH_Job_TestVersion_FreeData(void *bp, void *p);
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
static void GWENHYWFAR_CB AH_Job_GetStatus_FreeData(void *bp, void *p);
static int AH_Job_GetStatus_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_GetStatus_Exchange(AH_JOB *j, AB_JOB *bj,
				     AH_JOB_EXCHANGE_MODE m,
				     AB_IMEXPORTER_CONTEXT *ctx);



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_Tan
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */
typedef struct AH_JOB_TAN AH_JOB_TAN;
struct AH_JOB_TAN {
  char *challenge;
  char *challengeHhd;
  char *reference;
  int tanMethod;
  char *tanMediumId;
};
static void GWENHYWFAR_CB AH_Job_Tan_FreeData(void *bp, void *p);
static int AH_Job_Tan_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_Tan_Exchange(AH_JOB *j, AB_JOB *bj,
			       AH_JOB_EXCHANGE_MODE m,
			       AB_IMEXPORTER_CONTEXT *ctx);



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetItanModes
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */
#define AH_JOB_GETITANMODES_MAXMODES 20
typedef struct AH_JOB_GETITANMODES AH_JOB_GETITANMODES;
struct AH_JOB_GETITANMODES {
  int modeList[AH_JOB_GETITANMODES_MAXMODES+1];
  int modeCount;
};
static void GWENHYWFAR_CB AH_Job_GetItanModes_FreeData(void *bp, void *p);
static int AH_Job_GetItanModes_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);




/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetAccountSepaInfo
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

typedef struct AH_JOB_GETACCSEPAINFO AH_JOB_GETACCSEPAINFO;
struct AH_JOB_GETACCSEPAINFO {
  AB_ACCOUNT *account;
  int scanned;
};
static void GWENHYWFAR_CB AH_Job_GetAccountSepaInfo_FreeData(void *bp, void *p);

static int AH_Job_GetAccountSepaInfo_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);



#endif /* AH_ADMINJOBS_P_H */



