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

#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/gui_be.h>

#include <stdio.h>

#include <q4banking/qbanking.h>
#include <q4banking/qbgui.h>





int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QBanking *ab;
  QBGui *gui;
  int err;
  const char *s;

  err=GWEN_Init();
  if (err) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Info);

  ab=new QBanking("qt4-wizard", 0);
  gui=new QBGui(ab);
  GWEN_Gui_SetGui(gui->getCInterface());
  ab->setGui(gui);

  GWEN_Logger_Open(0,
		   "qt4_wizard", 0,
		   GWEN_LoggerType_Console,
		   GWEN_LoggerFacility_User);

  // set loglevels
  s=getenv("LOGLEVEL");
  if (s) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(s);
    if (ll!=GWEN_LoggerLevel_Unknown) {
      GWEN_Logger_SetLevel(0, ll);
      DBG_WARN(0,
               "Overriding loglevel for Q4Banking with \"%s\"",
               s);
    }
    else {
      DBG_ERROR(0, "Unknown loglevel \"%s\"",
                s);
    }
  }
  else {
    GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Notice);
  }


  if (ab->init()) {
    DBG_ERROR(0, "Error on QBanking::init");
    return 2;
  }

  if (ab->onlineInit()) {
    DBG_ERROR(0, "Error on QBanking::onlineInit");
    return 2;
  }

  QObject::connect(&app,SIGNAL(lastWindowClosed()),
		   &app,SLOT(quit()));

  ab->setupDialog();

  if (ab->onlineFini()) {
    DBG_ERROR(0, "Error on QBanking::onlineFini");
  }
  if (ab->fini()) {
    DBG_ERROR(0, "Error on QBanking::fini");
  }
  DBG_DEBUG(0, "QBanking::fini done");

  delete ab;

  return 0;
}





