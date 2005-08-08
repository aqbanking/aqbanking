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


#include "importer.h"
#include "kbanking.h"

#include <qlistview.h>
#include <qpushbutton.h>
#include <qtextbrowser.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qprogressbar.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <kfiledialog.h>

#include <gwenhywfar/debug.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


Importer::WorkCallback::WorkCallback(Importer *ie,
                                     const char *id)
:WaitCallback(id), _importer(ie){
}



Importer::WorkCallback::~WorkCallback(){
}



WaitCallback *Importer::WorkCallback::instantiate(){
  return new WorkCallback(_importer, getId());
}



GWEN_WAITCALLBACK_RESULT
  Importer::WorkCallback::checkAbort(unsigned int level){
    return _importer->checkAbort(this, level);
  }



void Importer::WorkCallback::log(unsigned int level,
                                 unsigned int loglevel,
                                 const char *s){
  _importer->log(level, loglevel, s);
}





Importer::Importer(KBanking *kb,
                   QWidget* parent,
                   const char* name,
                   bool modal,
                   WFlags fl)
:ImporterUi(parent, name, modal, fl)
,_app(kb)
,_context(0)
,_aborted(false)
,_importerList(0)
,_importer(0)
,_profiles(0)
,_profile(0)
,_dbData(0)
,_waitCallback(0){

  // connect buttons
  QObject::connect((QObject*)selectFileButton, SIGNAL(clicked()),
		   this, SLOT(slotSelectFile()));

  QObject::connect((QObject*)profileEditButton, SIGNAL(clicked()),
                   this, SLOT(slotProfileEdit()));


  QObject::connect((QObject*)profileList, SIGNAL(selectionChanged()),
                   this, SLOT(slotProfileSelected()));

  QObject::connect((QObject*)profileDetailsButton, SIGNAL(clicked()),
                   this, SLOT(slotProfileDetails()));
  QObject::connect((QObject*)profileEditButton, SIGNAL(clicked()),
                   this, SLOT(slotProfileEdit()));

}



Importer::~Importer(){
  AB_ImExporterContext_free(_context);
}


bool Importer::init() {
  GWEN_DB_NODE *db;

  _waitCallback=new WorkCallback(this, KBANKING_IMPORTER_CB_ID);
  if (_waitCallback->registerCallback()) {
    QMessageBox::critical(0,
			  tr("Internal Error"),
			  tr("<qt>"
			     "<p>"
			     "Could not register WorkCallback."
			     "</p>"
			     "<p>"
			     "This is an internal error, please report "
			     "it to <b>martin@libchipcard.de</b>"
			     "</p>"
			     "</qt>"
			    ),
			  tr("Dismiss"), 0, 0, 0);
    delete _waitCallback;
    _waitCallback=0;
    return false;
  }

  db=_app->getAppData();
  assert(db);
  _dbData=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
			   "gui/dlgs/importer");
  return _updateImporterList();
}



void Importer::save() {
  GWEN_DB_NODE *db;

  db=_dbData;
  assert(db);
  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "importers");
  assert(db);

  if (_profile && !_importerName.isEmpty()) {
    const char *s;

    s=GWEN_DB_GetCharValue(_profile, "name", 0, 0);
    if (s) {
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			   _importerName.latin1(),
			   s);
    }
  }

}


bool Importer::fini() {
  if (_importerList)
    GWEN_PluginDescription_List2_freeAll(_importerList);
  _importerList=0;
  _importer=0;
  GWEN_DB_Group_free(_profiles);
  _profiles=0;
  _profile=0;

  if (_waitCallback) {
    _waitCallback->unregisterCallback();
    delete _waitCallback;
    _waitCallback=0;
  }

  return true;
}



void Importer::_wcbLog(GWEN_LOGGER_LEVEL loglevel, const QString &s){
  QCString utfData;
  std::string ls;

  utfData=s.utf8();
  ls=std::string(utfData.data(), utfData.length());
  GWEN_WaitCallback_Log(loglevel, ls.data());
}



bool Importer::_updateImporterList() {
  if (_importerList)
    GWEN_PluginDescription_List2_freeAll(_importerList);
  _importerList=AB_Banking_GetImExporterDescrs(_app->getCInterface());
  if (!_importerList) {
    QMessageBox::critical(0,
			  tr("No Importers"),
			  tr("<qt>"
			     "<p>"
			     "There are currently no importers installed."
			     "</p>"
			     "</qt>"
			    ),
			  tr("Dismiss"), 0, 0, 0);
    return false;
  }
  return true;
}


void Importer::slotSelectFile(){
  QString s=KFileDialog::getOpenFileName(QString::null,
                                         tr("*.*|All files (*.*)"),
                                         this,
                                         tr("Choose a file"));
  if (!s.isEmpty())
    selectFileEdit->setText(s);
}



void Importer::slotFileNameChanged(const QString &s){
  if (s.isEmpty())
    setNextEnabled(selectSourcePage, false);
  else
    setNextEnabled(selectSourcePage, true);
}



void Importer::slotProfileDetails(){
}



void Importer::slotProfileEdit(){
}



void Importer::log(unsigned int level,
                   unsigned int loglevel,
                   const char *text){
  QString tmp;
  QWidget *p;

  if ((int)loglevel<=GWEN_DB_GetIntValue(_dbData, "loglevel", 0, 1)) {
    tmp=_logText;
    tmp+="<tr><td>";
    tmp+=text;
    tmp+="</td></tr>";
    _logText=tmp;
    tmp="<qt><table>"+_logText+"</table></qt>";

    p=currentPage();
    if (p==selectImporterPage) {
      checkLogBrowser->setText(tmp);
      checkLogBrowser->scrollToBottom();
    }
    else if (p==workingPage) {
      readBrowser->setText(tmp);
      readBrowser->scrollToBottom();
    }
    else if (p==importPage) {
      importBrowser->setText(tmp);
      importBrowser->scrollToBottom();
    }
  }
  qApp->processEvents();
}



GWEN_WAITCALLBACK_RESULT Importer::checkAbort(Importer::WorkCallback *wcb,
                                              unsigned int level){
  QWidget *p;
  time_t lastCalled;
  time_t currentTime;

  lastCalled=wcb->lastCalled();
  currentTime=time(0);

  p=currentPage();
  if (p==selectImporterPage) {
  }
  else if (p==workingPage) {
    currentProgressBar->setTotalSteps(wcb->getProgressTotal());
    if (level==0) {
      currentProgressBar->setProgress(wcb->getProgressPos());
    }
  }
  else if (p==importPage) {
  }

  if (currentTime!=lastCalled)
    // only update once per second
    qApp->processEvents();

  if (_aborted)
    return GWEN_WaitCallbackResult_Abort;
  else
    return GWEN_WaitCallbackResult_Continue;
}



void Importer::back(){
  QWidget *p;

  p=currentPage();
  if (p)
    leavePage(p, true);
  QWizard::back();
  p=currentPage();
  if (p)
    enterPage(p, true);
}



void Importer::next(){
  QWidget *p;

  p=currentPage();
  if (p)
    if (!leavePage(p, false))
      return;
  QWizard::next();
  p=currentPage();
  if (p)
    enterPage(p, false);
}



void Importer::reject(){
  QWidget *p;

  DBG_WARN(0, "Undoing all pages");
  while(_pagesDone.size()) {
    bool rv;

    p=_pagesDone.front();
    //DBG_NOTICE(0, "Undoing page %08x", (unsigned int)(p));
    rv=_undoPage(p);
    if (!rv)
      _pagesDone.pop_front();
  } // while

  QWizard::reject();
}



void Importer::accept(){
  save();
  QWizard::accept();
}







bool Importer::_doPage(QWidget *p){
  bool rv = true;

  if (p==selectSourcePage)
    rv=doSelectSourcePage(p);
  else if (p==selectImporterPage)
    rv=doSelectImporterPage(p);
  else if (p==selectProfilePage)
    rv=doSelectProfilePage(p);

  if (rv) {
    //DBG_NOTICE(0, "Pushing page %08x", (unsigned int)p);
    _pagesDone.push_front(p);
  }
  return rv;
}



bool Importer::_undoPage(QWidget *p){
  bool rv = true;

  if (p==selectSourcePage)
    rv=undoSelectSourcePage(p);
  else if (p==selectImporterPage)
    rv=undoSelectImporterPage(p);
  else if (p==selectProfilePage)
    rv=undoSelectProfilePage(p);

  if (rv) {
    DBG_NOTICE(0, "Popping page");
    _pagesDone.pop_front();
  }
  return rv;
}



bool Importer::enterPage(QWidget *p, bool bk){
  if (p==selectProfilePage) {
    QListViewItemIterator it(profileList);
    bool isSelected;

    isSelected=false;
    for (;it.current();++it) {
      if (it.current()->isSelected()) {
        isSelected=true;
        break;
      }
    } // for
    setNextEnabled(selectProfilePage, isSelected);
  }

  if (p==selectImporterPage) {
    if (!bk) {
      enterSelectImporterPage(p);
    }
  }
  else if (p==workingPage) {
    if (!bk) {
      enterWorkingPage(p);
    }
  }
  else if (p==importPage) {
    if (!bk) {
      enterImportingPage(p);
    }
  }

  if (bk)
    return _undoPage(p);
  return true;

}



bool Importer::leavePage(QWidget *p, bool bk){
  if (!bk)
    return _doPage(p);
  return true;
}



bool Importer::initSelectSourcePage(){
  return true;
}



bool Importer::doSelectSourcePage(QWidget *p){
  return true;
}



bool Importer::undoSelectSourcePage(QWidget *p){
  return true;
}



bool Importer::initSelectImporterPage(){
  return true;
}



bool Importer::enterSelectImporterPage(QWidget *p){
  bool res;

  res=_checkFileType(selectFileEdit->text());
  setNextEnabled(selectImporterPage, res);
  return res;
}



void Importer::slotProfileSelected(){
  QListViewItemIterator it(profileList);

  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      setNextEnabled(selectProfilePage, true);
      return;
    }
  } // for
  setNextEnabled(selectProfilePage, false);
}



bool Importer::doSelectImporterPage(QWidget *p){
  assert(_importer);
  assert(!_importerName.isEmpty());

  profileList->clear();
  GWEN_DB_Group_free(_profiles);

  _profiles=AB_Banking_GetImExporterProfiles(_app->getCInterface(),
					     _importerName.latin1());
  if (_profiles) {
    GWEN_DB_NODE *dbT;
    int count;
    GWEN_DB_NODE *db;
    const char *lastProfile;

    lastProfile=0;
    db=_dbData;
    assert(db);
    db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "importers");
    assert(db);
    lastProfile=GWEN_DB_GetCharValue(db, _importerName.latin1(), 0, 0);

    count=0;
    dbT=GWEN_DB_GetFirstGroup(_profiles);
    while(dbT) {
      const char *n;
      const char *d;
      QListViewItem *qv=new QListViewItem(profileList);

      n=GWEN_DB_GetCharValue(dbT, "name", 0, 0);
      d=GWEN_DB_GetCharValue(dbT, "shortDescr", 0, "");
      qv->setText(0, n);
      qv->setText(1, d);
      count++;
      if (lastProfile) {
	if (strcasecmp(lastProfile, n)==0)
	  profileList->setSelected(qv, TRUE);
      }

      dbT=GWEN_DB_GetNextGroup(dbT);
    } // while */
    if (count) {
      return true;
    }
  }

  QMessageBox::critical(0,
                        tr("No Profiles"),
                        tr("<qt>"
                           "<p>"
                           "There are no profiles installed for the "
                           "selected importer."
                           "</p>"
                           "<p>"
                           "</p>"
                           "Please select another one or abort."
                           "</qt>"
                          ),
                        tr("Dismiss"), 0, 0, 0);
  return false;
}



bool Importer::undoSelectImporterPage(QWidget *p){
  profileList->clear();
  GWEN_DB_Group_free(_profiles);
  _profiles=0;
  _profile=0;
  _importer=0;
  return true;
}



bool Importer::initSelectProfilePage(){
  return true;
}



bool Importer::doSelectProfilePage(QWidget *p){
  QListViewItemIterator it(profileList);
  QString qs;

  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      qs=it.current()->text(0);
      break;
    }
  } // for

  if (qs.isEmpty()) {
    QMessageBox::critical(0,
                          tr("Selection Error"),
                          tr("Please select the profile you want to use."),
                          tr("Dismiss"), 0, 0, 0);
    return false;
  }

  _profile=GWEN_DB_GetGroup(_profiles, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                            qs.latin1());
  if (!_profile) {
    QMessageBox::critical(0,
                          tr("Internal Error"),
                          tr("<qt>"
                             "<p>"
                             "Could not select the profile."
                             "</p>"
                             "<p>"
                             "This is an internal error, please report "
                             "it to <b>martin@libchipcard.de</b>"
                             "</p>"
                             "</qt>"
                            ),
                          tr("Dismiss"), 0, 0, 0);
    return false;
  }

  return true;
}



bool Importer::undoSelectProfilePage(QWidget *p){
  return true;
}



void Importer::enterWorkingPage(QWidget *p){
  setNextEnabled(workingPage, false);
  setNextEnabled(workingPage, _readFile(selectFileEdit->text()));
}



void Importer::enterImportingPage(QWidget *p){
  bool res;

  setNextEnabled(importPage, false);
  setBackEnabled(importPage, false);
  res=_importData(_context);
  setFinishEnabled(importPage, true);
  cancelButton()->setEnabled(!res);
}





bool Importer::_importData(AB_IMEXPORTER_CONTEXT *ctx) {
  QString qs;
  bool res;
  int cnt;
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  _logText="";
  ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  cnt=0;
  while(ai) {
    cnt++;
    ai=AB_ImExporterContext_GetNextAccountInfo(ctx);
  }
  qs=tr("Letting application import data");
  log(0, AB_Banking_LogLevelNotice, qs.utf8().data());

  res=_app->importContext(ctx);
  if (res) {
    qs=tr("Importing files completed.");
    log(0, AB_Banking_LogLevelNotice, qs.utf8().data());
  }
  else {
    qs=tr("Error importing files.");
    log(0, AB_Banking_LogLevelError, qs.utf8().data());
  }

  return res;
}



bool Importer::_checkFileType(const QString &fname){
  QString qs;
  GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;
  GWEN_PLUGIN_DESCRIPTION *pd;
  AB_IMEXPORTER *importer;

  importer=0;
  _logText="";
  setNextEnabled(selectImporterPage, false);

  GWEN_WaitCallback_Enter(KBANKING_IMPORTER_CB_ID);
  assert(_importerList);
  checkFileNameLabel->setText(fname);
  checkFileTypeLabel->setText(tr("-- checking --"));

  _wcbLog(GWEN_LoggerLevelNotice, tr("Checking for file type ..."));

  // check for type
  it=GWEN_PluginDescription_List2_First(_importerList);
  assert(it);
  pd=GWEN_PluginDescription_List2Iterator_Data(it);
  assert(pd);

  while(pd) {
    const char *pdtype;

    pdtype=GWEN_PluginDescription_GetType(pd);
    if (pdtype) {
      if (strcasecmp(pdtype, "imexporter")==0) {
	_wcbLog(GWEN_LoggerLevelNotice,
		QString(tr("Trying type \"%1\""))
                .arg(GWEN_PluginDescription_GetName(pd)));
	importer=AB_Banking_GetImExporter(_app->getCInterface(),
					  GWEN_PluginDescription_GetName(pd));
	if (!importer) {
	  _wcbLog(GWEN_LoggerLevelNotice,
		  QString(tr("Type \"%1\" is not available"))
		  .arg(GWEN_PluginDescription_GetName(pd)));
	}
	else {
	  int rv;
    
	  // let the importer check the file
	  rv=AB_ImExporter_CheckFile(importer, fname.latin1());
	  if (rv==0) {
	    _wcbLog(GWEN_LoggerLevelNotice,
		    QString(tr("File type is \"%1\""))
		    .arg(GWEN_PluginDescription_GetName(pd)));
	    break;
	  }
	  else {
	    _wcbLog(GWEN_LoggerLevelDebug,
		    QString(tr("File is not of type \"%1\""))
		    .arg(GWEN_PluginDescription_GetName(pd)));
	  }
	}
      }
    }
    pd=GWEN_PluginDescription_List2Iterator_Next(it);
  } // while
  GWEN_PluginDescription_List2Iterator_free(it);
  if (!pd) {
    QMessageBox::critical(0,
			  tr("Error"),
			  tr("<qt>"
			     "<p>"
                             "Could not determine the file type."
			     "</p>"
			     "<p>"
			     "This file can not be imported."
			     "</p>"
			     "</qt>"
			    ),
			  tr("Dismiss"), 0, 0, 0);
    GWEN_WaitCallback_Leave();
    return false;
  }

  GWEN_WaitCallback_Leave();
  checkFileTypeLabel->setText(GWEN_PluginDescription_GetName(pd));
  _importerName=GWEN_PluginDescription_GetName(pd);
  _importer=importer;
  return true;
}



bool Importer::_readFile(const QString &fname){
  QString qs;
  int rv;

  _logText="";
  setNextEnabled(workingPage, false);

  AB_ImExporterContext_free(_context);
  _context=AB_ImExporterContext_new();

  GWEN_WaitCallback_Enter(KBANKING_IMPORTER_CB_ID);
  _wcbLog(GWEN_LoggerLevelNotice, "Starting...");

  QFile f(fname);

  if (!f.exists()) {
    DBG_NOTICE(0, "File \"%s\" does not exist",
	       fname.latin1());
    qs=QString(tr("File \"%s\" does not exist"))
      .arg(fname);
    GWEN_WaitCallback_Leave();
    return false;
  }
  else {
    int fd;

    qs=QString(tr("Importing file \"%1\""))
      .arg(fname);
    _wcbLog(GWEN_LoggerLevelNotice, qs);
    DBG_INFO(0, "Importing file \"%s\"", fname.latin1());
    GWEN_WaitCallback_SetProgressTotal(0);
    GWEN_WaitCallback_SetProgressPos(0);
    currentProgressBar->setTotalSteps(0);
    currentProgressBar->setProgress(0);
    fd=open(fname.latin1(), O_RDONLY);
    if (fd==-1) {
      qs=QString(tr("Could not open file \"%1\": %2"))
	.arg(fname)
	.arg(strerror(errno));
      _wcbLog(GWEN_LoggerLevelNotice, qs);
      GWEN_WaitCallback_Leave();
      return false;
    }
    else {
      GWEN_BUFFEREDIO *bio;

      bio=GWEN_BufferedIO_File_new(fd);
      GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);
      rv=AB_ImExporter_Import(_importer,
			      _context,
			      bio,
			      _profile);
      if (rv) {
	GWEN_BufferedIO_Abandon(bio);
      }
      else
	GWEN_BufferedIO_Close(bio);
      GWEN_BufferedIO_free(bio);
      if (rv) {
	DBG_NOTICE(0, "Error importing file \"%s\"",
		   fname.latin1());
	_wcbLog(GWEN_LoggerLevelNotice,
		QString(tr("Error importing file \"%1\"")).arg(fname));
	QMessageBox::critical(0,
			      tr("Error"),
			      tr("<qt>"
				 "<p>"
				 "An error occurred while importing a "
				 "file."
				 "</>"
				 "<p>"
				 "Please see the log for a reason"
				 "</p>"
				 "</qt>"
				),
			      tr("Dismiss"), 0, 0, 0);
	GWEN_WaitCallback_Leave();
	return false;
      }
      DBG_NOTICE(0, "File \"%s\" imported",
		 fname.latin1());
    } // if open succeeded
  } // if file exists

  DBG_NOTICE(0, "Reading files completed.");
  _wcbLog(GWEN_LoggerLevelNotice, tr("Reading files completed."));
  GWEN_WaitCallback_Leave();
  DBG_NOTICE(0, "Returning to caller.");
  return true;
}






