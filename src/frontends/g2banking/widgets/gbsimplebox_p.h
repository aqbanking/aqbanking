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

#ifndef GB_SIMPLEBOX_P_H
#define GB_SIMPLEBOX_P_H

#define GB_SIMPLEBOX_ID "GB_SIMPLEBOX"


#include <gtk/gtk.h>

#include "gbsimplebox_l.h"



typedef struct GB_SIMPLEBOX GB_SIMPLEBOX;
struct GB_SIMPLEBOX {
  GtkWidget *widget;
  GtkWidget *titleText;
  GtkWidget *msgText;

  GWEN_TYPE_UINT32 id;
  GWEN_TYPE_UINT32 flags;
};


void GB_SimpleBox_freeData(gpointer data);



#endif



