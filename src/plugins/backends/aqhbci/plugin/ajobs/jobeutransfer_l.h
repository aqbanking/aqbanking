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


#ifndef AH_JOBEUTRANSFER_L_H
#define AH_JOBEUTRANSFER_L_H


#include "accountjob_l.h"

#include <gwenhywfar/gwentime.h>


AH_JOB *AH_Job_EuTransfer_new(AB_USER *cu, AB_ACCOUNT *account);


#endif /* AH_JOBEUTRANSFER_L_H */


