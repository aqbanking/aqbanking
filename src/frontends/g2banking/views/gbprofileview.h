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



#ifndef GB_PROFILEVIEW_H
#define GB_PROFILEVIEW_H

#include <gtk/gtk.h>
#include <g2banking/gbanking.h>
#include <aqbanking/banking.h>


GBANKING_API 
GtkWidget *GB_ProfileView_new(AB_BANKING *ab,
                              GWEN_DB_NODE *dbProfiles,
                              GtkWidget *parent);



#endif /* GB_PROFILEVIEW_H */

