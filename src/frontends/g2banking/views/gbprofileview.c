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


#include "gbanking_l.h"
#include "gbprofileview_p.h"
#include "gbprofilelist.h"

#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <aqbanking/banking.h>

#include <glade/glade-xml.h>



GtkWidget *GB_ProfileView_new(AB_BANKING *ab,
                              GWEN_DB_NODE *dbProfiles,
                              GtkWidget *parent) {
  GB_PROFILEVIEW *wd;
  GladeXML *xml;

  GWEN_NEW_OBJECT(GB_PROFILEVIEW, wd);

  xml=GBanking_GladeXml_new(ab, "g2banking.glade", "GBProfileView");
  assert(xml);

  g_assert((wd->profileView=glade_xml_get_widget(xml, "GBProfileView")));

  gtk_object_set_data_full(GTK_OBJECT(wd->profileView),
                           GB_PROFILEVIEW_ID,
                           wd,
                           GB_ProfileView_freeData);

  if (parent)
    gtk_widget_reparent(wd->profileView, parent);

  wd->banking=ab;
  g_assert((wd->profileListView=glade_xml_get_widget(xml,
                                                     "profileListView")));
  g_assert((wd->profileList=GB_ProfileList_new(wd->profileView,
                                               wd->profileListView)));
  g_assert((wd->newButton=glade_xml_get_widget(xml, "newButton")));
  g_assert((wd->editButton=glade_xml_get_widget(xml, "editButton")));
  g_assert((wd->copyButton=glade_xml_get_widget(xml, "copyButton")));
  g_assert((wd->deleteButton=glade_xml_get_widget(xml, "deleteButton")));

  gtk_signal_connect(GTK_OBJECT (wd->newButton),
                     "clicked",
                     GTK_SIGNAL_FUNC(GB_ProfileView_slotButtonClicked),
                     wd);
  gtk_signal_connect(GTK_OBJECT(wd->editButton), "clicked",
                     GTK_SIGNAL_FUNC(GB_ProfileView_slotButtonClicked),
                     wd);
  gtk_signal_connect(GTK_OBJECT(wd->copyButton), "clicked",
                     GTK_SIGNAL_FUNC(GB_ProfileView_slotButtonClicked),
                     wd);
  gtk_signal_connect(GTK_OBJECT(wd->deleteButton), "clicked",
                     GTK_SIGNAL_FUNC(GB_ProfileView_slotButtonClicked),
                     wd);

  GB_ProfileList_AddProfiles(wd->profileList, dbProfiles);

  return wd->profileView;
}



void GB_ProfileView_freeData(gpointer data){
  GB_PROFILEVIEW *wd;

  wd=(GB_PROFILEVIEW*)data;
  g_assert(wd);

  GWEN_FREE_OBJECT(wd);
}



void GB_ProfileView_slotButtonClicked(GtkButton *button,
                                      gpointer user_data){
  GB_PROFILEVIEW *wd;
  const gchar *name;

  wd=user_data;
  g_assert(wd);

  name=gtk_widget_get_name(GTK_WIDGET(button));
  DBG_ERROR(0, "Button pressed: \"%s\"", name);
  if (strcmp(name, "newButton")==0) {
    /* newButton */
  }
  else {
    DBG_WARN(0, "Unknown button \"%s\"", name);
  }
  /* redraw the child widgets */
  while (g_main_iteration (FALSE));
}




