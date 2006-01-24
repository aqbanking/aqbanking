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

#include "ginputbox_p.h"
#include "gbanking_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>

#include <glade/glade-xml.h>

#include <ctype.h>



GtkWidget *GBanking_InputBox_new(AB_BANKING *ab,
                                 GWEN_TYPE_UINT32 flags,
                                 const char *title,
                                 const char *text,
                                 int minLen,
                                 int maxLen,
                                 GtkWidget *parent){
  GBANKING_INPUTBOX *wd;
  GladeXML *xml;

  GWEN_NEW_OBJECT(GBANKING_INPUTBOX, wd);
  wd->banking=ab;
  wd->flags=flags;

  xml=GBanking_GladeXml_new(ab, "g2banking.glade", "GInputBox");
  assert(xml);

  g_assert((wd->widget=glade_xml_get_widget(xml, "GInputBox")));
  gtk_object_set_data_full(GTK_OBJECT(wd->widget),
                           GBANKING_INPUTBOX_ID,
                           wd,
                           GBanking_InputBox_freeData);

  g_assert((wd->textLabel=glade_xml_get_widget(xml, "textLabel")));
  g_assert((wd->inputEntry=glade_xml_get_widget(xml, "inputEntry")));
  g_assert((wd->confirmLabel=glade_xml_get_widget(xml, "confirmLabel")));
  g_assert((wd->confirmEntry=glade_xml_get_widget(xml, "confirmEntry")));

  gtk_window_set_title(GTK_WINDOW(wd->widget), title);

  if (text) {
    GWEN_BUFFER *txtBuf;

    txtBuf=GWEN_Buffer_new(0, strlen(text), 0, 1);
    GBanking_GetHtmlText(ab, text, txtBuf);
    gtk_label_set_markup(GTK_LABEL(wd->textLabel),
                         GWEN_Buffer_GetStart(txtBuf));
    GWEN_Buffer_free(txtBuf);
  }

  gtk_object_ref(GTK_OBJECT(wd->widget));
  gtk_widget_show(wd->widget);

  gtk_entry_set_visibility(GTK_ENTRY(wd->inputEntry),
			   (flags & AB_BANKING_INPUT_FLAGS_SHOW));

  if (flags & AB_BANKING_INPUT_FLAGS_CONFIRM) {
    gtk_widget_show(wd->confirmLabel);
    gtk_widget_show(wd->confirmEntry);
    gtk_entry_set_visibility(GTK_ENTRY(wd->confirmEntry),
			     (flags & AB_BANKING_INPUT_FLAGS_SHOW));
  }

  /* handle events */
  while (g_main_iteration (FALSE));

  return wd->widget;

}



void GBanking_InputBox_free(GBANKING_INPUTBOX *wd){
  GWEN_FREE_OBJECT(wd);
}



void GBanking_InputBox_freeData(gpointer data){
  GBANKING_INPUTBOX *wd;

  wd=(GBANKING_INPUTBOX*)data;
  g_assert(wd);

  GBanking_InputBox_free(wd);
}



gboolean GBanking_GetInput(AB_BANKING *ab,
                           GWEN_TYPE_UINT32 flags,
                           const char *title,
			   const char *text,
                           char *buffer,
                           int minLen,
                           int maxLen,
                           GtkWidget *parent){
  GtkWidget *w;
  gint result;
  GBANKING_INPUTBOX *wd;
  gboolean haveNoPassword;

  w=GBanking_InputBox_new(ab, flags, title, text, minLen, maxLen, parent);
  wd=gtk_object_get_data(GTK_OBJECT(w), GBANKING_INPUTBOX_ID);
  g_assert(wd);

  haveNoPassword=1;
  while(haveNoPassword) {

    result=gtk_dialog_run(GTK_DIALOG(w));

    if (result==GTK_RESPONSE_OK) {
      const char *text1;
      int l1;

      DBG_ERROR(GBANKING_LOGDOMAIN, "Accepted");
      text1=gtk_entry_get_text(GTK_ENTRY(wd->inputEntry));
      if (text1) {
	l1=strlen(text1);

	/* check for limits */
	if (l1<minLen || l1>maxLen) {
	  /* TODO: show limits */
	  DBG_ERROR(GBANKING_LOGDOMAIN, "Out of limits");
          continue;
	}

	/* check for numeric input */
	if (flags & AB_BANKING_INPUT_FLAGS_NUMERIC) {
	  const char *p;
	  gboolean isOk;

	  isOk=TRUE;
	  p=text1;
	  while(*p) {
	    if (!isdigit(*p)) {
	      isOk=FALSE;
              break;
	    }
	    p++;
	  }
	  if (!isOk) {
	    /* TODO: show message */

	    DBG_ERROR(GBANKING_LOGDOMAIN, "Not numeric");
	    continue;
	  }
	} /* if numeric */

	if (flags & AB_BANKING_INPUT_FLAGS_CONFIRM) {
	  const char *text2;

	  text2=gtk_entry_get_text(GTK_ENTRY(wd->confirmEntry));
	  if (text2) {
	    if (strcmp(text1, text2)!=0) {
	      /* TODO: inputs differ */
	      DBG_ERROR(GBANKING_LOGDOMAIN, "Unmatching confirmation");
              continue;
	    }
	  }
	  else {
	    /* TODO: error message (no confirmation) */
	    DBG_ERROR(GBANKING_LOGDOMAIN, "Not confirmed");
	    continue;
	  }
	}

	/* if all tests were ok we did it ;-) */
	haveNoPassword=FALSE;
	/* copy password */
        DBG_ERROR(GBANKING_LOGDOMAIN,
                  "Copying this into buffer: \"%s\"", text1);
	strcpy(buffer, text1);
	break;

      } /* if text1 */
    } /* if accepted */
    else {
      /* user aborted */
      DBG_ERROR(GBANKING_LOGDOMAIN, "Bad result: %d", result);
      break;
    }

  } /* while */

  gtk_widget_destroy(w);
  return !haveNoPassword;
}














