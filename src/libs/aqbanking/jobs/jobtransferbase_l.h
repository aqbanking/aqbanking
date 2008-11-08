/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOBTRANSFERBASE_L_H
#define AQBANKING_JOBTRANSFERBASE_L_H


#include <aqbanking/job.h>


AB_JOB *AB_JobTransferBase_new(AB_JOB_TYPE jt, AB_ACCOUNT *a);

int AB_JobTransferBase_SetTransaction(AB_JOB *j, const AB_TRANSACTION *t);
AB_TRANSACTION *AB_JobTransferBase_GetTransaction(const AB_JOB *j);

const AB_TRANSACTION_LIMITS *AB_JobTransferBase_GetFieldLimits(AB_JOB *j);

void AB_JobTransferBase_SetFieldLimits(AB_JOB *j,
                                       AB_TRANSACTION_LIMITS *limits);


#endif

