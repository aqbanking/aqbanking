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


#include "gbsimplebox_p.h"
#include "gbanking_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <glade/glade-xml.h>


GtkWidget *GB_SimpleBox_new(AB_BANKING *ab,
                            GWEN_TYPE_UINT32 id,
                            GWEN_TYPE_UINT32 flags,
                            const char *title,
                            const char *text){
  GB_SIMPLEBOX *wd;
  GladeXML *xml;

  GWEN_NEW_OBJECT(GB_SIMPLEBOX, wd);
  wd->id=id;
  wd->flags=flags;

  xml=GBanking_GladeXml_new(ab, "g2banking.glade", "GBSimpleBox");
  assert(xml);

  g_assert((wd->widget=glade_xml_get_widget(xml, "GBSimpleBox")));
  gtk_object_set_data_full(GTK_OBJECT(wd->widget),
                           GB_SIMPLEBOX_ID,
                           wd,
                           GB_SimpleBox_freeData);

  g_assert((wd->msgText=glade_xml_get_widget(xml, "textLabel")));

  gtk_window_set_title(GTK_WINDOW(wd->widget), title);
  gtk_window_set_position(GTK_WINDOW(wd->widget), GTK_WIN_POS_CENTER);

  if (text) {
    GWEN_BUFFER *txtBuf;

    txtBuf=GWEN_Buffer_new(0, strlen(text), 0, 1);
    GBanking_GetHtmlText(ab, text, txtBuf);
    gtk_label_set_markup(GTK_LABEL(wd->msgText),
                         GWEN_Buffer_GetStart(txtBuf));
    GWEN_Buffer_free(txtBuf);
  }

  gtk_object_ref(GTK_OBJECT(wd->widget));
  gtk_widget_show(wd->widget);

  /* handle events */
  while (g_main_iteration (FALSE));

  return wd->widget;
}



void GB_SimpleBox_freeData(gpointer data){
  GB_SIMPLEBOX *wd;

  wd=(GB_SIMPLEBOX*)data;
  g_assert(wd);

  GWEN_FREE_OBJECT(wd);
}



GWEN_TYPE_UINT32 GB_SimpleBox_GetId(GtkWidget *w){
  GB_SIMPLEBOX *wd;

  wd=gtk_object_get_data(GTK_OBJECT(w), GB_SIMPLEBOX_ID);
  g_assert(wd);
  return wd->id;
}








