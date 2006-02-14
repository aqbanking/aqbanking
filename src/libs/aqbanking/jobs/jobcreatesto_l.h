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


#ifndef AQBANKING_JOBCREATESTO_L_H
#define AQBANKING_JOBCREATESTO_L_H


#include <aqbanking/jobcreatesto_be.h>


AB_JOB *AB_JobCreateStandingOrder_fromDb(AB_ACCOUNT *a, GWEN_DB_NODE *db);
int AB_JobCreateStandingOrder_toDb(const AB_JOB *j, GWEN_DB_NODE *db);


#endif

