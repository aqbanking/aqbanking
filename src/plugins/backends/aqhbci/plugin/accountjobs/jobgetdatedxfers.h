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


#ifndef AH_JOBGETDATEDTRANSFERS_H
#define AH_JOBGETDATEDTRANSFERS_H


#include <aqhbci/accountjob.h>


#ifdef __cplusplus
extern "C" {
#endif


AH_JOB *AH_Job_GetDatedTransfers_new(AH_CUSTOMER *cu, AH_ACCOUNT *account);


#ifdef __cplusplus
}
#endif


#endif /* AH_JOBGETDATEDTRANSFERS_H */


