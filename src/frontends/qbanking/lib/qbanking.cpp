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


#include "qbanking_p.h"
#include "qbprogress.h"
#include "qbsimplebox.h"
#include "qbinputbox.h"
#include "qbflagstaff.h"
#include "qbmapaccount.h"
#include "qbimporter.h"
#include "qbpickstartdate.h"
#include "qbprintdialog.h"
#include "qbselectbank.h"
#include "qbcfgmodule.h"
#include "qbcfgtabsettings.h"

#include "qbwcb_fast.h"
#include "qbwcb_simple.h"

#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgettransactions.h>

#include <assert.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qapplication.h>
#include <qdatetime.h>
#include <qwidget.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <qprocess.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/libloader.h>


#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif



QBanking::QBanking(const char *appname,
                   const char *fname)
:Banking(appname, fname)
,_parentWidget(0)
,_lastWidgetId(0)
,_logLevel(AB_Banking_LogLevelInfo)
,_translator(0)
,_simpleCallback(0)
,_fastCallback(0)
,_pluginManagerCfgModules(0)
,_appCfgModule(0){
  _flagStaff=new QBFlagStaff();
}



QBanking::~QBanking(){
  if (_translator) {
    qApp->removeTranslator(_translator);
    delete _translator;
  }
  delete _flagStaff;
}



void QBanking::setAppHelpPath(const QString &s) {
  _appHelpPath=s;
}



int QBanking::_extractHTML(const char *text, GWEN_BUFFER *tbuf) {
  GWEN_BUFFEREDIO *bio;
  GWEN_XMLNODE *xmlNode;
  GWEN_BUFFER *buf;
  int rv;

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(buf, text);
  GWEN_Buffer_Rewind(buf);
  bio=GWEN_BufferedIO_Buffer2_new(buf, 1);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 256);
  xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "html");
  rv=GWEN_XML_Parse(xmlNode, bio,
		    GWEN_XML_FLAGS_DEFAULT |
                    GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS);
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  if (rv) {
    DBG_INFO(0, "here");
    GWEN_XMLNode_free(xmlNode);
    return -1;
  }
  else {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(xmlNode, "html", 0, 0);
    if (nn) {
      GWEN_XMLNODE *on, *onn;

      on=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
      onn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "qt");
      GWEN_XMLNode_AddChild(on, onn);
      GWEN_XMLNode_AddChildrenOnly(onn, nn, 1);

        /* text contains HTML tag, take it */
      bio=GWEN_BufferedIO_Buffer2_new(tbuf, 0);
      GWEN_BufferedIO_SetWriteBuffer(bio, 0, 256);
      rv=GWEN_XMLNode_WriteToStream(on, bio, GWEN_XML_FLAGS_DEFAULT);
      GWEN_XMLNode_free(on);
      if (rv) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing data to stream");
	GWEN_BufferedIO_Abandon(bio);
	GWEN_BufferedIO_free(bio);
        GWEN_XMLNode_free(xmlNode);
	return -1;
      }
      else {
	rv=GWEN_BufferedIO_Close(bio);
	GWEN_BufferedIO_free(bio);
        if (rv) {
          GWEN_XMLNode_free(xmlNode);
          return -1;
        }
      }
    }
    else {
      GWEN_XMLNode_free(xmlNode);
      return 1;
    }
  }
  GWEN_XMLNode_free(xmlNode);
  return 0;
}



int QBanking::messageBox(GWEN_TYPE_UINT32 flags,
                         const char *title,
                         const char *text,
                         const char *b1,
                         const char *b2,
                         const char *b3){
  int rv;
  GWEN_BUFFER *buf;

  buf=GWEN_Buffer_new(0, strlen(text), 0, 1);
  if (!_extractHTML(text, buf)) {
    text=GWEN_Buffer_GetStart(buf);
  }

  switch(flags & AB_BANKING_MSG_FLAGS_TYPE_MASK) {
  case AB_BANKING_MSG_FLAGS_TYPE_WARN:
    rv=QMessageBox::warning(_parentWidget, QString::fromUtf8(title), QString::fromUtf8(text), 
			    b1 ? QString::fromUtf8(b1) : QString::null,
			    b2 ? QString::fromUtf8(b2) : QString::null,
			    b3 ? QString::fromUtf8(b3) : QString::null);
    break;
  case AB_BANKING_MSG_FLAGS_TYPE_ERROR:
    rv=QMessageBox::critical(_parentWidget, QString::fromUtf8(title), QString::fromUtf8(text), 
			     b1 ? QString::fromUtf8(b1) : QString::null,
			     b2 ? QString::fromUtf8(b2) : QString::null,
			     b3 ? QString::fromUtf8(b3) : QString::null);
    break;
  case AB_BANKING_MSG_FLAGS_TYPE_INFO:
  default:
    rv=QMessageBox::information(_parentWidget, QString::fromUtf8(title), QString::fromUtf8(text),
				b1 ? QString::fromUtf8(b1) : QString::null,
				b2 ? QString::fromUtf8(b2) : QString::null,
				b3 ? QString::fromUtf8(b3) : QString::null);
    break;
  }
  rv++;
  GWEN_Buffer_free(buf);
  return rv;
}



int QBanking::inputBox(GWEN_TYPE_UINT32 flags,
                       const char *title,
                       const char *text,
                       char *buffer,
                       int minLen,
                       int maxLen){
  GWEN_BUFFER *buf;

  buf=GWEN_Buffer_new(0, strlen(text), 0, 1);
  if (!_extractHTML(text, buf)) {
    text=GWEN_Buffer_GetStart(buf);
  }

  QBInputBox ib(QString::fromUtf8(title), QString::fromUtf8(text),
		flags, minLen, maxLen, 0, "InputBox", true);
  GWEN_Buffer_free(buf);
  if (ib.exec()==QDialog::Accepted) {
    QString s;

    s=ib.getInput();
    int len=s.length();
    if (len && len<maxLen-1) {
      // FIXME: QString::latin1() is most probably wrong here!
      // This means that the entered string will be passed into
      // AQ_BANKING in latin1 encoding, not in utf8. This should
      // probably be replaced by s.utf8()! But we need to watch
      // out for potentially breaking some people's PINs. For
      // those who had Umlauts in their PIN there should at least
      // be a commandline-tool available that will accept PINs in
      // a configurable encoding for reading, and a different PIN
      // for writing. -- cstim, 2005-09-15
      memmove(buffer, s.latin1(), len);
      buffer[len]=0;
    }
    else {
      DBG_ERROR(0, "Bad pin length");
      return AB_ERROR_INVALID;
    }
    return 0;
  }
  else {
    DBG_WARN(0, "Aborted by user");
    return AB_ERROR_USER_ABORT;
  }
}



GWEN_TYPE_UINT32 QBanking::showBox(GWEN_TYPE_UINT32 flags,
                                   const char *title,
                                   const char *text){
  GWEN_TYPE_UINT32 id;
  QBSimpleBox *b;
  QWidget *w;
  GWEN_BUFFER *buf;

  buf=GWEN_Buffer_new(0, strlen(text), 0, 1);
  if (!_extractHTML(text, buf)) {
    text=GWEN_Buffer_GetStart(buf);
  }

  w=_findProgressWidget(0);
  id=++_lastWidgetId;
  b=new QBSimpleBox(id, QString::fromUtf8(title),
		    QString::fromUtf8(text),
		    w, "SimpleBox",
		    Qt::WType_TopLevel | Qt::WType_Dialog | Qt::WShowModal);
  GWEN_Buffer_free(buf);
  //b->setModal(true);
  if (flags & AB_BANKING_SHOWBOX_FLAGS_BEEP)
    QApplication::beep();

  b->show();
  _simpleBoxWidgets.push_front(b);
  qApp->processEvents();
  return id;
}



void QBanking::hideBox(GWEN_TYPE_UINT32 id){
  if (_simpleBoxWidgets.size()==0) {
    DBG_WARN(0, "No simpleBox widgets");
    return;
  }
  if (id==0) {
    QBSimpleBox *b;

    b=_simpleBoxWidgets.front();
    b->close(true);
    _simpleBoxWidgets.pop_front();
  }
  else {
    std::list<QBSimpleBox*>::iterator it;
    for (it=_simpleBoxWidgets.begin(); it!=_simpleBoxWidgets.end(); it++) {
      if ((*it)->getId()==id) {
        (*it)->close(true);
        _simpleBoxWidgets.erase(it);
        break;
      }
    }
  }
  qApp->processEvents();
}



GWEN_TYPE_UINT32 QBanking::progressStart(const char *title,
                                         const char *text,
                                         GWEN_TYPE_UINT32 total){
  GWEN_TYPE_UINT32 id;
  QBProgress *pr;
  GWEN_BUFFER *buf;

  buf=GWEN_Buffer_new(0, strlen(text), 0, 1);
  if (!_extractHTML(text, buf)) {
    text=GWEN_Buffer_GetStart(buf);
  }

  _cleanupProgressWidgets();
  id=++_lastWidgetId;
  pr=new QBProgress(id,
                    QBProgress::ProgressTypeNormal,
                    QString::fromUtf8(title),
                    QString::fromUtf8(text),
                    QString::null,
                    _parentWidget, "ProgressWidget",
                    Qt::WType_Dialog | Qt::WShowModal);
  GWEN_Buffer_free(buf);
  if (pr->start(total)) {
    DBG_ERROR(0, "Could not start progress dialog");
    delete pr;
    return 0;
  }
  pr->show();
  _progressWidgets.push_front(pr);
  return id;
}



QBProgress *QBanking::_findProgressWidget(GWEN_TYPE_UINT32 id) {
  std::list<QBProgress*>::iterator it;

  if (_progressWidgets.size()==0) {
    DBG_WARN(0, "No progress widgets");
    return 0;
  }
  if (id==0)
    return _progressWidgets.front();
  for (it=_progressWidgets.begin(); it!=_progressWidgets.end(); it++) {
    if ((*it)->getId()==id)
      return *it;
  }

  DBG_WARN(0, "Progress widget \"%08x\" not found", (unsigned int)id);
  return 0;
}



void QBanking::_cleanupProgressWidgets() {
  std::list<QBProgress*>::iterator it;

  while(1) {
    bool del;

    del=false;
    for (it=_progressWidgets.begin(); it!=_progressWidgets.end(); it++) {
      if ((*it)->isClosed()) {
        delete *it;
        _progressWidgets.erase(it);
        del=true;
        break;
      }
    } /* for */
    if (!del)
      break;
  } /* while */
}



int QBanking::progressAdvance(GWEN_TYPE_UINT32 id,
                              GWEN_TYPE_UINT32 progress){
  QBProgress *pr;

  pr=_findProgressWidget(id);
  if (pr) {
    return pr->advance(progress);
  }
  else
    return AB_ERROR_NOT_FOUND;
}



int QBanking::progressLog(GWEN_TYPE_UINT32 id,
                          AB_BANKING_LOGLEVEL level,
                          const char *chartext){
  QBProgress *pr;
  QString text(QString::fromUtf8(chartext));

  // Necessary when passing this QString into the macros
  QCString local8Bit = text.local8Bit();

  if (level>_logLevel) {
    DBG_NOTICE(0, "Not logging this: %02d: %s (we are at %d)",
               level, local8Bit.data(), _logLevel);
    /* don't log this */
    return 0;
  }

  DBG_INFO(0, "%02d: %s", level, local8Bit.data());
  pr=_findProgressWidget(id);
  if (pr) {
    return pr->log(level, text);
  }
  else {
    return AB_ERROR_NOT_FOUND;
  }
}



int QBanking::progressEnd(GWEN_TYPE_UINT32 id){
  QBProgress *pr;
  int res;

  if (_progressWidgets.size()==0) {
    DBG_INFO(0, "No active progress widget");
    return AB_ERROR_NOT_FOUND;
  }

  res=AB_ERROR_NOT_FOUND;
  if (id==0) {
    pr=_progressWidgets.front();
    res=pr->end();
    _progressWidgets.pop_front();
  }
  else {
    std::list<QBProgress*>::iterator it;

    for (it=_progressWidgets.begin(); it!=_progressWidgets.end(); it++) {
      if ((*it)->getId()==id) {
	res=(*it)->end();
	_progressWidgets.erase(it);
	break;
      }
    }
  }

  return res;
}



void QBanking::setParentWidget(QWidget *w) {
  _parentWidget=w;
}

QWidget *QBanking::getParentWidget() {
  return _parentWidget;
}


QBFlagStaff *QBanking::flagStaff(){
  return _flagStaff;
}



int QBanking::enqueueJob(AB_JOB *j){
  int rv;

  rv=Banking::enqueueJob(j);

  flagStaff()->queueUpdated();
  return rv;
}



int QBanking::dequeueJob(AB_JOB *j){
  int rv;

  rv=Banking::dequeueJob(j);
  flagStaff()->queueUpdated();
  return rv;
}



int QBanking::executeQueue(AB_IMEXPORTER_CONTEXT *ctx){
  int rv;

  rv=Banking::executeQueue(ctx);
  flagStaff()->queueUpdated();
  return rv;
}



void QBanking::accountsUpdated(){
  flagStaff()->accountsUpdated();
}



void QBanking::invokeHelp(const QString &context,
                          const QString &subject){
  QString url;
  QProcess *p;

  DBG_ERROR(0, "Help wanted for \"%s\"/\"%s\"",
            context.latin1(), subject.latin1());

  url=context.lower()+".html";
  if (!subject.isEmpty())
    url+=+"#"+subject;

  p=new QProcess();
  p->addArgument("qb-help");
  p->addArgument(url);
  if (!_appHelpPath.isEmpty())
    p->addArgument(_appHelpPath);
  if (!p->launch(QString::null)) {
    DBG_ERROR(0, "Could not start process");
  }
  delete p;
}



bool QBanking::mapAccount(const AB_ACCOUNT *a){
  return false;
}



bool QBanking::addTransaction(const AB_ACCOUNT *a, const AB_TRANSACTION *t){
  return false;
}



bool QBanking::setAccountStatus(const AB_ACCOUNT *a,
                                const AB_ACCOUNT_STATUS *ast){
  return false;
}



AB_ACCOUNT *QBanking::_getAccount(const char *accountId){
  AB_ACCOUNT *a;

  a=AB_Banking_GetAccountByAlias(getCInterface(), accountId);
  if (!a) {
    // should not happen anyway
    QMessageBox::critical(_parentWidget,
			  QWidget::tr("Account Not Mapped"),
			  QWidget::tr("<qt>"
				      "<p>"
				      "The given application account has not "
				      "been mapped to banking accounts."
				      "</p>"
				      "</qt>"
				     ),
			  QMessageBox::Ok,QMessageBox::NoButton);
    return 0;
  }

  return a;
}




bool QBanking::requestBalance(const char *accountId){
  AB_ACCOUNT *a;
  int rv;

  if (!accountId) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Account id is required");
    return AB_ERROR_INVALID;
  }

  a=_getAccount(accountId);
  if (!a)
    return false;

  rv=AB_Banking_RequestBalance(getCInterface(),
			       AB_Account_GetBankCode(a),
			       AB_Account_GetAccountNumber(a));
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Request error (%d)",
	      rv);
    QMessageBox::critical(_parentWidget,
                          QWidget::tr("Queue Error"),
                          QWidget::tr("<qt>"
				      "<p>"
				      "Unable to enqueue your request."
				      "</p>"
				      "</qt>"
				     ),
			  QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN,
	   "Job successfully enqueued");
  return true;
}



bool QBanking::requestTransactions(const char *accountId,
                                   const QDate &fromDate,
                                   const QDate &toDate){
  AB_ACCOUNT *a;
  int rv;
  AB_JOB *job;

  if (!accountId) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Account id is required");
    return AB_ERROR_INVALID;
  }

  a=_getAccount(accountId);
  if (!a)
    return false;

  job=AB_JobGetTransactions_new(a);
  rv=AB_Job_CheckAvailability(job);
  if (rv) {
    DBG_NOTICE(0, "Job \"GetTransactions\" is not available (%d)", rv);
    AB_Job_free(job);
    QMessageBox::critical(_parentWidget,
			  QWidget::tr("Job Not Available"),
			  QWidget::tr("The job you requested is not available"
				      "with\n"
				      "the backend which handles "
				      "this account.\n"),
			  QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }

  /* check/set fromDate */
  if (fromDate.isValid()) {
    GWEN_TIME *ti1;

    ti1=GWEN_Time_new(fromDate.year(),
		      fromDate.month()-1,
		      fromDate.day(), 0, 0, 0, 0);
    AB_JobGetTransactions_SetFromTime(job, ti1);
    GWEN_Time_free(ti1);
  }
  else {
    GWEN_DB_NODE *db;

    db=getSharedData("qbanking");
    assert(db);
    db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			"banking/lastUpdate");
    if (db) {
      GWEN_TIME *ti;

      ti=GWEN_Time_fromDb(db);
      if (ti) {
	AB_JobGetTransactions_SetFromTime(job, ti);
	GWEN_Time_free(ti);
      }
    }
    else {
      QDate qd;
      GWEN_TIME *ti1;
      int days;
      int day, month, year;

      ti1=GWEN_CurrentTime();
      days=AB_JobGetTransactions_GetMaxStoreDays(job);
      if (days!=-1) {
	GWEN_TIME *ti2;
  
	ti2=GWEN_Time_fromSeconds(GWEN_Time_Seconds(ti1)-(60*60*24*days));
	GWEN_Time_free(ti1);
	ti1=ti2;
      }
      if (GWEN_Time_GetBrokenDownDate(ti1, &day, &month, &year)) {
	DBG_ERROR(0, "Bad date");
	qd=QDate();
      }
      else
	qd=QDate(year, month+1, day);
      GWEN_Time_free(ti1);

      QBPickStartDate psd(this, qd, QDate(), 3, 0, "PickStartDate", true);
      if (psd.exec()!=QDialog::Accepted) {
	AB_Job_free(job);
	return false;
      }
      qd=psd.getDate();
      if (qd.isValid()) {
	ti1=GWEN_Time_new(qd.year(), qd.month()-1, qd.day(), 0, 0, 0, 0);
	AB_JobGetTransactions_SetFromTime(job, ti1);
	GWEN_Time_free(ti1);
      }
    }
  }

  /* check/set toDate */
  if (toDate.isValid()) {
    GWEN_TIME *ti1;

    ti1=GWEN_Time_new(toDate.year(),
		      toDate.month()-1,
		      toDate.day(), 0, 0, 0, 0);
    AB_JobGetTransactions_SetToTime(job, ti1);
    GWEN_Time_free(ti1);
  }

  DBG_NOTICE(0, "Enqueuing job");
  rv=enqueueJob(job);
  AB_Job_free(job);
  if (rv) {
    DBG_ERROR(0,
	      "Request error (%d)",
	      rv);
    QMessageBox::critical(_parentWidget,
			  QWidget::tr("Queue Error"),
			  QWidget::tr("<qt>"
				      "<p>"
				      "Unable to enqueue your request."
				      "</p>"
				      "</qt>"
				     ),
			  QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }
  else {
    GWEN_TIME *ti1;
    GWEN_DB_NODE *dbT;

    ti1=GWEN_CurrentTime();
    dbT=getSharedData("qbanking");
    assert(dbT);
    dbT=GWEN_DB_GetGroup(dbT, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
			 "banking/lastUpdate");
    assert(dbT);
    if (GWEN_Time_toDb(ti1, dbT)) {
      DBG_ERROR(0, "Could not save time");
    }
    GWEN_Time_free(ti1);
  }

  DBG_INFO(AQBANKING_LOGDOMAIN,
	   "Job successfully enqueued");
  statusMessage(QWidget::tr("Jobs added to outbox"));

  return true;
}



void QBanking::setAccountAlias(AB_ACCOUNT *a, const char *alias){
  assert(a);
  assert(alias);

  AB_Banking_SetAccountAlias(getCInterface(), a, alias);
}



bool QBanking::askMapAccount(const char *id,
                             const char *bankCode,
                             const char *accountId){
  QBMapAccount *w;

  w=new QBMapAccount(this, bankCode, accountId,
                     _parentWidget, "MapAccount", true);
  if (w->exec()==QDialog::Accepted) {
    AB_ACCOUNT *a;

    a=w->getAccount();
    assert(a);
    DBG_NOTICE(0,
               "Mapping application account \"%s\" to "
               "online account \"%s/%s\"",
               id,
               AB_Account_GetBankCode(a),
               AB_Account_GetAccountNumber(a));
    setAccountAlias(a, id);
    delete w;
    return true;
  }

  delete w;
  return false;
}



bool QBanking::importContext(AB_IMEXPORTER_CONTEXT *ctx,
                             GWEN_TYPE_UINT32 flags){
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while(ai) {
    if (!importAccountInfo(ai, flags))
      return false;
    ai=AB_ImExporterContext_GetNextAccountInfo(ctx);
  }
  return true;
}


bool QBanking::importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai,
                                 GWEN_TYPE_UINT32 flags){
  DBG_NOTICE(0, "Import account info function not overloaded");
  return false;
}



bool QBanking::interactiveImport(){
  QBImporter *w;
  bool res;

  w=new QBImporter(this, _parentWidget, "Importer", true);
  if (!w->init()) {
    delete w;
    return false;
  }
  res=(w->exec()==QDialog::Accepted);
  res&=w->fini();
  delete w;
  return res;
}



int QBanking::init(){
  int rv;
  GWEN_PLUGIN_MANAGER *pm;

  rv=Banking::init();
  if (rv)
    return rv;

  _translator=new QTranslator(0);
  DBG_DEBUG(0, "PKGDATADIR="PKGDATADIR);
  // no need to specify ".qm" suffix; QTranslator tries that itself
  if (_translator->load(QTextCodec::locale(),
			QString(PKGDATADIR DIRSEP "i18n"))) {
    DBG_DEBUG(0, "I18N available for your language");
    qApp->installTranslator(_translator);
  }
  else {
    DBG_INFO(0, "Internationalisation is not available for your language %s", 
	     QTextCodec::locale());
    delete _translator;
    _translator=0;
  }

  _simpleCallback=new QBSimpleCallback(GWEN_WAITCALLBACK_ID_SIMPLE_PROGRESS);
  if (_simpleCallback->registerCallback()) {
    QMessageBox::critical(_parentWidget,
                          QWidget::tr("Internal Error"),
                          QWidget::tr("<qt>"
                                      "<p>"
                                      "Could not register SimpleCallback."
                                      "</p>"
                                      "<p>"
                                      "This is an internal error, please "
                                      "report it to "
                                      "<b>martin@libchipcard.de</b>"
                                      "</p>"
                                      "</qt>"
                                     ),
			  QMessageBox::Ok,QMessageBox::NoButton);
    delete _simpleCallback;
    _simpleCallback=0;
    return false;
  }

  _fastCallback=new QBFastCallback(GWEN_WAITCALLBACK_ID_FAST);
  if (_fastCallback->registerCallback()) {
    QMessageBox::critical(_parentWidget,
                          QWidget::tr("Internal Error"),
                          QWidget::tr("<qt>"
                                      "<p>"
                                      "Could not register FastCallback."
                                      "</p>"
                                      "<p>"
                                      "This is an internal error, please "
                                      "report it to "
                                      "<b>martin@libchipcard.de</b>"
                                      "</p>"
                                      "</qt>"
                                     ),
			  QMessageBox::Ok,QMessageBox::NoButton);
    delete _fastCallback;
    _fastCallback=0;
    return false;
  }

  /* create cfg module plugin manager */
  DBG_INFO(0, "Registering cfg module plugin manager");
  pm=GWEN_PluginManager_new(QBANKING_PM_CFGMODULE);
  GWEN_PluginManager_AddPathFromWinReg(pm,
                                       QBANKING_REGKEY_PATHS,
                                       QBANKING_REGKEY_CFGMODULEDIR);
  GWEN_PluginManager_AddPath(pm,
                             QBANKING_PLUGINS
                             DIRSEP
                             QBANKING_CFGMODULEDIR);
  if (GWEN_PluginManager_Register(pm)) {
    DBG_ERROR(0,
              "Could not register cfg module plugin manager");
    return false;
  }
  _pluginManagerCfgModules=pm;

  return 0;
}



int QBanking::fini(){
  int rv;
  std::list<QBCfgModule*>::iterator it;

  if (_fastCallback) {
    _fastCallback->unregisterCallback();
    delete _fastCallback;
    _fastCallback=0;
  }
  if (_simpleCallback) {
    _simpleCallback->unregisterCallback();
    delete _simpleCallback;
    _simpleCallback=0;
  }

  /* unload and free all config modules */
  for (it=_cfgModules.begin(); it!=_cfgModules.end(); it++) {
    GWEN_PLUGIN *pl;
    pl=(*it)->getPlugin();
    (*it)->setPlugin(0);
    delete *it;
    GWEN_Plugin_free(pl);
  }
  _cfgModules.clear();

  if (_pluginManagerCfgModules) {
    if (GWEN_PluginManager_Unregister(_pluginManagerCfgModules)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Could not unregister cfg module plugin manager");
    }
    GWEN_PluginManager_free(_pluginManagerCfgModules);
    _pluginManagerCfgModules=0;
  }

  /* deinit base class */
  rv=Banking::fini();
  if (_translator) {
    qApp->removeTranslator(_translator);
    delete _translator;
    _translator=0;
  }

  return rv;
}



void QBanking::outboxCountChanged(int count){
  flagStaff()->outboxCountChanged(count);
}



void QBanking::statusMessage(const QString &s){
  flagStaff()->statusMessage(s);
}



int QBanking::print(const char *docTitle,
                    const char *docType,
                    const char *descr,
                    const char *text){
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

  QBPrintDialog pdlg(this, docTitle, docType, descr, text, _parentWidget,
                     "printdialog", true);

  if (pdlg.exec()==QDialog::Accepted)
    rv=0;
  else
    rv=AB_ERROR_USER_ABORT;

  GWEN_Buffer_free(buf2);
  GWEN_Buffer_free(buf1);
  return rv;
}



std::string QBanking::QStringToUtf8String(const QString &qs) {
  if (qs.isEmpty())
    return "";
  else {
    QCString utfData=qs.utf8();
  
    // Note: This commented-out code below introduced an extra
    // char-by-char copying that I don't consider necessary. The
    // std::string contains "char", not "unsigned char", so the
    // right side below ("unsigned char") will be converted back to
    // "char" when it is appended to the std::string on the left
    // side. This is unnecessary. The only necessary thing is that
    // the "const char*" buffer of the QCString is copied into a
    // std::string, and exactly that happens when the std::string
    // return value is created, so there is really nothing more to
    // do.  -- cstim, 2005-09-19
  
     //std::string result;
     //int len;
     //int i;
     //len=utfData.length();
     //for (i=0; i<len; i++)
     //  result+=(unsigned char)utfData[i];
     //return result;
  
    return utfData.data();
  }
}



std::string QBanking::guiString(const char *s) {
  GWEN_BUFFER *tbuf;
  std::string res;

  assert(s);
  tbuf=GWEN_Buffer_new(0, strlen(s), 0, 1);
  if (_extractHTML(s, tbuf)) {
    GWEN_Buffer_free(tbuf);
    return s;
  }
  res=std::string(GWEN_Buffer_GetStart(tbuf),
                  GWEN_Buffer_GetUsedBytes(tbuf));
  GWEN_Buffer_free(tbuf);
  return res;
}



AB_BANKINFO *QBanking::selectBank(QWidget* parent,
                                  const QString &title,
                                  const QString &country,
                                  const QString &bankCode,
                                  const QString &swiftCode,
                                  const QString &bankName,
                                  const QString &location) {
  return QBSelectBank::selectBank(this, parent, title, country,
                                  bankCode, swiftCode,
                                  bankName, location);
}


bool QBanking::isPure7BitAscii(const QString &input){
  unsigned stringlength = input.length();

  for (unsigned k = 0; k < stringlength; ++k) {
    if (input[k].unicode() > 0x7f) {
      DBG_DEBUG(0, "String \"%s\" is not pure-7bit-ascii at character %d.\n",
		input.local8Bit().data(), k);
      return false;
    }
  }
  return true;
}



GWEN_TYPE_UINT32 QBanking::progressStart(const QString &title,
					 const QString &text,
					 GWEN_TYPE_UINT32 total) {
  std::string s1;
  std::string s2;

  s1=QStringToUtf8String(title);
  s2=QStringToUtf8String(text);

  return QBanking::progressStart(s1.c_str(), s2.c_str(), total);
}



int QBanking::progressLog(GWEN_TYPE_UINT32 id,
			  AB_BANKING_LOGLEVEL level,
			  const QString &text) {
  std::string s1;

  s1=QStringToUtf8String(text);
  return QBanking::progressLog(id, level, s1.c_str());
}



GWEN_TYPE_UINT32 QBanking::showBox(GWEN_TYPE_UINT32 flags,
                                   const QString &title,
                                   const QString &text) {
  std::string s1;
  std::string s2;

  s1=QStringToUtf8String(title);
  s2=QStringToUtf8String(text);

  return QBanking::showBox(flags, s1.c_str(), s2.c_str());
}



QBCfgModule *QBanking::_loadCfgModule(const char *modname){
  GWEN_LIBLOADER *ll;
  QBCfgModule *mod;
  QBCFGMODULE_FACTORY_FN fn;
  void *p;
  const char *s;
  GWEN_ERRORCODE err;
  GWEN_BUFFER *mbuf;
  GWEN_PLUGIN *pl;
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_FindPluginManager(QBANKING_PM_CFGMODULE);
  if (!pm) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not find plugin manager for \"%s\"",
              QBANKING_PM_CFGMODULE);
    return 0;
  }
  pl=GWEN_PluginManager_LoadPlugin(pm, modname);
  if (!pl) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not load %s plugin for \"%s\"",
              QBANKING_PM_CFGMODULE, modname);
    return 0;
  }
  ll=GWEN_Plugin_GetLibLoader(pl);

  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(mbuf, QBANKING_PM_CFGMODULE "_");
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(mbuf, tolower(*(s++)));

  /* create name of init function */
  GWEN_Buffer_AppendString(mbuf, "_modfactory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(mbuf), &p);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(mbuf);
    GWEN_Plugin_free(pl);
    return 0;
  }
  GWEN_Buffer_free(mbuf);

  fn=(QBCFGMODULE_FACTORY_FN)p;
  assert(fn);
  mod=fn(this);
  if (!mod) {
    DBG_ERROR(0, "Error in plugin: No module created");
    GWEN_Plugin_free(pl);
    return 0;
  }

  /* store libloader */
  mod->setPlugin(pl);

  return mod;
}



QBCfgModule *QBanking::_findCfgModule(const char *modname) {
  std::list<QBCfgModule*>::iterator it;
  QString qs;

  qs=QString::fromUtf8(modname).lower();
  for (it=_cfgModules.begin(); it!=_cfgModules.end(); it++) {
    if (qs.compare((*it)->getName().lower())==0)
      return *it;
  }
  return 0;
}



QBCfgModule *QBanking::getConfigModule(const char *modname) {
  QBCfgModule *mod;

  if (modname==0)
    return _appCfgModule;
  mod=_findCfgModule(modname);
  if (mod)
    return mod;
  mod=_loadCfgModule(modname);
  if (!mod)
    return 0;
  _cfgModules.push_back(mod);
  return mod;
}



void QBanking::setAppConfigModule(QBCfgModule *mod) {
  _appCfgModule=mod;
}



void QBanking::setupDialog(QWidget *parent) {
  QBCfgTabSettings dlg(this, parent, "QBankingSettings");

  dlg.addUsersPage();
  dlg.addAccountsPage();
  dlg.addBackendsPage();
  if (!dlg.toGui()) {
    DBG_ERROR(0, "Could not init dialog");
    return;
  }
  if (dlg.exec()==QDialog::Accepted) {
    dlg.fromGui();
    flagStaff()->accountsUpdated();
  }
}



int QBanking::print(const QString &docTitle,
                    const QString &docType,
                    const QString &descr,
                    const QString &text) {
  return print(QStringToUtf8String(docTitle).c_str(),
               QStringToUtf8String(docType).c_str(),
               QStringToUtf8String(descr).c_str(),
               QStringToUtf8String(text).c_str());
}
















