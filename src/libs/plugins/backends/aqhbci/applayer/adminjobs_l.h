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
#include "aqhbci/banking/account.h"

#include <gwenhywfar/ct.h>

#ifdef __cplusplus
extern "C" {
#endif



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




#ifdef __cplusplus
}
#endif


#endif /* AH_ADMINJOBS_L_H */

