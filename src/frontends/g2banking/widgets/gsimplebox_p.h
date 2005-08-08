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

#ifndef GBANKING_SIMPLEBOX_P_H
#define GBANKING_SIMPLEBOX_P_H

#define GBANKING_SIMPLEBOX_ID "GB_SIMPLEBOX"


#include <gtk/gtk.h>

#include "gsimplebox.h"



typedef struct GBANKING_SIMPLEBOX GBANKING_SIMPLEBOX;
struct GBANKING_SIMPLEBOX {
  GtkWidget *widget;
  GtkWidget *titleText;
  GtkWidget *msgText;

  GWEN_TYPE_UINT32 id;
  GWEN_TYPE_UINT32 flags;
};


void GBanking_SimpleBox_free(GBANKING_SIMPLEBOX *wd);
void GBanking_SimpleBox_freeData(gpointer data);



#endif



