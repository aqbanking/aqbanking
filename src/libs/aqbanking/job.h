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


#ifndef AQBANKING_JOB_H
#define AQBANKING_JOB_H

#include <gwenhywfar/misc2.h>

typedef struct AB_JOB AB_JOB;
GWEN_LIST2_FUNCTION_DEFS(AB_JOB, AB_Job);
void AB_Job_List2_FreeAll(AB_JOB_LIST2 *jl);

#include <aqbanking/account.h>


typedef enum {
  AB_Job_StatusNew=0,
  AB_Job_StatusEnqueued,
  AB_Job_StatusSent,
  AB_Job_StatusAnswered,
  AB_Job_StatusError,
  AB_Job_StatusUnknown=999
} AB_JOB_STATUS;


typedef enum {
  AB_Job_TypeUnknown=0,
  AB_Job_TypeGetBalance,
  AB_Job_TypeGetTransactions,
  AB_Job_TypeTransfer,
  AB_Job_TypeDebitNote
} AB_JOB_TYPE;



void AB_Job_free(AB_JOB *j);
void AB_Job_Attach(AB_JOB *j);


int AB_Job_CheckAvailability(AB_JOB *j);

/**
 * This id can be used by a AB_PROVIDER to map AB_Jobs to whatever the
 * provider uses. This id is not used by AB_Banking itself.
 */
GWEN_TYPE_UINT32 AB_Job_GetIdForProvider(const AB_JOB *j);
void AB_Job_SetIdForProvider(AB_JOB *j, GWEN_TYPE_UINT32 i);

AB_JOB_STATUS AB_Job_GetStatus(const AB_JOB *j);
void  AB_Job_SetStatus(AB_JOB *j, AB_JOB_STATUS st);
AB_JOB_TYPE AB_Job_GetType(const AB_JOB *j);


const char *AB_Job_Status2Char(AB_JOB_STATUS i);
AB_JOB_STATUS AB_Job_Char2Status(const char *s);

const char *AB_Job_Type2Char(AB_JOB_TYPE i);
AB_JOB_TYPE AB_Job_Char2Type(const char *s);


AB_ACCOUNT *AB_Job_GetAccount(const AB_JOB *j);

const char *AB_Job_GetResultText(const AB_JOB *j);
void AB_Job_SetResultText(AB_JOB *j, const char *s);



#endif /* AQBANKING_JOB_H */
