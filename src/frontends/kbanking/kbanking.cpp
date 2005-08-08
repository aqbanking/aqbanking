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


#include "kbanking.h"
#include "flagstaff.h"

#include <assert.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qapplication.h>
#include <qdatetime.h>
#include <qwidget.h>
#include <qtranslator.h>
#include <qtextcodec.h>


#include <gwenhywfar/debug.h>



KBanking::KBanking(const char *appname,
                   const char *fname)
:QBanking(appname, fname)
,_translator(0){
  _flagStaff=new FlagStaff();
}



KBanking::~KBanking(){
  if (_translator) {
    qApp->removeTranslator(_translator);
    delete _translator;
  }
  delete _flagStaff;
}



FlagStaff *KBanking::flagStaff(){
  return _flagStaff;
}



int KBanking::executeQueue(){
  int rv;

  rv=QBanking::executeQueue();
  _flagStaff->queueUpdated();
  return rv;
}



bool KBanking::importContext(AB_IMEXPORTER_CONTEXT *ctx){
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while(ai) {
    if (!importAccountInfo(ai))
      return false;
    ai=AB_ImExporterContext_GetNextAccountInfo(ctx);
  }
  return true;
}



bool KBanking::importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai){
  DBG_NOTICE(0, "Import account info function not overloaded");
  return false;
}



bool KBanking::importContext(AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_TYPE_UINT32 flags) {
  return importContext(ctx);
}




bool KBanking::importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai,
				 GWEN_TYPE_UINT32 flags) {
  return importAccountInfo(ai);
}



int KBanking::init(){
  int rv;

  rv=QBanking::init();
  if (rv)
    return rv;

  _translator=new QTranslator(0);
  if (_translator->load(QTextCodec::locale()+QString(".qm"),
			QString(DATADIR "/i18n/"))) {
    DBG_INFO(0, "I18N available for your language");
    qApp->installTranslator(_translator);
  }
  else {
    DBG_WARN(0, "Internationalisation is not available for your language");
    delete _translator;
    _translator=0;
  }

  return 0;
}



int KBanking::fini(){
  int rv;

  rv=QBanking::fini();
  if (_translator) {
    qApp->removeTranslator(_translator);
    delete _translator;
    _translator=0;
  }

  return rv;
}








