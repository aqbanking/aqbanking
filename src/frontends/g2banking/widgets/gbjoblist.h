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


/** @defgroup G_AB_G2BANKING_W_JOBLIST Job List
 * @ingroup G_AB_G2BANKING_WIDGETS
 *
 */
/*@{*/

/**
 * Create a new joblist.
 * @param ab GBanking object
 * @param parent parent widget (or 0 if none)
 * @param tree tree widget to use (if NULL then this function will
 * create a new one)
 */
GBANKING_API 
GtkWidget *GB_JobList_new(AB_BANKING *ab,
                          GtkWidget *parent,
                          GtkWidget *tree);

/**
 * Returns a list containing only selected jobs from the given job list.
 * The caller is responsible for calling @ref AB_Job_List2_free.
 * The jobs in the returned list are only valid as long as the list widget
 * exists without changes.
 * @param w widget created by @ref GB_JobList_new
 */
GBANKING_API 
AB_JOB_LIST2 *GB_JobList_GetSelectedJobs(GtkWidget *w);

/**
 * This function redraws the jobs in the list.
 * @param w widget created by @ref GB_JobList_new
 */
GBANKING_API 
void GB_JobList_Update(GtkWidget *w);


/*@}*/ /* defgroup */


#endif



