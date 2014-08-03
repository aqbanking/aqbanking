/***************************************************************************
 begin       : Wed Jan 15 2014
 copyright   : (C) 2014 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBSEPASTANDINGORDERGET_P_H
#define AH_JOBSEPASTANDINGORDERGET_P_H


#include "jobsepastandingorderget_l.h"


static int AH_Job_SepaStandingOrderGet_Prepare(AH_JOB *j);
static int AH_Job_SepaStandingOrdersGet_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx);
static int AH_Job_SepaStandingOrdersGet_Exchange(AH_JOB *j, AB_JOB *bj,
                                                 AH_JOB_EXCHANGE_MODE m,
                                                 AB_IMEXPORTER_CONTEXT *ctx);





#endif /* AH_JOBSEPASTANDINGORDERGET_L_H */


