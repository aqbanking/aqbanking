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

#ifndef GB_JOBLIST_H
#define GB_JOBLIST_H

#include <gtk/gtk.h>

#include <g2banking/gbanking.h>
#include <aqbanking/banking.h>
#include <aqbanking/job.h>

#include <gwenhywfar/types.h>



GBANKING_API 
GtkWidget *GB_JobList_new(AB_BANKING *ab,
                          GtkWidget *parent,
                          GtkWidget *tree);

GBANKING_API 
AB_JOB_LIST2 *GB_JobList_GetSelectedJobs(GtkWidget *w);

GBANKING_API 
void GB_JobList_Update(GtkWidget *w);




#endif



