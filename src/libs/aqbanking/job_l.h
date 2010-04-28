/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_JOB_L_H
#define AQBANKING_JOB_L_H


#include <aqbanking/job.h>
#include <aqbanking/job_be.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/db.h>

GWEN_LIST_FUNCTION_DEFS(AB_JOB, AB_Job)
GWEN_INHERIT_FUNCTION_DEFS(AB_JOB)


AB_JOB *AB_Job_new(AB_JOB_TYPE jt, AB_ACCOUNT *a);

int AB_Job_Update(AB_JOB *j);

void AB_Job_SetUniqueId(AB_JOB *j, uint32_t jid);



#endif /* AQBANKING_JOB_L_H */
