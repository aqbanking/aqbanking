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

#ifndef GB_PROFILELIST_H
#define GB_PROFILELIST_H

#include <gtk/gtk.h>

#include <g2banking/gbanking.h>
#include <aqbanking/banking.h>
#include <aqbanking/job.h>

#include <gwenhywfar/types.h>


#define GB_PROFILELIST_COL_NAME    0
#define GB_PROFILELIST_COL_VERSION 1
#define GB_PROFILELIST_COL_SHORT   2



GBANKING_API 
GtkWidget *GB_ProfileList_new(GtkWidget *parent,
                              GtkWidget *tree);

GBANKING_API 
GWEN_DB_NODE *GB_ProfileList_GetSelectedProfiles(GtkWidget *w);

GBANKING_API 
void GB_ProfileList_Update(GtkWidget *w);


GBANKING_API 
void GB_ProfileList_Clear(GtkWidget *w, GWEN_DB_NODE *db);


GBANKING_API 
void GB_ProfileList_AddProfile(GtkWidget *w, GWEN_DB_NODE *db);

GBANKING_API 
void GB_ProfileList_AddProfiles(GtkWidget *w, GWEN_DB_NODE *db);


#endif



