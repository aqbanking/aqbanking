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

#include <aqgeldkarte/provider.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>

#include <stdio.h>

#include <qbanking/qbanking.h>
#include "accountview.h"




int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QBanking *ab;
  AccountView *av;
  int result;
  AB_PROVIDER *pro;
  QTranslator translator(0);

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);
  //GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevelInfo);
  GWEN_Logger_SetLevel("aqgeldkarte", GWEN_LoggerLevelDebug);
  //GWEN_Logger_SetLevel("aqbanking", GWEN_LoggerLevelInfo);

  QString datadir(PKGDATADIR);
  if (translator.load(QTextCodec::locale()+QString(".qm"),
		      datadir + QString("/i18n/"))) {
    DBG_INFO(0, "I18N available for your language");
    app.installTranslator(&translator);
  }
  else {
    DBG_WARN(0, "Internationalisation is not available for your language");
  }

  ab=new QBanking("aqgeldkarte-qt-wizard", 0);
  if (ab->init()) {
    DBG_ERROR(0, "Error on QBanking::init");
    return 2;
  }

  pro=ab->getProvider("aqgeldkarte");
  assert(pro);

  QObject::connect(&app,SIGNAL(lastWindowClosed()),
                   &app,SLOT(quit()));

  av=new AccountView(ab, pro);
  av->init();
  app.setMainWidget(av);
  av->show();

  result=app.exec();

  av->fini();

  if (ab->fini()) {
    DBG_ERROR(0, "Error on QBanking::fini");
  }
  DBG_INFO(0, "QBanking::fini done");

  delete av;
  delete ab;

  return 0;
}





