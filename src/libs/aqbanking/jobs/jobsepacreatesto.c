/***************************************************************************
 begin       : Wed Jan 15 2014
 copyright   : (C) 2014 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsepacreatesto.h"
#include "job_l.h"


AB_JOB *AB_JobSepaCreateStandingOrder_new(AB_ACCOUNT *a){
  return AB_Job_new(AB_Job_TypeSepaCreateStandingOrder, a);
}



