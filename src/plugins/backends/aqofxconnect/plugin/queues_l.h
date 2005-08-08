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

#ifndef AO_QUEUES_H
#define AO_QUEUES_H

#include <gwenhywfar/misc.h>
#include <aqofxconnect/user.h>
#include <aqofxconnect/bank.h>


typedef struct AO_BANKQUEUE AO_BANKQUEUE;
typedef struct AO_USERQUEUE AO_USERQUEUE;

GWEN_LIST_FUNCTION_DEFS(AO_BANKQUEUE, AO_BankQueue)
GWEN_LIST_FUNCTION_DEFS(AO_USERQUEUE, AO_UserQueue)


AO_USERQUEUE *AO_UserQueue_new(AO_USER *u);
void AO_UserQueue_free(AO_USERQUEUE *uq);
AO_USER *AO_UserQueue_GetUser(const AO_USERQUEUE *uq);
void AO_UserQueue_AddJob(AO_USERQUEUE *uq, AB_JOB *bj);
AB_JOB_LIST2 *AO_UserQueue_GetJobs(const AO_USERQUEUE *uq);

AO_BANKQUEUE *AO_BankQueue_new(AO_BANK *b);
void AO_BankQueue_free(AO_BANKQUEUE *bq);
AO_BANK *AO_BankQueue_GetBank(const AO_BANKQUEUE *bq);

AO_USERQUEUE_LIST *AO_BankQueue_GetUserQueues(const AO_BANKQUEUE *bq);
AO_USERQUEUE *AO_BankQueue_FindUserQueue(const AO_BANKQUEUE *bq,
                                         const char *uid);
void AO_BankQueue_AddUserQueue(AO_BANKQUEUE *bq, AO_USERQUEUE *uq);
void AO_BankQueue_AddJob(AO_BANKQUEUE *bq, const char *uid, AB_JOB *bj);


#endif
