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


#include <qapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>

#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>

#include <stdio.h>

#include <qbanking/qbanking.h>


#include "qbhelpbrowser.h"

#include <qapplication.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <qmessagebox.h>


#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif



int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QTranslator translator(0);
  QStringList paths;
  QString qs;
  QString shortLoc;
  const char *s;
  int i;

  QString datadir(PKGDATADIR);
  if (translator.load(QTextCodec::locale()+QString(".qm"),
		      datadir + QString("/i18n/"))) {
    DBG_INFO(0, "I18N available for your language");
    app.installTranslator(&translator);
  }
  else {
    DBG_WARN(0, "Internationalisation is not available for your language");
  }

  QObject::connect(&app,SIGNAL(lastWindowClosed()),
                   &app,SLOT(quit()));

  if (app.argc()<2) {
    QMessageBox::critical(0,
                          QWidget::tr("Argument Error"),
                          QWidget::tr("Missing URL"),
                          QWidget::tr("Dismiss"));
    return 1;
  }

  s=QTextCodec::locale();
  if (!s)
    s="de";
  shortLoc=QString::fromUtf8(s);
  i=shortLoc.find('_');
  if (i)
    shortLoc=shortLoc.left(i);

  if (app.argc()>2) {
    QString appHelpPath;

    // application help path given, insert it into path list
    appHelpPath=QString::fromUtf8(app.argv()[2]);
    paths+=appHelpPath+DIRSEP+shortLoc;
    paths+=appHelpPath+DIRSEP+shortLoc+DIRSEP "images";
    paths+=appHelpPath+DIRSEP "C";
    paths+=appHelpPath+DIRSEP "C" DIRSEP "images";
  }

  qs=QString::fromUtf8(QBANKING_HELPDIR DIRSEP)+shortLoc;
  paths+=qs;
  qs=QString::fromUtf8(QBANKING_HELPDIR DIRSEP)+shortLoc+DIRSEP "images";
  paths+=qs;
  qs=QString::fromUtf8(QBANKING_HELPDIR DIRSEP "C");
  paths+=qs;
  qs=QString::fromUtf8(QBANKING_HELPDIR DIRSEP "C" DIRSEP "images");
  paths+=qs;

  qs=QString::fromUtf8(app.argv()[1]);
  QBHelpBrowser *hb=new QBHelpBrowser(qs, paths);
  hb->resize(800, 600);
  hb->show();
  app.setMainWidget(hb);
  return app.exec();
}





