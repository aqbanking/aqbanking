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


#ifndef AH_JOBGETTRANSACTIONS_L_H
#define AH_JOBGETTRANSACTIONS_L_H


#include "accountjob_l.h"

#include <gwenhywfar/gwentime.h>


AH_JOB *AH_Job_GetTransactions_new(AB_USER *u,
                                   AB_ACCOUNT *account);


#endif /* AH_JOBGETTRANSACTIONS_H */


