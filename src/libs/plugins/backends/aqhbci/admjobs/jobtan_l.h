/***************************************************************************
    begin       : Thu Jan 31 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBTAN_L_H
#define AH_JOBTAN_L_H


#include "aqhbci_l.h"
#include "job_l.h"



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




#endif

