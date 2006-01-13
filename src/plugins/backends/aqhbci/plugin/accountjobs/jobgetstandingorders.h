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


#ifndef AH_JOBGETSTANDINGORDERS_H
#define AH_JOBGETSTANDINGORDERS_H


#include <aqhbci/accountjob.h>


#ifdef __cplusplus
extern "C" {
#endif


AH_JOB *AH_Job_GetStandingOrders_new(AB_USER *u, AB_ACCOUNT *account);


#ifdef __cplusplus
}
#endif


#endif /* AH_JOBGETSTANDINGORDERS_H */


