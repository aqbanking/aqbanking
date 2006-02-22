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


#ifndef AH_JOBMULTITRANSFER_L_H
#define AH_JOBMULTITRANSFER_L_H


#include "accountjob_l.h"

#include <gwenhywfar/gwentime.h>


AH_JOB *AH_Job_MultiTransfer_new(AB_USER *u,
                                 AB_ACCOUNT *account);

AH_JOB *AH_Job_MultiDebitNote_new(AB_USER *cu,
                                  AB_ACCOUNT *account);


int AH_Job_MultiTransferBase_GetTransferCount(AH_JOB *j);

int AH_Job_MultiTransferBase_GetMaxTransfers(AH_JOB *j);

int AH_Job_MultiTransferBase_GetTransferCount(AH_JOB *j);


#endif /* AH_JOBMULTITRANSFER_L_H */


