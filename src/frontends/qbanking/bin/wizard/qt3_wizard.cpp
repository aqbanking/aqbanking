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

#include <stdio.h>

#include <qbanking/qbanking.h>





int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QBanking *ab;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);

  ab=new QBanking("qt3-wizard", 0);
  if (ab->init()) {
    DBG_ERROR(0, "Error on QBanking::init");
    return 2;
  }

  QObject::connect(&app,SIGNAL(lastWindowClosed()),
                   &app,SLOT(quit()));

  ab->setupDialog();

  if (ab->fini()) {
    DBG_ERROR(0, "Error on QBanking::fini");
  }
  DBG_DEBUG(0, "QBanking::fini done");

  delete ab;

  return 0;
}





