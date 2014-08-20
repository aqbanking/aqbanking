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


#ifndef AH_JOBGETBALANCE_L_H
#define AH_JOBGETBALANCE_L_H


#include "accountjob_l.h"
#include <aqbanking/accstatus.h>


AH_JOB *AH_Job_GetBalance_new(AB_USER *u, AB_ACCOUNT *account);


#endif /* AH_JOBGETBALANCE_L_H */


