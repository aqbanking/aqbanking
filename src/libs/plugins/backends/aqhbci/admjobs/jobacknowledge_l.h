/***************************************************************************
    begin       : Mon Mar 16 2020
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBACKNOWLEDGE_L_H
#define AH_JOBACKNOWLEDGE_L_H


#include "aqhbci/aqhbci_l.h"
#include "aqhbci/joblayer/job_l.h"


AH_JOB *AH_Job_Acknowledge_new(AB_PROVIDER *pro, AB_USER *u,
                               const uint8_t *ptrAckCode,
                               uint32_t lenAckCode);




#endif

