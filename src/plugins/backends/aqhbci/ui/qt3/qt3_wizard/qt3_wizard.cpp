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

#include <aqhbci/hbci.h>
#include <aqhbci/provider.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>

#include <stdio.h>

#include <qbanking/qbanking.h>
#include "hbcisettings.h"

#include "w_pintan_new.h"



int debug(int argc, char **argv) {
  QApplication app(argc, argv);
  QBanking *ab;
  AH_HBCI *hbci;
  AB_PROVIDER *pro;
  QTranslator translator(0);

  WizardPinTanNew *w;
  WizardInfo *wInfo;
  AH_MEDIUM *m;
  int rv;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);
  //GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevelInfo);
  //GWEN_Logger_SetLevel("aqhbci", GWEN_LoggerLevelNotice);
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

  ab=new QBanking("qt3_wizard", 0);
  if (ab->init()) {
    fprintf(stderr, "Error on init.\n");
    return 2;
  }

  pro=ab->getProvider("aqhbci");
  assert(pro);
  hbci=AH_Provider_GetHbci(pro);

  QObject::connect(&app,SIGNAL(lastWindowClosed()),
                   &app,SLOT(quit()));

  wInfo=new WizardInfo(hbci);

  if (1) {
    GWEN_TIME *ti;
    GWEN_BUFFER *bufName;

    bufName=GWEN_Buffer_new(0, 128, 0, 1);
    GWEN_Buffer_AppendString(bufName, "PINTAN-");
    ti=GWEN_CurrentTime();
    assert(ti);
    GWEN_Time_toString(ti, "YYYYMMDD-hhmmss", bufName);
    GWEN_Time_free(ti);
    m=AH_HBCI_MediumFactory(hbci,
                            "pintan", 0,
                            GWEN_Buffer_GetStart(bufName));
    GWEN_Buffer_free(bufName);
  }

  rv=AH_Medium_Mount(m);
  if (rv) {
    DBG_ERROR(0, "Could not mount medium (%d)", rv);
    return 3;
  }
  wInfo->setMedium(m);

  w=new WizardPinTanNew(ab, wInfo, 0, "WizardPinTanNew", TRUE);
  if (w->exec()==QDialog::Accepted) {
    DBG_NOTICE(0, "Accepted");
  }
  else {
    DBG_NOTICE(0, "Rejected");
  }

  rv=AH_Medium_Unmount(m, 1);
  if (rv) {
    DBG_ERROR(0, "Could not unmount medium (%d)", rv);
    return 3;
  }

  return 2;
}



int wizard(int argc, char **argv) {
  QApplication app(argc, argv);
  QBanking *ab;
  AH_HBCI *hbci;
  HBCISettings *hb;
  int result;
  AB_PROVIDER *pro;
  QTranslator translator(0);

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);
  //GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevelInfo);
  //GWEN_Logger_SetLevel("aqhbci", GWEN_LoggerLevelNotice);
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

  ab=new QBanking("kde_wizard", 0);
  if (ab->init()) {
    fprintf(stderr, "Error on init.\n");
    return 2;
  }

  pro=ab->getProvider("aqhbci");
  assert(pro);
  hbci=AH_Provider_GetHbci(pro);

  QObject::connect(&app,SIGNAL(lastWindowClosed()),
                   &app,SLOT(quit()));

  hb=new HBCISettings(hbci, ab, 0, "HBCISettings", false);
  app.setMainWidget(hb);
  hb->show();

  result=app.exec();

  if (ab->fini()) {
    fprintf(stderr, "Error on fini.\n");
  }
  fprintf(stderr, "FINI done.\n");

  delete hb;
  delete ab;

  // if (result==QDialog::Accepted)
  return 0;
  // return 2;
}



int main(int argc, char **argv) {
  if (argc>1) {
    if (strcmp(argv[1], "debug")==0)
      return debug(argc, argv);
  }
  return wizard(argc, argv);
}



