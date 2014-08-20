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


#ifndef AH_JOBSINGLETRANSFER_L_H
#define AH_JOBSINGLETRANSFER_L_H


#include "accountjob_l.h"

#include <gwenhywfar/gwentime.h>


AH_JOB *AH_Job_SingleTransfer_new(AB_USER *u,
                                  AB_ACCOUNT *account);

AH_JOB *AH_Job_SingleDebitNote_new(AB_USER *u,
                                   AB_ACCOUNT *account);

AH_JOB *AH_Job_CreateStandingOrder_new(AB_USER *u,
                                       AB_ACCOUNT *account);

AH_JOB *AH_Job_ModifyStandingOrder_new(AB_USER *u,
                                       AB_ACCOUNT *account);

AH_JOB *AH_Job_DeleteStandingOrder_new(AB_USER *u,
                                       AB_ACCOUNT *account);

AH_JOB *AH_Job_CreateDatedTransfer_new(AB_USER *u,
                                       AB_ACCOUNT *account);

AH_JOB *AH_Job_ModifyDatedTransfer_new(AB_USER *u,
                                       AB_ACCOUNT *account);

AH_JOB *AH_Job_DeleteDatedTransfer_new(AB_USER *u,
                                       AB_ACCOUNT *account);

AH_JOB *AH_Job_InternalTransfer_new(AB_USER *u,
                                    AB_ACCOUNT *account);


const char *AH_Job_SingleTransfer_GetFiid(AH_JOB *j);

const char *AH_Job_SingleTransfer_GetOldFiid(AH_JOB *j);


#endif /* AH_JOBSINGLETRANSFER_L_H */


