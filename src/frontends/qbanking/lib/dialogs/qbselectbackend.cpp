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

// QBanking includes
#include "qbselectbackend.h"
#include "qbanking.h"

// Gwenhywfar includes
#include <gwenhywfar/debug.h>

// QT includes
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtextbrowser.h>
#include <qlabel.h> // for qt4 setWordWrap(true)



QBSelectBackend::QBSelectBackend(QBanking *kb,
                                 const QString &backend,
                                 QWidget* parent,
                                 const char* name,
                                 bool modal,
                                 WFlags fl)
:QBSelectBackendUi(parent, name, modal, fl)
,_app(kb) {
  std::list<GWEN_PLUGIN_DESCRIPTION*>::iterator it;
  int idx=-1;
  int i=0;

  _plugins=_app->getProviderDescrs();
  for (it=_plugins.begin(); it!=_plugins.end(); it++) {
    const char *s;

    s=GWEN_PluginDescription_GetName(*it);
    if (s) {
      QString l;

      l=QString::fromUtf8(s);
      if (!backend.isEmpty() && l.upper()==backend.upper())
        idx=i;
      l+=" - ";
      s=GWEN_PluginDescription_GetShortDescr(*it);
      if (s)
        l+=QString::fromUtf8(s);
      backendCombo->insertItem(l);
      i++;
    }
  }

  connect(backendCombo, SIGNAL(activated(int)),
          this, SLOT(slotActivated(int)));
  connect(helpButton, SIGNAL(clicked()),
          this, SLOT(slotHelp()));
  connect(okButton, SIGNAL(clicked()),
          this, SLOT(accept()));
  connect(abortButton, SIGNAL(clicked()),
          this, SLOT(reject()));

  if (idx!=-1) {
    backendCombo->setCurrentItem(idx);
    slotActivated(idx);
  }
  else
    slotActivated(0);

#if (QT_VERSION >= 0x040000)
  // In qt4, QLabel has word-wrap disabled by default
  textLabel1->setWordWrap(true);
#endif // QT_VERSION >= 4
}



QBSelectBackend::~QBSelectBackend() {
  _app->clearPluginDescrs(_plugins);
}



const QString &QBSelectBackend::getSelectedBackend() const {
  return _selectedBackend;
}



void QBSelectBackend::slotActivated(int idx) {
  std::list<GWEN_PLUGIN_DESCRIPTION*>::iterator it;

  for (it=_plugins.begin(); it!=_plugins.end(); it++) {
    if (idx--==0) {
      GWEN_BUFFER *buf;
      QString l;
      int rv;

      _selectedBackend=QString::fromUtf8(GWEN_PluginDescription_GetName(*it));
      buf=GWEN_Buffer_new(0, 512, 0, 1);
      rv=GWEN_PluginDescription_GetLongDescrByFormat(*it, "html", buf);
      if (rv) {
        const char *s;

        DBG_INFO(0, "No long HTML description");
        s=GWEN_PluginDescription_GetLongDescr(*it);
        if (s)
          GWEN_Buffer_AppendString(buf, s);
      }
      l="<qt>";
      l+=QString::fromUtf8(GWEN_Buffer_GetStart(buf))+"</qt>";
      GWEN_Buffer_free(buf);
      backendBrowser->setText(l);
      break;
    }
  }
}



void QBSelectBackend::slotHelp() {
  _app->invokeHelp("QBSelectBackend", "none");
}



QString QBSelectBackend::selectBackend(QBanking *kb,
                                       const QString &backend,
                                       QWidget* parent) {
  QBSelectBackend dlg(kb, backend, parent);

  if (dlg.exec()==QDialog::Accepted)
    return dlg.getSelectedBackend();
  else
    return "";
}






#include "qbselectbackend.moc"


