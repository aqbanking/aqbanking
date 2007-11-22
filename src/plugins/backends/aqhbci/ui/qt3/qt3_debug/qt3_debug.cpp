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
#include <qdir.h>

#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>

#include <stdio.h>

#include "logmanager.h"




int main(int argc, char **argv) {
  QApplication app(argc, argv);
  LogManager *hb;
  int result;
  string hdir;
  QTranslator translator(0);

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Notice);
  //GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevelNotice);
  //GWEN_Logger_SetLevel("aqhbci", GWEN_LoggerLevelNotice);

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

  hdir=QDir::homeDirPath().local8Bit().data();
  hdir+="/";
  hdir+=".aqbanking";

  hb=new LogManager(hdir.c_str(), 0, "LogManager", false);
  app.setMainWidget(hb);
  hb->show();

  result=app.exec();

  delete hb;

  // if (result==QDialog::Accepted)
  return 0;
  // return 2;
}





