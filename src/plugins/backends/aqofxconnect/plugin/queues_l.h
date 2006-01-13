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


typedef struct AO_QUEUE AO_QUEUE;
typedef struct AO_USERQUEUE AO_USERQUEUE;

GWEN_LIST_FUNCTION_DEFS(AO_USERQUEUE, AO_UserQueue)


AO_USERQUEUE *AO_UserQueue_new(AB_USER *u);
void AO_UserQueue_free(AO_USERQUEUE *uq);
AB_USER *AO_UserQueue_GetUser(const AO_USERQUEUE *uq);
void AO_UserQueue_AddJob(AO_USERQUEUE *uq, AB_JOB *bj);
AB_JOB_LIST2 *AO_UserQueue_GetJobs(const AO_USERQUEUE *uq);

AO_QUEUE *AO_Queue_new();
void AO_Queue_free(AO_QUEUE *q);

AO_USERQUEUE *AO_Queue_FindUserQueue(AO_QUEUE *q, const AB_USER *u);
AO_USERQUEUE *AO_Queue_GetUserQueue(AO_QUEUE *q, AB_USER *u);
AO_USERQUEUE *AO_Queue_FirstUserQueue(AO_QUEUE *q);
void AO_Queue_AddJob(AO_QUEUE *q, AB_USER *u, AB_JOB *bj);
void AO_Queue_Clear(AO_QUEUE *q);



#endif
