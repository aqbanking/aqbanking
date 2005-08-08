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

#ifndef GBANKING_JOBLIST_P_H
#define GBANKING_JOBLIST_P_H

#define GBANKING_JOBLIST_ID "GB_JOBLIST"

#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>
#include <aqbanking/job.h>

#include "joblist.h"

#define GB_JOBLISTCOLUMN_JOBID   0
#define GB_JOBLISTCOLUMN_JOBTYPE 1
#define GB_JOBLISTCOLUMN_BANK    2
#define GB_JOBLISTCOLUMN_ACCOUNT 3
#define GB_JOBLISTCOLUMN_STATUS  4
#define GB_JOBLISTCOLUMN_BACKEND 5
#define GB_JOBLISTCOLUMN_APP     6






typedef struct GBANKING_JOBLIST GBANKING_JOBLIST;
struct GBANKING_JOBLIST {
  AB_BANKING *banking;
  GtkWidget *tree;
  GtkListStore *store;
  AB_JOB_LIST2 *jobs;
};

void GBanking_JobList_free(GBANKING_JOBLIST *jl);
void GBanking_JobList_freeData(gpointer data);


void GBanking_JobList__populate(GBANKING_JOBLIST *jl);

static void
GBanking_JobList__selection_changed_cb(GtkTreeSelection *selection,
                                       gpointer data);

AB_JOB *GBanking_JobList__FindJob(AB_JOB_LIST2 *sjl, GWEN_TYPE_UINT32 id);






#endif



