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
#include "gbjobview_p.h"
#include "gbjoblist.h"

#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <aqbanking/banking.h>

#include <glade/glade-xml.h>



GtkWidget *GBanking_JobView_new(AB_BANKING *ab, int fullWindow) {
  GBANKING_JOBVIEW *wd;
  GladeXML *xml;

  GWEN_NEW_OBJECT(GBANKING_JOBVIEW, wd);
  assert(ab);

  if (fullWindow) {
    xml=GBanking_GladeXml_new(ab, "g2banking.glade", "GBJobView");
    assert(xml);
    g_assert((wd->jobView=glade_xml_get_widget(xml, "GBJobView")));
  }
  else {
    xml=GBanking_GladeXml_new(ab, "g2banking.glade", "GBJobViewContent");
    assert(xml);
    g_assert((wd->jobView=glade_xml_get_widget(xml, "GBJobViewContent")));
  }

  gtk_object_set_data_full(GTK_OBJECT(wd->jobView),
                           GBANKING_JOBVIEW_ID,
                           wd,
                           GBanking_JobView_freeData);

  wd->banking=ab;
  g_assert((wd->jobListView=glade_xml_get_widget(xml, "jobListView")));
  g_assert((wd->jobListView=GB_JobList_new(ab,
                                                 wd->jobView,
                                                 wd->jobListView)));
  g_assert((wd->dequeueButton=glade_xml_get_widget(xml, "dequeueButton")));
  g_assert((wd->execButton=glade_xml_get_widget(xml, "execButton")));

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
                            I18N("Execution Error"),
                            I18N("Some errors occurred while executing "
                              "the outbox."),
                            I18N("Dismiss"), 0, 0);
    }
    GB_JobList_Update(wd->jobListView);

    ctx=AB_ImExporterContext_new();
    rv=AB_Banking_GatherResponses(wd->banking, ctx);
    if (!rv) {
      rv=GBanking_ImportContext(wd->banking, ctx);
      if (rv) {
        if (rv==AB_ERROR_NOFN) {
          DBG_WARN(GBANKING_LOGDOMAIN,
                   "ImportContext function not implemented by application");
        }
        else {
          DBG_WARN(GBANKING_LOGDOMAIN,
                   "Error processing queue results (%d)", rv);
          AB_Banking_MessageBox(wd->banking,
                                AB_BANKING_MSG_FLAGS_TYPE_ERROR |
                                AB_BANKING_MSG_FLAGS_SEVERITY_NORMAL,
                                I18N("Execution Error"),
                                I18N("Some errors occurred while processing "
                                     "the outbox results."),
                                I18N("Dismiss"), 0, 0);
        }
      }
    }
    else {
      DBG_ERROR(GBANKING_LOGDOMAIN,
                "Error gathering queue results (%d)", rv);
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














