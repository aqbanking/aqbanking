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


#ifndef AQBANKING_JOB_L_H
#define AQBANKING_JOB_L_H


#include <aqbanking/job.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/db.h>

GWEN_LIST_FUNCTION_DEFS(AB_JOB, AB_Job);



AB_JOB *AB_Job_new(AB_JOB_TYPE jt, AB_ACCOUNT *a);

GWEN_DB_NODE *AB_Job_GetData(const AB_JOB *j);


int AB_Job_toDb(const AB_JOB *j, GWEN_DB_NODE *db);
AB_JOB *AB_Job_fromDb(AB_BANKING *ab, GWEN_DB_NODE *db);



#endif /* AQBANKING_JOB_L_H */
