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

#include "gbanking_p.h"
#include "gsimplebox.h"
#include "gprogress.h"
#include "ginputbox.h"
#include "gmsgbox.h"

#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/bio_buffer.h>
#include <gwenhywfar/xml.h>

#include <glade/glade-xml.h>

#include <errno.h>
#include <string.h>


#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif



GWEN_INHERIT(AB_BANKING, GBANKING)



AB_BANKING *GBanking_new(const char *appName,
                         const char *fname){
  GBANKING *gb;
  AB_BANKING *ab;

  ab=AB_Banking_new(appName, fname);
  GWEN_NEW_OBJECT(GBANKING, gb);
  GWEN_INHERIT_SETDATA(AB_BANKING, GBANKING,
                       ab, gb,
                       GBanking_FreeData);

  AB_Banking_SetMessageBoxFn(ab, GBanking_MessageBox);
  AB_Banking_SetInputBoxFn(ab, GBanking_InputBox);
  AB_Banking_SetShowBoxFn(ab, GBanking_ShowBox);
  AB_Banking_SetHideBoxFn(ab, GBanking_HideBox);
  AB_Banking_SetProgressStartFn(ab, GBanking_ProgressStart);
  AB_Banking_SetProgressAdvanceFn(ab, GBanking_ProgressAdvance);
  AB_Banking_SetProgressLogFn(ab, GBanking_ProgressLog);
  AB_Banking_SetProgressEndFn(ab, GBanking_ProgressEnd);

  return ab;
}



void GBanking_FreeData(void *bp, void *p) {
  GBANKING *gb;

  DBG_INFO(GBANKING_LOGDOMAIN, "Destroying GBANKING");
  gb=(GBANKING*)p;

  GWEN_FREE_OBJECT(gb);
}



int GBanking_MessageBox(AB_BANKING *ab,
                        GWEN_TYPE_UINT32 flags,
                        const char *title,
                        const char *text,
                        const char *b1,
                        const char *b2,
                        const char *b3){
  GBANKING *gb;
  GtkWidget *w;
  gint result;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  w=GBanking_MsgBox_new(ab,
                        flags,
                        title,
                        text,
                        b1, b2, b3, gb->parentWidget);
  result=gtk_dialog_run(GTK_DIALOG(w));
  gtk_widget_destroy(w);
  if (result<1 || result>3) {
    DBG_INFO(GBANKING_LOGDOMAIN, "Bad result %d", result);
    return 0;
  }
  return result;
}



int GBanking_InputBox(AB_BANKING *ab,
                      GWEN_TYPE_UINT32 flags,
                      const char *title,
                      const char *text,
                      char *buffer,
                      int minLen,
                      int maxLen) {
  GBANKING *gb;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  if (!GBanking_GetInput(ab, flags, title, text, buffer, minLen, maxLen,
                         gb->parentWidget)) {
    DBG_ERROR(GBANKING_LOGDOMAIN, "Error in input, user gave up");
    return AB_ERROR_USER_ABORT;
  }

  return 0;
}



GWEN_TYPE_UINT32 GBanking_ShowBox(AB_BANKING *ab,
                                  GWEN_TYPE_UINT32 flags,
                                  const char *title,
                                  const char *text){
  GBANKING *gb;
  GtkWidget *w;
  GWEN_TYPE_UINT32 id;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);
  id=++(gb->lastWidgetId);
  w=GBanking_SimpleBox_new(ab, id, flags, title, text);
  gb->simpleBoxes=g_slist_prepend(gb->simpleBoxes, w);

  return id;
}



GtkWidget *GBanking__findSimpleBox(AB_BANKING *ab, GWEN_TYPE_UINT32 id) {
  GBANKING *gb;
  GtkWidget *w;
  int size;
  int i;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  size=g_slist_length(gb->simpleBoxes);
  w=0;
  for (i=0; i< size; i++) {
    w=g_slist_nth_data(gb->simpleBoxes, i);
    if (id==0 || GBanking_SimpleBox_GetId(w)==id)
      break;
  }
  return w;
}



GtkWidget *GBanking__findProgressWidget(AB_BANKING *ab, GWEN_TYPE_UINT32 id){
  GBANKING *gb;
  GtkWidget *w;
  int size;
  int i;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  size=g_slist_length(gb->progressWidgets);
  w=0;
  for (i=0; i< size; i++) {
    w=g_slist_nth_data(gb->progressWidgets, i);
    if (id==0 || GBanking_Progress_GetId(w)==id)
      break;
  }
  return w;
}



void GBanking_HideBox(AB_BANKING *ab, GWEN_TYPE_UINT32 id){
  GBANKING *gb;
  GtkWidget *w;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  w=GBanking__findSimpleBox(ab, id);
  if (w) {
    gb->simpleBoxes=g_slist_remove(gb->simpleBoxes, w);
    gtk_widget_destroy(w);
  }
  else {
    DBG_ERROR(GBANKING_LOGDOMAIN, "Box not found");
  }
}



GWEN_TYPE_UINT32 GBanking_ProgressStart(AB_BANKING *ab,
                                        const char *title,
                                        const char *text,
                                        GWEN_TYPE_UINT32 total){
  GBANKING *gb;
  GtkWidget *w;
  GWEN_TYPE_UINT32 id;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  id=++(gb->lastWidgetId);
  w=GBanking_Progress_new(ab, id);
  gtk_widget_ref(w);
  GBanking_Progress_Start(w, title, text, total);
  gb->progressWidgets=g_slist_prepend(gb->progressWidgets, w);

  return id;
}



int GBanking_ProgressAdvance(AB_BANKING *ab,
                             GWEN_TYPE_UINT32 id,
                             GWEN_TYPE_UINT32 progress){
  GBANKING *gb;
  GtkWidget *w;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  w=GBanking__findProgressWidget(ab, id);
  if (w) {
    return GBanking_Progress_Advance(w, progress);
  }
  else {
    DBG_ERROR(GBANKING_LOGDOMAIN, "Progress widget %d not found", id);
    return AB_ERROR_INVALID;
  }
  return 0;
}



int GBanking_ProgressLog(AB_BANKING *ab,
                         GWEN_TYPE_UINT32 id,
                         AB_BANKING_LOGLEVEL level,
                         const char *text){
  GBANKING *gb;
  GtkWidget *w;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  w=GBanking__findProgressWidget(ab, id);
  if (w) {
    return GBanking_Progress_Log(w, level, text);
  }
  else {
    DBG_ERROR(GBANKING_LOGDOMAIN, "Progress widget not found");
    return AB_ERROR_INVALID;
  }
  return 0;
}



int GBanking_ProgressEnd(AB_BANKING *ab, GWEN_TYPE_UINT32 id){
  GBANKING *gb;
  GtkWidget *w;
  int rv;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  w=GBanking__findProgressWidget(ab, id);
  if (w) {
    rv=GBanking_Progress_End(w);
    gb->progressWidgets=g_slist_remove(gb->progressWidgets, w);
    gtk_widget_unref(w);
  }
  else {
    DBG_ERROR(GBANKING_LOGDOMAIN, "Progress widget not found");
  }
  return 0;
}



GWEN_TYPE_UINT32 GBanking_GetLastAccountUpdate(const AB_BANKING *ab){
  GBANKING *gb;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  return gb->_lastAccountUpdate;
}



GWEN_TYPE_UINT32 GBanking_GetLastQueueUpdate(const AB_BANKING *ab){
  GBANKING *gb;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  return gb->_lastQueueUpdate;
}



void GBanking_AccountsUpdated(AB_BANKING *ab){
  GBANKING *gb;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  gb->_lastAccountUpdate++;;

}



void GBanking_QueueUpdated(AB_BANKING *ab){
  GBANKING *gb;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  gb->_lastQueueUpdate++;
}




int GBanking_ImportContext(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx){
  GBANKING *gb;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  if (gb->importContextFn)
    return gb->importContextFn(ab, ctx);
  else
    return AB_ERROR_NOFN;
}




void GBanking_SetImportContextFn(AB_BANKING *ab,
				 GBANKING_IMPORTCONTEXT_FN fn){
  GBANKING *gb;

  assert(ab);
  gb=GWEN_INHERIT_GETDATA(AB_BANKING, GBANKING, ab);
  assert(gb);

  gb->importContextFn=fn;
}




int GBanking__extractText(const char *text, GWEN_BUFFER *tbuf) {
  GWEN_BUFFEREDIO *bio;
  GWEN_XMLNODE *xmlNode;
  GWEN_BUFFER *buf;
  int rv;

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(buf, text);
  GWEN_Buffer_Rewind(buf);
  bio=GWEN_BufferedIO_Buffer2_new(buf, 1);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 256);
  xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "html");
  rv=GWEN_XML_Parse(xmlNode, bio,
		    GWEN_XML_FLAGS_DEFAULT |
		    GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS |
		    GWEN_XML_FLAGS_NO_CONDENSE |
		    GWEN_XML_FLAGS_KEEP_CNTRL);
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  if (rv) {
    DBG_NOTICE(0, "here");
    GWEN_XMLNode_free(xmlNode);
    return -1;
  }
  else {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_GetFirstData(xmlNode);
    if (nn) {
      GWEN_Buffer_AppendString(tbuf, GWEN_XMLNode_GetData(nn));
    }
    else {
      GWEN_XMLNode_free(xmlNode);
      return 1;
    }
  }
  GWEN_XMLNode_free(xmlNode);
  return 0;
}



GladeXML *GBanking_GladeXml_new(AB_BANKING *ab,
                                const char *relFname,
                                const char *wname) {
  GWEN_BUFFER *fbuf;
  FILE *f;

  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(fbuf, PACKAGE_GLADE_DIR);
  GWEN_Buffer_AppendString(fbuf, DIRSEP);
  GWEN_Buffer_AppendString(fbuf, relFname);
  f=fopen(GWEN_Buffer_GetStart(fbuf), "r");
  if (f) {
    GladeXML *xml;

    fclose(f);
    xml=glade_xml_new(GWEN_Buffer_GetStart(fbuf), wname, PACKAGE);
    GWEN_Buffer_free(fbuf);
    if (!xml) {
      DBG_ERROR(0, "Error on glade_xml_new, no XML tree");
      return 0;
    }
    return xml;
  }
  else {
    DBG_ERROR(0, "Could not open file \"%s\": %s",
              GWEN_Buffer_GetStart(fbuf),
              strerror(errno));
    GWEN_Buffer_free(fbuf);
    return 0;
  }
}



