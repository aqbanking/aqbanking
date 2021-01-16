/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_JOBQUEUE_TOMSG_H
#define AH_JOBQUEUE_TOMSG_H

#include "aqhbci/joblayer/jobqueue_l.h"


AH_MSG *AH_JobQueue_ToMessage(AH_JOBQUEUE *jq, AH_DIALOG *dlg);
AH_MSG *AH_JobQueue_ToMessageWithTan(AH_JOBQUEUE *jq, AH_DIALOG *dlg, const char *sTan);




#endif

