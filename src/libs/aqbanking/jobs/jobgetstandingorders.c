/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobgetstandingorders.h"
#include "job_l.h"



GWEN_INHERIT(AB_JOB, AB_JOB_GETSTANDINGORDERS)



AB_JOB *AB_JobGetStandingOrders_new(AB_ACCOUNT *a) {
  return AB_Job_new(AB_Job_TypeGetStandingOrders, a);
}



