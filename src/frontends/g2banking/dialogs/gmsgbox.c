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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "gmsgbox_p.h"
#include "gbanking.h"

#include "interface.h"
#include "callbacks.h"
#include "support.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <ctype.h>



GtkWidget *GBanking_MsgBox_new(AB_BANKING *ab,
                               GWEN_TYPE_UINT32 flags,
                               const char *title,
                               const char *text,
                               const char *b1,
                               const char *b2,
                               const char *b3,
                               GtkWidget *parent){
  GBANKING_MSGBOX *wd;

  GWEN_NEW_OBJECT(GBANKING_MSGBOX, wd);
  wd->banking=ab;
  wd->flags=flags;

  g_assert((wd->widget=create_GMessageBox()));
  gtk_object_set_data_full(GTK_OBJECT(wd->widget),
                           GBANKING_MSGBOX_ID,
                           wd,
                           GBanking_MsgBox_freeData);
  if (parent)
    gtk_widget_reparent(wd->widget, parent);

  g_assert((wd->textLabel=lookup_widget(wd->widget, "textLabel")));
  g_assert((wd->button1=lookup_widget(wd->widget, "button1")));
  g_assert((wd->button2=lookup_widget(wd->widget, "button2")));
  g_assert((wd->button3=lookup_widget(wd->widget, "button3")));

  gtk_window_set_title(GTK_WINDOW(wd->widget), title);
  gtk_window_set_position(GTK_WINDOW(wd->widget), GTK_WIN_POS_CENTER);

  gtk_label_set_text(GTK_LABEL(wd->textLabel), text);

  gtk_object_ref(GTK_OBJECT(wd->widget));

  gtk_button_set_label(GTK_BUTTON(wd->button1), b1);

  if (b2) {
    gtk_button_set_label(GTK_BUTTON(wd->button2), b2);
    gtk_widget_show(wd->button2);
  }

  if (b3) {
    gtk_button_set_label(GTK_BUTTON(wd->button3), b3);
    gtk_widget_show(wd->button3);
  }

  gtk_signal_connect(GTK_OBJECT (wd->button1),
                     "clicked",
                     GTK_SIGNAL_FUNC(GBanking_MsgBox_slotButtonClicked),
                     wd);
  gtk_signal_connect(GTK_OBJECT (wd->button2),
                     "clicked",
                     GTK_SIGNAL_FUNC(GBanking_MsgBox_slotButtonClicked),
                     wd);
  gtk_signal_connect(GTK_OBJECT (wd->button3),
                     "clicked",
                     GTK_SIGNAL_FUNC(GBanking_MsgBox_slotButtonClicked),
                     wd);

  /* handle events */
  while (g_main_iteration (FALSE));

  return wd->widget;

}



void GBanking_MsgBox_free(GBANKING_MSGBOX *wd){
  GWEN_FREE_OBJECT(wd);
}



void GBanking_MsgBox_freeData(gpointer data){
  GBANKING_MSGBOX *wd;

  wd=(GBANKING_MSGBOX*)data;
  g_assert(wd);

  GBanking_MsgBox_free(wd);
}



void GBanking_MsgBox_slotButtonClicked(GtkButton *button,
                                       gpointer user_data){
  GBANKING_MSGBOX *wd;
  const gchar *name;
  int response;

  wd=user_data;

  g_assert(wd);

  name=gtk_widget_get_name(GTK_WIDGET(button));
  if (strcmp(name, "button1")==0) {
    response=1;
  }
  else if (strcmp(name, "button2")==0) {
    response=2;
  }
  else if (strcmp(name, "button3")==0) {
    response=3;
  }
  else {
    DBG_WARN(0, "Unknown button \"%s\"", name);
    response=0;
  }
  gtk_dialog_response(GTK_DIALOG(wd->widget), response);
  /* redraw the child widgets */
  while (g_main_iteration (FALSE));
}











