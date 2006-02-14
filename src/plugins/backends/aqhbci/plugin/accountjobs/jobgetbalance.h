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


#ifndef AH_JOBGETBALANCE_H
#define AH_JOBGETBALANCE_H


#include <aqhbci/accountjob.h>
#include <aqbanking/accstatus.h>


#ifdef __cplusplus
extern "C" {
#endif


AH_JOB *AH_Job_GetBalance_new(AB_USER *u, AB_ACCOUNT *account);


#ifdef __cplusplus
}
#endif


#endif /* AH_JOBGETBALANCE_H */


