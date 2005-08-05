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


#include <aqhbci/jobmultitransfer.h>

#ifdef __cplusplus
extern "C" {
#endif


int AH_Job_MultiTransferBase_GetTransferCount(AH_JOB *j);


#ifdef __cplusplus
}
#endif


#endif /* AH_JOBMULTITRANSFER_L_H */


