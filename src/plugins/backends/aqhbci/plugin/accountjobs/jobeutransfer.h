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


#ifndef AH_JOBEUTRANSFER_H
#define AH_JOBEUTRANSFER_H


#include <aqhbci/accountjob.h>

#include <gwenhywfar/gwentime.h>


#ifdef __cplusplus
extern "C" {
#endif


AH_JOB *AH_Job_EuTransfer_new(AB_USER *cu,
                              AB_ACCOUNT *account);


#ifdef __cplusplus
}
#endif


#endif /* AH_JOBEUTRANSFER_H */


