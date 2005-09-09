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


#ifndef AQBANKING_JOBINTERNALTRANSFER_BE_H
#define AQBANKING_JOBINTERNALTRANSFER_BE_H


#include <aqbanking/jobinternaltransfer.h>


void AB_JobInternalTransfer_SetFieldLimits(AB_JOB *j,
                                           AB_TRANSACTION_LIMITS *limits);
AB_JOB *AB_JobInternalTransfer_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);
int AB_JobInternalTransfer_toDb(const AB_JOB *j, GWEN_DB_NODE *db);


#endif

