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


#include "gbanking.h"
#include "jobview_p.h"
#include "joblist.h"
#include "interface.h"
#include "callbacks.h"
#include "support.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <aqbanking/banking.h>


#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)


GtkWidget*
create_GJobView2 (void)
{
  GtkWidget *GJobView;
  GtkWidget *scrolledwindow2;
  GtkWidget *jobListView;
  GtkWidget *vbuttonbox1;
  GtkWidget *dequeueButton;
  GtkWidget *alignment2;
  GtkWidget *hbox3;
  GtkWidget *image2;
  GtkWidget *label5;
  GtkWidget *execButton;
  GtkWidget *alignment1;
  GtkWidget *hbox2;
  GtkWidget *image1;
  GtkWidget *label4;


  GJobView= gtk_hbox_new (FALSE, 0);
  gtk_widget_set_name (GJobView, "GJobView2");
  gtk_widget_show (GJobView);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_name (scrolledwindow2, "scrolledwindow2");
  gtk_widget_show (scrolledwindow2);
  gtk_box_pack_start (GTK_BOX (GJobView), scrolledwindow2, TRUE, TRUE, 0);

  jobListView = gtk_tree_view_new ();
  gtk_widget_set_name (jobListView, "jobListView");
  gtk_widget_show (jobListView);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), jobListView);
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (jobListView), TRUE);

  vbuttonbox1 = gtk_vbutton_box_new ();
  gtk_widget_set_name (vbuttonbox1, "vbuttonbox1");
  gtk_widget_show (vbuttonbox1);
  gtk_box_pack_start (GTK_BOX (GJobView), vbuttonbox1, FALSE, FALSE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (vbuttonbox1), GTK_BUTTONBOX_START);

  dequeueButton = gtk_button_new ();
  gtk_widget_set_name (dequeueButton, "dequeueButton");
  gtk_widget_show (dequeueButton);
  gtk_container_add (GTK_CONTAINER (vbuttonbox1), dequeueButton);
  GTK_WIDGET_SET_FLAGS (dequeueButton, GTK_CAN_DEFAULT);

  alignment2 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_set_name (alignment2, "alignment2");
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (dequeueButton), alignment2);

  hbox3 = gtk_hbox_new (FALSE, 2);
  gtk_widget_set_name (hbox3, "hbox3");
  gtk_widget_show (hbox3);
  gtk_container_add (GTK_CONTAINER (alignment2), hbox3);

  image2 = gtk_image_new_from_stock ("gtk-delete", GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_name (image2, "image2");
  gtk_widget_show (image2);
  gtk_box_pack_start (GTK_BOX (hbox3), image2, FALSE, FALSE, 0);

  label5 = gtk_label_new_with_mnemonic (_("Dequeue"));
  gtk_widget_set_name (label5, "label5");
  gtk_widget_show (label5);
  gtk_box_pack_start (GTK_BOX (hbox3), label5, FALSE, FALSE, 0);

  execButton = gtk_button_new ();
  gtk_widget_set_name (execButton, "execButton");
  gtk_widget_show (execButton);
  gtk_container_add (GTK_CONTAINER (vbuttonbox1), execButton);
  GTK_WIDGET_SET_FLAGS (execButton, GTK_CAN_DEFAULT);

  alignment1 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_set_name (alignment1, "alignment1");
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (execButton), alignment1);

  hbox2 = gtk_hbox_new (FALSE, 2);
  gtk_widget_set_name (hbox2, "hbox2");
  gtk_widget_show (hbox2);
  gtk_container_add (GTK_CONTAINER (alignment1), hbox2);

  image1 = gtk_image_new_from_stock ("gtk-execute", GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_name (image1, "image1");
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox2), image1, FALSE, FALSE, 0);

  label4 = gtk_label_new_with_mnemonic (_("Execute"));
  gtk_widget_set_name (label4, "label4");
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (hbox2), label4, FALSE, FALSE, 0);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (GJobView, GJobView, "GJobView");
  GLADE_HOOKUP_OBJECT (GJobView, GJobView, "hbox1");
  GLADE_HOOKUP_OBJECT (GJobView, scrolledwindow2, "scrolledwindow2");
  GLADE_HOOKUP_OBJECT (GJobView, jobListView, "jobListView");
  GLADE_HOOKUP_OBJECT (GJobView, vbuttonbox1, "vbuttonbox1");
  GLADE_HOOKUP_OBJECT (GJobView, dequeueButton, "dequeueButton");
  GLADE_HOOKUP_OBJECT (GJobView, alignment2, "alignment2");
  GLADE_HOOKUP_OBJECT (GJobView, hbox3, "hbox3");
  GLADE_HOOKUP_OBJECT (GJobView, image2, "image2");
  GLADE_HOOKUP_OBJECT (GJobView, label5, "label5");
  GLADE_HOOKUP_OBJECT (GJobView, execButton, "execButton");
  GLADE_HOOKUP_OBJECT (GJobView, alignment1, "alignment1");
  GLADE_HOOKUP_OBJECT (GJobView, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (GJobView, image1, "image1");
  GLADE_HOOKUP_OBJECT (GJobView, label4, "label4");

  return GJobView;
}





GtkWidget *GBanking_JobView_new(AB_BANKING *ab, GtkWidget *parent) {
  GBANKING_JOBVIEW *wd;

  GWEN_NEW_OBJECT(GBANKING_JOBVIEW, wd);

  g_assert((wd->jobView=create_GJobView2()));
  gtk_object_set_data_full(GTK_OBJECT(wd->jobView),
                           GBANKING_JOBVIEW_ID,
                           wd,
                           GBanking_JobView_freeData);

  if (parent)
    gtk_widget_reparent(wd->jobView, parent);

  wd->banking=ab;
  g_assert((wd->jobListView=lookup_widget(wd->jobView, "jobListView")));
  g_assert((wd->jobListView=GBanking_JobList_new(ab,
                                                 wd->jobView,
                                                 lookup_widget(wd->jobView,
                                                               "jobListView")
                                                )));
  g_assert((wd->dequeueButton=lookup_widget(wd->jobView, "dequeueButton")));
  g_assert((wd->execButton=lookup_widget(wd->jobView, "execButton")));

  gtk_signal_connect(GTK_OBJECT (wd->dequeueButton),
                     "clicked",
                     GTK_SIGNAL_FUNC(GBanking_JobView_slotButtonClicked),
                     wd);
  gtk_signal_connect(GTK_OBJECT(wd->execButton), "clicked",
                     GTK_SIGNAL_FUNC(GBanking_JobView_slotButtonClicked),
                     wd);

  return wd->jobView;
}




void GBanking_JobView_free(GBANKING_JOBVIEW *wd){
  GWEN_FREE_OBJECT(wd);
}



void GBanking_JobView_freeData(gpointer data){
  GBANKING_JOBVIEW *wd;

  wd=(GBANKING_JOBVIEW*)data;
  g_assert(wd);

  GBanking_JobView_free(wd);
}



void GBanking_JobView_slotButtonClicked(GtkButton *button,
                                        gpointer user_data){
  GBANKING_JOBVIEW *wd;
  const gchar *name;

  wd=user_data;
  g_assert(wd);

  name=gtk_widget_get_name(GTK_WIDGET(button));
  if (strcmp(name, "dequeueButton")==0) {
    /* dequeue */
  }
  else if (strcmp (name, "execButton")==0) {
    int rv;
    AB_IMEXPORTER_CONTEXT *ctx;

    /* execute */
    rv=AB_Banking_ExecuteQueue(wd->banking);
    if (rv) {
      AB_Banking_MessageBox(wd->banking,
                            AB_BANKING_MSG_FLAGS_TYPE_ERROR |
                            AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                            _("Execution Error"),
                            _("Some errors occurred while executing "
                              "the outbox."),
                            _("Dismiss"), 0, 0);
    }
    GBanking_JobList_Update(wd->jobListView);

    ctx=AB_ImExporterContext_new();
    rv=AB_Banking_GatherResponses(wd->banking, ctx);
    if (!rv) {
      rv=GBanking_ImportContext(wd->banking, ctx);
      if (rv) {
        DBG_WARN(GBANKING_LOGDOMAIN,
                 "Error processing queue results (%d)", rv);
        AB_Banking_MessageBox(wd->banking,
                              AB_BANKING_MSG_FLAGS_TYPE_ERROR |
                              AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                              _("Execution Error"),
                              _("Some errors occurred while processing "
                                "the outbox results."),
                              _("Dismiss"), 0, 0);
      }
    }
    AB_ImExporterContext_free(ctx);

  }
  else {
    DBG_WARN(0, "Unknown button \"%s\"", name);
  }
  /* redraw the child widgets */
  while (g_main_iteration (FALSE));
}



int GBanking_JobView_HasChanged(GBANKING_JOBVIEW *wd){
  GWEN_TYPE_UINT32 i;

  i=GBanking_GetLastQueueUpdate(wd->banking);
  if (i!=wd->lastQueueUpdate) {
    wd->lastQueueUpdate=i;
    return 1;
  }

  return 0;
}














