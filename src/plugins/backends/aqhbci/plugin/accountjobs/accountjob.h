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


#ifndef AH_ACCOUNTJOBS_H
#define AH_ACCOUNTJOBS_H


#include <aqhbci/job.h>
#include <aqbanking/account.h>


#ifdef __cplusplus
extern "C" {
#endif

AB_ACCOUNT *AH_AccountJob_GetAccount(const AH_JOB *j);



#ifdef __cplusplus
}
#endif



#endif /* AH_ACCOUNTJOBS_H */


