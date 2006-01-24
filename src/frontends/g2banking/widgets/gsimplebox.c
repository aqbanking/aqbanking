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


#include "gsimplebox_p.h"
#include "gbanking_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <glade/glade-xml.h>


GtkWidget *GBanking_SimpleBox_new(AB_BANKING *ab,
                                  GWEN_TYPE_UINT32 id,
                                  GWEN_TYPE_UINT32 flags,
                                  const char *title,
                                  const char *text){
  GBANKING_SIMPLEBOX *wd;
  GladeXML *xml;

  GWEN_NEW_OBJECT(GBANKING_SIMPLEBOX, wd);
  wd->id=id;
  wd->flags=flags;

  xml=GBanking_GladeXml_new(ab, "g2banking.glade", "GSimpleBox");
  assert(xml);

  g_assert((wd->widget=glade_xml_get_widget(xml, "GSimpleBox")));
  gtk_object_set_data_full(GTK_OBJECT(wd->widget),
                           GBANKING_SIMPLEBOX_ID,
                           wd,
                           GBanking_SimpleBox_freeData);

  g_assert((wd->msgText=glade_xml_get_widget(xml, "textLabel")));

  gtk_window_set_title(GTK_WINDOW(wd->widget), title);
  gtk_window_set_position(GTK_WINDOW(wd->widget), GTK_WIN_POS_CENTER);

  gtk_label_set_text(GTK_LABEL(wd->msgText), text);

  gtk_object_ref(GTK_OBJECT(wd->widget));
  gtk_widget_show(wd->widget);

  /* handle events */
  while (g_main_iteration (FALSE));

  return wd->widget;
}



void GBanking_SimpleBox_free(GBANKING_SIMPLEBOX *wd){
  GWEN_FREE_OBJECT(wd);
}



void GBanking_SimpleBox_freeData(gpointer data){
  GBANKING_SIMPLEBOX *wd;

  wd=(GBANKING_SIMPLEBOX*)data;
  g_assert(wd);

  fprintf(stderr, "deleting.\n");
  GBanking_SimpleBox_free(wd);
}



GWEN_TYPE_UINT32 GBanking_SimpleBox_GetId(GtkWidget *w){
  GBANKING_SIMPLEBOX *wd;

  wd=gtk_object_get_data(GTK_OBJECT(w), GBANKING_SIMPLEBOX_ID);
  g_assert(wd);
  return wd->id;
}








