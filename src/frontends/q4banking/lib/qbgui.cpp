/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "qbgui.h"
#include "qbprintdialog.h"


#include <qwidget.h>

#include <aqbanking/abgui.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/debug.h>

#define I18S(msg) msg



QBGui::QBGui(QBanking *qb)
:QT4_Gui()
,_qbanking(qb)  {
  AB_Gui_Extend(getCInterface(), qb->getCInterface());
}



QBGui::~QBGui() {
}



int QBGui::_extractHTML(const char *text, GWEN_BUFFER *tbuf) {
  GWEN_XMLNODE *xmlNode;

  xmlNode=GWEN_XMLNode_fromString(text, strlen(text),
				  GWEN_XML_FLAGS_DEFAULT |
				  GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS);
  if (xmlNode==NULL) {
    DBG_DEBUG(0, "here");
    return -1;
  }
  else {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(xmlNode, "html", 0, 0);
    if (nn) {
      GWEN_XMLNODE *on, *onn;
      int rv;

      on=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
      onn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "qt");
      GWEN_XMLNode_AddChild(on, onn);
      GWEN_XMLNode_AddChildrenOnly(onn, nn, 1);

      /* text contains HTML tag, take it */
      rv=GWEN_XMLNode_toBuffer(on, tbuf, GWEN_XML_FLAGS_DEFAULT);
      GWEN_XMLNode_free(on);
      if (rv) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing data to stream");
	GWEN_XMLNode_free(xmlNode);
	return -1;
      }
    }
    else {
      GWEN_XMLNode_free(xmlNode);
      return 1;
    }
  }
  GWEN_XMLNode_free(xmlNode);
  return 0;
}



int QBGui::print(const char *docTitle,
		 const char *docType,
		 const char *descr,
		 const char *text,
		 uint32_t guiid) {
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  int rv;

  buf1=GWEN_Buffer_new(0, strlen(descr)+32, 0, 1);

  if (!_extractHTML(descr, buf1)) {
    descr=GWEN_Buffer_GetStart(buf1);
  }
  buf2=GWEN_Buffer_new(0, strlen(text)+32, 0, 1);
  if (!_extractHTML(text, buf2)) {
    text=GWEN_Buffer_GetStart(buf2);
  }

  QBPrintDialog pdlg(_qbanking,
		     docTitle, docType, descr, text, getParentWidget(),
		     "printdialog", true);

  if (pdlg.exec()==QDialog::Accepted)
    rv=0;
  else
    rv=GWEN_ERROR_USER_ABORTED;

  GWEN_Buffer_free(buf2);
  GWEN_Buffer_free(buf1);
  return rv;
}



