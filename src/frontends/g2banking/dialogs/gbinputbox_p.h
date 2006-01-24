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

#ifndef GB_INPUTBOX_P_H
#define GB_INPUTBOX_P_H

#include "gbinputbox.h"


#define GB_INPUTBOX_ID "GB_INPUTBOX"



typedef struct GB_INPUTBOX GB_INPUTBOX;
struct GB_INPUTBOX {
  AB_BANKING *banking;
  GWEN_TYPE_UINT32 flags;
  GtkWidget *widget;
  GtkWidget *textLabel;
  GtkWidget *inputEntry;
  GtkWidget *confirmLabel;
  GtkWidget *confirmEntry;
  int accepted;
};

void GBanking_InputBox_free(GB_INPUTBOX *wd);
void GBanking_InputBox_freeData(gpointer data);

GtkWidget *GBanking_InputBox_new(AB_BANKING *ab,
                                 GWEN_TYPE_UINT32 flags,
                                 const char *title,
                                 const char *text,
                                 int minLen,
                                 int maxLen,
                                 GtkWidget *parent);


#endif /* GB_INPUTBOX_P_H */



