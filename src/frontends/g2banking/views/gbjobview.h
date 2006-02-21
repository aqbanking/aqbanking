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



#ifndef GBANKING_JOBVIEW_H
#define GBANKING_JOBVIEW_H

#include <gtk/gtk.h>
#include <g2banking/gbanking.h>
#include <aqbanking/banking.h>

/** @defgroup G_AB_G2BANKING_V_JOBS Job View
 * @ingroup G_AB_G2BANKING_VIEWS
 *
 * A job view is a widget which contains a job list and some buttons for
 * queue execution and job list manipulation.
 */
/*@{*/

/**
 * Creates a job view. You can create a full window or embed it into
 * an existing window.
 * @param ab GBanking object
 * @param fullWindow if 0 then the view is to become a part of an eyisting
 *  window, otherwise it will be created as a new toplevel window
 */
GBANKING_API 
GtkWidget *GBanking_JobView_new(AB_BANKING *ab, int fullWindow);

/*@}*/


#endif /* GBANKING_JOBVIEW_H */

