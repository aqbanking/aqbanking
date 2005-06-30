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


#ifndef AQBANKING_JOBMODIFYSTO_BE_H
#define AQBANKING_JOBMODIFYSTO_BE_H


#include <aqbanking/jobmodifysto.h>


void AB_JobModifyStandingOrder_SetFieldLimits(AB_JOB *j,
                                              AB_TRANSACTION_LIMITS *limits);
AB_JOB *AB_JobModifyStandingOrder_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);
int AB_JobModifyStandingOrder_toDb(const AB_JOB *j, GWEN_DB_NODE *db);


#endif

