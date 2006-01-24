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

#ifndef GB_PROGRESS_P_H
#define GB_PROGRESS_P_H

#include <gtk/gtk.h>

#include "gbprogress.h"

#define GB_PROGRESS_ID "GB_PROGRESS"



typedef struct GB_PROGRESS GB_PROGRESS;
struct GB_PROGRESS {
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

void GB_Progress_free(GB_PROGRESS *wd);
void GB_Progress_freeData(gpointer data);


static void GB_Progress_slotButtonClicked(GtkButton *button,
                                          gpointer user_data);


gboolean GB_Progress_slotDelete(GtkWidget *w,
                                GdkEvent *event,
                                gpointer user_data);







#endif



