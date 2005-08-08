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

#ifndef GBANKING_PROGRESS_P_H
#define GBANKING_PROGRESS_P_H

#include <gtk/gtk.h>

#include "gprogress.h"

#define GBANKING_PROGRESS_ID "GB_PROGRESS"



typedef struct GBANKING_PROGRESS GBANKING_PROGRESS;
struct GBANKING_PROGRESS {
  AB_BANKING *banking;
  GtkWidget *dialog;
  GtkWidget *titleText;
  GtkWidget *logText;
  GtkWidget *progressBar;
  GtkWidget *closeButton;
  GtkWidget *abortButton;
  GWEN_TYPE_UINT32 id;
  int aborted;
  int finished;
  int allowClose;
  GWEN_TYPE_UINT32 totalProgress;
  GWEN_TYPE_UINT32 currentProgress;
  GWEN_BUFFER *logBuffer;
};

void GBanking_Progress_free(GBANKING_PROGRESS *wd);
void GBanking_Progress_freeData(gpointer data);


static void GBanking_Progress_slotButtonClicked(GtkButton *button,
                                                gpointer user_data);


gboolean GBanking_Progress_slotDelete(GtkWidget *w,
                                      GdkEvent *event,
                                      gpointer user_data);







#endif



