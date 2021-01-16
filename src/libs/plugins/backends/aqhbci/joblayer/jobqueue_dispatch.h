/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBQUEUE_DISPATCH_H
#define AH_JOBQUEUE_DISPATCH_H

#include "aqhbci/joblayer/jobqueue_l.h"


int AH_JobQueue_DispatchMessage(AH_JOBQUEUE *jq, AH_MSG *msg, GWEN_DB_NODE *db);



#endif

