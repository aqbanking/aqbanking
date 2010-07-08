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
  const char *p=0;
  const char *p2=0;

  if (text==NULL)
    return 0;

  /* find begin of HTML area */
  p=text;
  while ((p=strchr(p, '<'))) {
    const char *t;

    t=p;
    t++;
    if (toupper(*t)=='H') {
      t++;
      if (toupper(*t)=='T') {
        t++;
        if (toupper(*t)=='M') {
          t++;
          if (toupper(*t)=='L') {
	    t++;
	    if (toupper(*t)=='>') {
	      break;
	    }
	  }
        }
      }
    }
    p++;
  } /* while */

  /* find end of HTML area */
  if (p) {
    p+=6; /* skip "<html>" */
    p2=p;
    while ((p2=strchr(p2, '<'))) {
      const char *t;
  
      t=p2;
      t++;
      if (toupper(*t)=='/') {
	t++;
	if (toupper(*t)=='H') {
	  t++;
	  if (toupper(*t)=='T') {
	    t++;
	    if (toupper(*t)=='M') {
	      t++;
	      if (toupper(*t)=='L') {
		t++;
		if (toupper(*t)=='>') {
		  break;
		}
	      }
	    }
	  }
	}
      }
      p2++;
    } /* while */
  }

  if (p && p2) {
    GWEN_Buffer_AppendString(tbuf, "<qt>");
    GWEN_Buffer_AppendBytes(tbuf, p, p2-p);
    GWEN_Buffer_AppendString(tbuf, "</qt>");
    return 0;
  }

  GWEN_Buffer_AppendString(tbuf, text);
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



