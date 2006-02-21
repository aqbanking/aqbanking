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

#ifndef GB_MSGBOX_P_H
#define GB_MSGBOX_P_H

#include "gbmsgbox_l.h"


#define GB_MSGBOX_ID "GB_MSGBOX"



typedef struct GB_MSGBOX GB_MSGBOX;
struct GB_MSGBOX {
  AB_BANKING *banking;
  GWEN_TYPE_UINT32 flags;
  GtkWidget *widget;
  GtkWidget *textLabel;
  GtkWidget *button1;
  GtkWidget *button2;
  GtkWidget *button3;
};

void GB_MsgBox_free(GB_MSGBOX *wd);
void GB_MsgBox_freeData(gpointer data);

void GB_MsgBox_slotButtonClicked(GtkButton *button,
                                 gpointer user_data);



#endif /* GB_MSGBOX_P_H */



