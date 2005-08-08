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

#ifndef GBANKING_MSGBOX_P_H
#define GBANKING_MSGBOX_P_H

#include "gmsgbox.h"


#define GBANKING_MSGBOX_ID "GB_MSGBOX"



typedef struct GBANKING_MSGBOX GBANKING_MSGBOX;
struct GBANKING_MSGBOX {
  AB_BANKING *banking;
  GWEN_TYPE_UINT32 flags;
  GtkWidget *widget;
  GtkWidget *textLabel;
  GtkWidget *button1;
  GtkWidget *button2;
  GtkWidget *button3;
};

void GBanking_MsgBox_free(GBANKING_MSGBOX *wd);
void GBanking_MsgBox_freeData(gpointer data);

void GBanking_MsgBox_slotButtonClicked(GtkButton *button,
                                       gpointer user_data);



#endif /* GBANKING_MSGBOX_P_H */



