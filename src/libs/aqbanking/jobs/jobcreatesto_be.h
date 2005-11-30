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


#ifndef AQBANKING_JOBCREATESTO_BE_H
#define AQBANKING_JOBCREATESTO_BE_H


#include <aqbanking/jobcreatesto.h>


AQBANKING_API void AB_JobCreateStandingOrder_SetFieldLimits(AB_JOB *j,
                                              AB_TRANSACTION_LIMITS *limits);
AB_JOB *AB_JobCreateStandingOrder_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);
int AB_JobCreateStandingOrder_toDb(const AB_JOB *j, GWEN_DB_NODE *db);


#endif

