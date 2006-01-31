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


GBANKING_API 
GtkWidget *GBanking_JobView_new(AB_BANKING *ab, int fullWindow);



#endif /* GBANKING_JOBVIEW_H */

