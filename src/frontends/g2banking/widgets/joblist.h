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

#ifndef GBANKING_JOBLIST_H
#define GBANKING_JOBLIST_H

#include <gtk/gtk.h>

#include <g2banking/gbanking.h>
#include <aqbanking/banking.h>
#include <aqbanking/job.h>

#include <gwenhywfar/types.h>



GBANKING_API 
GtkWidget *GBanking_JobList_new(AB_BANKING *ab,
                                GtkWidget *parent,
                                GtkWidget *tree);

GBANKING_API 
AB_JOB_LIST2 *GBanking_JobList_GetSelectedJobs(GtkWidget *w);

GBANKING_API 
void GBanking_JobList_Update(GtkWidget *w);




#endif



