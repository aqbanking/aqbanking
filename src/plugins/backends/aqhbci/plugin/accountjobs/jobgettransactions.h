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


#ifndef AH_JOBGETTRANSACTIONS_H
#define AH_JOBGETTRANSACTIONS_H


#include <aqhbci/accountjob.h>

#include <gwenhywfar/gwentime.h>

#define AQHBCI_JOB_GETTRANSACTIONS_WCB "AQHBCI_JOB_GETTRANSACTIONS_WCB"


#ifdef __cplusplus
extern "C" {
#endif


AH_JOB *AH_Job_GetTransactions_new(AB_USER *u,
                                   AB_ACCOUNT *account);


#ifdef __cplusplus
}
#endif


#endif /* AH_JOBGETTRANSACTIONS_H */


