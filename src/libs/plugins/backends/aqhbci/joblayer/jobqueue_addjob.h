/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBQUEUE_ADDJOB_H
#define AH_JOBQUEUE_ADDJOB_H

#include "aqhbci/joblayer/jobqueue_l.h"


AH_JOBQUEUE_ADDRESULT AH_JobQueue_AddJob(AH_JOBQUEUE *jq, AH_JOB *j);



#endif

