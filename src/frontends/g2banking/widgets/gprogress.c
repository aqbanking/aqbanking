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


#include "gprogress_p.h"
#include "gbanking_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <glade/glade-xml.h>

#include <time.h>



GtkWidget *GBanking_Progress_new(AB_BANKING *ab,
                                 GWEN_TYPE_UINT32 id) {
  GBANKING_PROGRESS *wd;
  GladeXML *xml;

  GWEN_NEW_OBJECT(GBANKING_PROGRESS, wd);
  wd->banking=ab;

  xml=GBanking_GladeXml_new(ab, "g2banking.glade", "GConnectionDialog");
  assert(xml);

  g_assert((wd->dialog=glade_xml_get_widget(xml, "GConnectionDialog")));

  gtk_object_set_data_full(GTK_OBJECT(wd->dialog),
                           GBANKING_PROGRESS_ID,
                           wd,
                           GBanking_Progress_freeData);

  g_assert((wd->titleText=glade_xml_get_widget(xml, "titleText")));
  g_assert((wd->logText=glade_xml_get_widget(xml, "logText")));
  g_assert((wd->progressBar=glade_xml_get_widget(xml, "progressBar")));
  g_assert((wd->closeButton=glade_xml_get_widget(xml, "closeButton")));
  g_assert((wd->abortButton=glade_xml_get_widget(xml, "abortButton")));

  gtk_widget_set_size_request(GTK_WIDGET(wd->dialog), 500, 400);

  gtk_widget_set_sensitive(GTK_WIDGET(wd->abortButton), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(wd->closeButton), TRUE);

  gtk_signal_connect(GTK_OBJECT (wd->abortButton),
                     "clicked",
                     GTK_SIGNAL_FUNC(GBanking_Progress_slotButtonClicked),
                     wd);
  gtk_signal_connect(GTK_OBJECT(wd->closeButton), "clicked",
                     GTK_SIGNAL_FUNC(GBanking_Progress_slotButtonClicked),
                     wd);
  gtk_signal_connect(GTK_OBJECT(wd->dialog), "delete-event",
                     GTK_SIGNAL_FUNC(GBanking_Progress_slotDelete),
                     wd);

  wd->logBuffer=GWEN_Buffer_new(0, 1024, 0, 1);
  wd->id=id;
  return wd->dialog;
}



void GBanking_Progress_free(GBANKING_PROGRESS *wd){
  fprintf(stderr, "deleting progress.\n");
  GWEN_Buffer_free(wd->logBuffer);
  GWEN_FREE_OBJECT(wd);
}



void GBanking_Progress_freeData(gpointer data){
  GBANKING_PROGRESS *wd;

  wd=(GBANKING_PROGRESS*)data;
  g_assert(wd);

  GBanking_Progress_free(wd);
}



gboolean GBanking_Progress_slotDelete(GtkWidget *w,
                                      GdkEvent *event,
                                      gpointer user_data) {
  GBANKING_PROGRESS *wd;

  g_assert(w);
  wd=gtk_object_get_data(GTK_OBJECT(w), GBANKING_PROGRESS_ID);
  g_assert(wd);

  return !(wd->allowClose);
}



void GBanking_Progress_slotButtonClicked(GtkButton *button,
                                         gpointer user_data){
  GBANKING_PROGRESS *wd;
  const gchar *name;

  wd=user_data;

  g_assert(wd);

  name=gtk_widget_get_name(GTK_WIDGET(button));
  if (strcmp(name, "abortButton")==0) {
    wd->aborted=1;
  }
  else if (strcmp (name, "closeButton")==0) {
    if (wd->finished)
      gtk_widget_hide_all (wd->dialog);
  }
  else {
    DBG_WARN(0, "Unknown button \"%s\"", name);
  }
  /* redraw the child widgets */
  while (g_main_iteration (FALSE));
}



int GBanking_Progress_Start(GtkWidget *w,
                            const char *title,
                            const char *text,
                            GWEN_TYPE_UINT32 total){
  GBANKING_PROGRESS *wd;

  wd=gtk_object_get_data(GTK_OBJECT(w), GBANKING_PROGRESS_ID);
  g_assert(wd);

  wd->totalProgress=total;
  wd->currentProgress=0;
  wd->allowClose=0;
  wd->finished=0;
  wd->aborted=0;
  gtk_window_set_title(GTK_WINDOW(wd->dialog), title);

  if (text) {
    GWEN_BUFFER *txtBuf;

    txtBuf=GWEN_Buffer_new(0, strlen(text), 0, 1);
    GBanking_GetHtmlText(wd->banking, text, txtBuf);
    gtk_label_set_markup(GTK_LABEL(wd->titleText),
                         GWEN_Buffer_GetStart(txtBuf));
    GWEN_Buffer_free(txtBuf);
  }

  gtk_progress_set_percentage(GTK_PROGRESS(wd->progressBar), 0.0);
  GWEN_Buffer_Reset(wd->logBuffer);
  gtk_text_buffer_set_text
      (gtk_text_view_get_buffer(GTK_TEXT_VIEW (wd->logText)), "", -1);
  gtk_widget_set_sensitive(GTK_WIDGET(wd->abortButton), TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(wd->closeButton), FALSE);
  gtk_widget_show(wd->dialog);
  /* handle events */
  while (g_main_iteration (FALSE));
  return 0;
}



int GBanking_Progress_Advance(GtkWidget *w, GWEN_TYPE_UINT32 progress){
  gdouble fract;
  GBANKING_PROGRESS *wd;

  wd=gtk_object_get_data(GTK_OBJECT(w), GBANKING_PROGRESS_ID);
  g_assert(wd);

  if (progress!=AB_BANKING_PROGRESS_NONE) {
    wd->currentProgress=progress;
    fract=((gdouble)progress/(gdouble)(wd->totalProgress));
    gtk_progress_set_percentage(GTK_PROGRESS(wd->progressBar), fract);
  }

  /* handle events */
  while (g_main_iteration (FALSE));
  return wd->aborted;
}



int GBanking_Progress_Log(GtkWidget *w,
                          AB_BANKING_LOGLEVEL level,
                          const char *text){
  int i;
  gint pos;
  GtkTextIter end;
  GBANKING_PROGRESS *wd;
  struct tm *t;
  time_t tt;
  char tbuf[10];

  wd=gtk_object_get_data(GTK_OBJECT(w), GBANKING_PROGRESS_ID);
  g_assert(wd);

  tt=time(0);
  t=localtime(&tt);
  snprintf(tbuf, sizeof(tbuf), "%02d:%02d:%02d ",
           t->tm_hour, t->tm_min, t->tm_sec);
  GWEN_Buffer_AppendString(wd->logBuffer, tbuf);

  if (text) {
    GWEN_BUFFER *txtBuf;

    txtBuf=GWEN_Buffer_new(0, strlen(text), 0, 1);
    GBanking_GetHtmlText(wd->banking, text, txtBuf);
    GWEN_Buffer_AppendString(wd->logBuffer,
                             GWEN_Buffer_GetStart(txtBuf));
    GWEN_Buffer_free(txtBuf);
  }

  i=strlen(text);
  if (text[i-1]!='\n')
    GWEN_Buffer_AppendByte(wd->logBuffer, '\n');

  pos=0;
  gtk_text_buffer_set_text
      (gtk_text_view_get_buffer(GTK_TEXT_VIEW (wd->logText)),
       GWEN_Buffer_GetStart(wd->logBuffer), -1);

  gtk_text_buffer_get_iter_at_offset
      (gtk_text_view_get_buffer(GTK_TEXT_VIEW (wd->logText)),
       &end,
       GWEN_Buffer_GetUsedBytes(wd->logBuffer));
  gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW (wd->logText),
                               &end,
                               0, FALSE, 0, 0);
  /* handle events */
  while (g_main_iteration (FALSE));
  return wd->aborted;
}



int GBanking_Progress_End(GtkWidget *w){
  GBANKING_PROGRESS *wd;

  wd=gtk_object_get_data(GTK_OBJECT(w), GBANKING_PROGRESS_ID);
  g_assert(wd);

  wd->allowClose=1;
  wd->finished=1;
  gtk_widget_set_sensitive(GTK_WIDGET(wd->abortButton), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(wd->closeButton), TRUE);
  /* handle events */
  while (g_main_iteration (FALSE));
  return wd->aborted;
}



GWEN_TYPE_UINT32 GBanking_Progress_GetId(GtkWidget *w){
  GBANKING_PROGRESS *wd;

  wd=gtk_object_get_data(GTK_OBJECT(w), GBANKING_PROGRESS_ID);
  g_assert(wd);

  return wd->id;
}









