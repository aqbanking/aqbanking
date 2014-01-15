/***************************************************************************
    begin       : Sat Dec 10 2011
    copyright   : (C) 2011 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_HHD_L_H
#define AQHBCI_HHD_L_H

#include <gwenhywfar/buffer.h>


int AH_HHD14_Translate(const char *code, GWEN_BUFFER *cbuf);



int AH_HHD14_AddChallengeParams_04(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteBankCode,
                                   const char *sRemoteAccountNumber);

int AH_HHD14_AddChallengeParams_05(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteAccountNumber);

int AH_HHD14_AddChallengeParams_09(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteIban);

int AH_HHD14_AddChallengeParams_12(AH_JOB *j, int numTransfers, const AB_VALUE *vSumOfAmount,
                                   const char *sLocalAccount, const AB_VALUE *vSumOfRemoteAccounts);

int AH_HHD14_AddChallengeParams_13(AH_JOB *j, int numTransfers, const AB_VALUE *vSumOfAmount, const char *sLocalIban);

int AH_HHD14_AddChallengeParams_17(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteIban);

int AH_HHD14_AddChallengeParams_19(AH_JOB *j, int numTransfers, const AB_VALUE *vSumOfAmount,
                                   const char *sLocalAccountNumber, const AB_VALUE *vSumOfRemoteAccounts);

int AH_HHD14_AddChallengeParams_23(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteIban,
                                   const GWEN_TIME *ti);

int AH_HHD14_AddChallengeParams_29(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteIban,
                                   const GWEN_TIME *ti);

int AH_HHD14_AddChallengeParams_32(AH_JOB *j,
                                   int transferCount,
                                   const AB_VALUE *vAmount,
                                   const char *sLocalIban,
                                   const GWEN_TIME *ti);

int AH_HHD14_AddChallengeParams_35(AH_JOB *j,
                                   const AB_VALUE *vAmount,
                                   const char *sRemoteIban);




#endif
