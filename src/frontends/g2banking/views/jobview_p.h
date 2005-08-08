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



#ifndef GBANKING_JOBVIEW_P_H
#define GBANKING_JOBVIEW_P_H

#define GBANKING_JOBVIEW_ID "GB_JOBVIEW"

#include "jobview.h"


typedef struct GBANKING_JOBVIEW GBANKING_JOBVIEW;
struct GBANKING_JOBVIEW {
  AB_BANKING *banking;
  GtkWidget *jobView;
  GtkWidget *jobListView;
  GtkWidget *dequeueButton;
  GtkWidget *execButton;
  GWEN_TYPE_UINT32 lastQueueUpdate;
};

void GBanking_JobView_free(GBANKING_JOBVIEW *wd);
void GBanking_JobView_freeData(gpointer data);
void GBanking_JobView_slotButtonClicked(GtkButton *button,
                                        gpointer user_data);


int GBanking_JobView_HasChanged(GBANKING_JOBVIEW *wd);


#endif /* GBANKING_JOBVIEW_P_H */



