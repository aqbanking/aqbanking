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



#ifndef GB_PROFILEVIEW_P_H
#define GB_PROFILEVIEW_P_H

#define GB_PROFILEVIEW_ID "GB_PROFILEVIEW"

#include "gbprofileview.h"


typedef struct GB_PROFILEVIEW GB_PROFILEVIEW;
struct GB_PROFILEVIEW {
  AB_BANKING *banking;
  GtkWidget *profileView;
  GtkWidget *profileListView;
  GtkWidget *profileList;
  GtkWidget *newButton;
  GtkWidget *editButton;
  GtkWidget *copyButton;
  GtkWidget *deleteButton;
};

static void GB_ProfileView_freeData(gpointer data);
static void GB_ProfileView_slotButtonClicked(GtkButton *button,
                                             gpointer user_data);



#endif /* GB_PROFILEVIEW_P_H */



