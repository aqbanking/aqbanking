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
#include <gwenhywfar/pathmanager.h> // for GWEN_PathManager_GetPaths

#include <stdio.h>

#include <qbanking/qbanking.h>





int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QBanking *ab;
  QTranslator translator(0);

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);

  ab=new QBanking("qt3-wizard", 0);
  if (ab->init()) {
    DBG_ERROR(0, "Error on QBanking::init");
    return 2;
  }

  QString languageCode = QTextCodec::locale();
  languageCode.truncate(2);

  GWEN_STRINGLIST *sl =
    GWEN_PathManager_GetPaths(AB_PM_LIBNAME, AB_PM_DATADIR);
  assert(sl);
  QString datadir(GWEN_StringList_FirstString(sl));
  GWEN_StringList_free(sl);
  QDir i18ndir = datadir;
  if (!i18ndir.exists())
    DBG_INFO(0, "Datadir %s does not exist.", i18ndir.path().ascii());
  i18ndir.cd("i18n");
  if (!i18ndir.exists())
    DBG_INFO(0, "I18ndir %s does not exist.", i18ndir.path().ascii());

  // no need to specify ".qm" suffix; QTranslator tries that itself
  if (translator.load(languageCode,
		      i18ndir.path())) {
    DBG_DEBUG(0, "I18N available for your language");
    app.installTranslator(&translator);
  }
  else {
    DBG_WARN(0, "Internationalisation is not available for your language %s", 
	     languageCode.ascii());
  }

  QObject::connect(&app,SIGNAL(lastWindowClosed()),
                   &app,SLOT(quit()));

  ab->setupDialog();

  if (ab->fini()) {
    DBG_ERROR(0, "Error on QBanking::fini");
  }
  DBG_INFO(0, "QBanking::fini done");

  delete ab;

  return 0;
}





