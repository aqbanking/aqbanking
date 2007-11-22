/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: jobgetbalance_p.h 1283 2007-07-25 14:28:36Z martin $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOB_FOREIGNXFERWH_L_H
#define AH_JOB_FOREIGNXFERWH_L_H


#include "accountjob_l.h"


AH_JOB *AH_Job_ForeignTransferWH_new(AB_USER *u, AB_ACCOUNT *account);

int AH_Job_ForeignTransferWH_SetDtazv(AH_JOB *j,
				      const uint8_t *dataPtr,
				      uint32_t dataLen);



#endif /* AH_JOB_FOREIGNXFERWH_L_H */


