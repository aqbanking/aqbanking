/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_ADMINJOBS_L_H
#define AH_ADMINJOBS_L_H


#include "joblayer/job_l.h"
#include <aqhbci/account.h>
#include <gwenhywfar/ct.h>

#ifdef __cplusplus
extern "C" {
#endif



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_SendKeys
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


AH_JOB *AH_Job_SendKeys_new(AB_PROVIDER *pro, AB_USER *u,
			    const GWEN_CRYPT_TOKEN_KEYINFO *cryptKeyInfo,
			    const GWEN_CRYPT_TOKEN_KEYINFO *signKeyInfo,
			    const GWEN_CRYPT_TOKEN_KEYINFO *authKeyInfo);






/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetSysId
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

AH_JOB *AH_Job_GetSysId_new(AB_PROVIDER *pro, AB_USER *u);

const char *AH_Job_GetSysId_GetSysId(AH_JOB *j);






/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_UpdateBank
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

AH_JOB *AH_Job_UpdateBank_new(AB_PROVIDER *pro, AB_USER *u);



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_TestVersion
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */


typedef enum {
  AH_JobTestVersion_ResultUnknown=0,
  AH_JobTestVersion_ResultNotSupported,
  AH_JobTestVersion_ResultMaybeSupported,
  AH_JobTestVersion_ResultSupported
} AH_JOB_TESTVERSION_RESULT;


AH_JOB *AH_Job_TestVersion_new(AB_PROVIDER *pro, AB_USER *u, int anon);
AH_JOB_TESTVERSION_RESULT AH_Job_TestVersion_GetResult(const AH_JOB *j);



/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetStatus
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

AH_JOB *AH_Job_GetStatus_new(AB_PROVIDER *pro, AB_USER *u,
                             const GWEN_TIME *fromDate,
                             const GWEN_TIME *toDate);
AH_RESULT_LIST *AH_Job_GetStatus_GetResults(const AH_JOB *j);




/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_Tan
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */
/**
 * This is an internal job. It is only used to present a TAN using a
 * two-step mechanism (iTAN).
 */
AH_JOB *AH_Job_Tan_new(AB_PROVIDER *pro, AB_USER *u, int process, int jobVersion);
void AH_Job_Tan_SetHash(AH_JOB *j,
                        const unsigned char *p,
                        unsigned int len);
void AH_Job_Tan_SetReference(AH_JOB *j, const char *p);
void AH_Job_Tan_SetTanList(AH_JOB *j, const char *s);
void AH_Job_Tan_SetTanInfo(AH_JOB *j, const char *p);

const char *AH_Job_Tan_GetChallenge(const AH_JOB *j);
const char *AH_Job_Tan_GetHhdChallenge(const AH_JOB *j);
const char *AH_Job_Tan_GetReference(const AH_JOB *j);

int AH_Job_Tan_GetTanMethod(const AH_JOB *j);
void AH_Job_Tan_SetTanMethod(AH_JOB *j, int i);

void AH_Job_Tan_SetTanMediumId(AH_JOB *j, const char *s);

void AH_Job_Tan_SetLocalAccountInfo(AH_JOB *j,
                                    const char *bankCode,
                                    const char *accountId,
                                    const char *accountSubId);
void AH_Job_Tan_SetSmsAccountInfo(AH_JOB *j,
                                  const char *bankCode,
                                  const char *accountId,
                                  const char *accountSubId);

void AH_Job_Tan_SetSegCode(AH_JOB *j, const char *p);

int AH_Job_Tan_FinishSetup(AH_JOB *j, AH_JOB *accJob);


/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetItanModes
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

AH_JOB *AH_Job_GetItanModes_new(AB_PROVIDER *pro, AB_USER *u);
const int *AH_Job_GetItanModes_GetModes(const AH_JOB *j);




/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_ChangePin
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

AH_JOB *AH_Job_ChangePin_new(AB_PROVIDER *pro, AB_USER *u, const char *newPin);




/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                             AH_Job_GetAccountSepaInfo
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

AH_JOB *AH_Job_GetAccountSepaInfo_new(AB_PROVIDER *pro, AB_USER *u, AB_ACCOUNT *acc);





#ifdef __cplusplus
}
#endif


#endif /* AH_ADMINJOBS_L_H */

