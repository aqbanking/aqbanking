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


// QBanking includes
#include "qbimporter.h"
#include "qbanking.h"
#include "qbselectfromlist.h"

// Gwenhywfar includes
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>

// QT includes
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

// C includes
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef WIN32
# include <io.h> // for open()
# define strcasecmp stricmp
#endif


QBImporter::QBImporter(QBanking *kb,
                       QWidget* parent,
                       const char* name,
                       bool modal)
:QBImporterUi(parent, name)
,_app(kb)
,_flags(QBANKING_IMPORTER_FLAGS_ASK_ALL_DUPES|
	QBANKING_IMPORTER_FLAGS_FUZZY)
,_context(0)
,_aborted(false)
,_importerList(0)
,_importer(0)
,_profiles(0)
,_profile(0)
,_dbData(0)
,_logLevel(GWEN_LoggerLevelInfo){
  setModal(modal);

  setBackEnabled(finishPage, false);
  setFinishEnabled(finishPage, true);

  // connect buttons
  QObject::connect(selectFileButton, SIGNAL(clicked()),
		   this, SLOT(slotSelectFile()));

  QObject::connect(profileEditButton, SIGNAL(clicked()),
                   this, SLOT(slotProfileEdit()));


  QObject::connect(profileList, SIGNAL(selectionChanged()),
                   this, SLOT(slotProfileSelected()));

  QObject::connect(profileDetailsButton, SIGNAL(clicked()),
                   this, SLOT(slotProfileDetails()));
  QObject::connect(profileEditButton, SIGNAL(clicked()),
                   this, SLOT(slotProfileEdit()));
}



QBImporter::QBImporter(QBanking *kb,
		       GWEN_TYPE_UINT32 flags,
                       QWidget* parent,
                       const char* name,
                       bool modal)
:QBImporterUi(parent, name)
,_app(kb)
,_flags(flags)
,_context(0)
,_aborted(false)
,_importerList(0)
,_importer(0)
,_profiles(0)
,_profile(0)
,_dbData(0)
,_logLevel(GWEN_LoggerLevelInfo){
  setModal(modal);

  setBackEnabled(finishPage, false);
  setFinishEnabled(finishPage, true);

  // connect buttons
  QObject::connect(selectFileButton, SIGNAL(clicked()),
		   this, SLOT(slotSelectFile()));

  QObject::connect(profileEditButton, SIGNAL(clicked()),
                   this, SLOT(slotProfileEdit()));


  QObject::connect(profileList, SIGNAL(selectionChanged()),
                   this, SLOT(slotProfileSelected()));

  QObject::connect(profileDetailsButton, SIGNAL(clicked()),
                   this, SLOT(slotProfileDetails()));
  QObject::connect(profileEditButton, SIGNAL(clicked()),
                   this, SLOT(slotProfileEdit()));
}



QBImporter::~QBImporter(){
  AB_ImExporterContext_free(_context);
}


bool QBImporter::init() {
  GWEN_DB_NODE *db;

  db=_app->getSharedData("qbanking");
  assert(db);
  _dbData=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
			   "gui/dlgs/importer");
  return _updateImporterList();
}



void QBImporter::save() {
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
			   _importerName.utf8(),
			   s);
    }
  }

}


bool QBImporter::fini() {
  if (_importerList)
    GWEN_PluginDescription_List2_freeAll(_importerList);
  _importerList=0;
  _importer=0;
  GWEN_DB_Group_free(_profiles);
  _profiles=0;
  _profile=0;

  return true;
}



bool QBImporter::_updateImporterList() {
  if (_importerList)
    GWEN_PluginDescription_List2_freeAll(_importerList);
  _importerList=AB_Banking_GetImExporterDescrs(_app->getCInterface());
  if (!_importerList) {
    QMessageBox::critical(this,
			  tr("No Importers"),
			  tr("<qt>"
			     "<p>"
			     "There are currently no importers installed."
			     "</p>"
			     "</qt>"
			    ),
			  QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }
  return true;
}


void QBImporter::slotSelectFile(){
  QString s = QFileDialog::getOpenFileName(QString::null,
					   QString::null,
					   this,
					   "OpenFile",
					   tr("Choose a file to import"));
  if (!s.isEmpty())
    selectFileEdit->setText(s);
}



void QBImporter::slotFileNameChanged(const QString &s){
  if (s.isEmpty())
    setNextEnabled(selectSourcePage, false);
  else
    setNextEnabled(selectSourcePage, true);
}



void QBImporter::slotProfileDetails(){
}



void QBImporter::slotProfileEdit(){
}



void QBImporter::back(){
  QWidget *p;

  p=currentPage();
  if (p)
    leavePage(p, true);
  QWizard::back();
  p=currentPage();
  if (p)
    enterPage(p, true);
}



void QBImporter::next(){
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



void QBImporter::reject(){
  QWidget *p;

  DBG_WARN(0, "Undoing all pages");
  while(_pagesDone.size()) {
    bool rv;

    p=_pagesDone.front();
    DBG_NOTICE(0, "Undoing page %p", p);
    rv=_undoPage(p);
    if (!rv)
      _pagesDone.pop_front();
  } // while

  QWizard::reject();
}



void QBImporter::accept(){
  save();
  QWizard::accept();
}







bool QBImporter::_doPage(QWidget *p){
  bool rv = true;

  if (p==selectSourcePage)
    rv=doSelectSourcePage(p);
  else if (p==selectImporterPage)
    rv=doSelectImporterPage(p);
  else if (p==selectProfilePage)
    rv=doSelectProfilePage(p);
  else if (p==workingPage)
    rv=doWorkingPage(p);
  else if (p==importPage)
    rv=doImportPage(p);

  if (rv) {
    DBG_DEBUG(0, "Pushing page %p", p);
    _pagesDone.push_front(p);
  }
  return rv;
}



bool QBImporter::_undoPage(QWidget *p){
  bool rv = true;

  if (p==selectSourcePage)
    rv=undoSelectSourcePage(p);
  else if (p==selectImporterPage)
    rv=undoSelectImporterPage(p);
  else if (p==selectProfilePage)
    rv=undoSelectProfilePage(p);
  else if (p==workingPage)
    rv=undoWorkingPage(p);

  if (rv) {
    DBG_DEBUG(0, "Popping page");
    _pagesDone.pop_front();
  }
  return rv;
}



bool QBImporter::enterPage(QWidget *p, bool bk){
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

  if (bk)
    return _undoPage(p);
  return true;

}



bool QBImporter::leavePage(QWidget *p, bool bk){
  if (!bk)
    return _doPage(p);
  return true;
}



bool QBImporter::initSelectSourcePage(){
  return true;
}



bool QBImporter::doSelectSourcePage(QWidget *p){
  bool res;

  res=_checkFileType(selectFileEdit->text());
  return res;
}



bool QBImporter::undoSelectSourcePage(QWidget *p){
  return true;
}



bool QBImporter::initSelectImporterPage(){
  return true;
}




void QBImporter::slotProfileSelected(){
  QListViewItemIterator it(profileList);

  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      setNextEnabled(selectProfilePage, true);
      return;
    }
  } // for
  setNextEnabled(selectProfilePage, false);
}



bool QBImporter::doSelectImporterPage(QWidget *p){
  assert(_importer);
  assert(!_importerName.isEmpty());

  profileList->clear();
  GWEN_DB_Group_free(_profiles);

  _profiles=AB_Banking_GetImExporterProfiles(_app->getCInterface(),
					     _importerName.utf8());
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
    lastProfile=GWEN_DB_GetCharValue(db, _importerName.utf8(), 0, 0);

    count=0;
    dbT=GWEN_DB_GetFirstGroup(_profiles);
    while(dbT) {
      const char *n;
      const char *d;
      QListViewItem *qv=new QListViewItem(profileList);

      n=GWEN_DB_GetCharValue(dbT, "name", 0, 0);
      d=GWEN_DB_GetCharValue(dbT, "shortDescr", 0, "");
      qv->setText(0, QString::fromUtf8(n));
      qv->setText(1, QString::fromUtf8(d));
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

  QMessageBox::critical(this,
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
			QMessageBox::Retry,QMessageBox::NoButton);
  return false;
}



bool QBImporter::undoSelectImporterPage(QWidget *p){
  profileList->clear();
  GWEN_DB_Group_free(_profiles);
  _profiles=0;
  _profile=0;
  _importer=0;
  return true;
}



bool QBImporter::initSelectProfilePage(){
  return true;
}



bool QBImporter::doSelectProfilePage(QWidget *p){
  QListViewItemIterator it(profileList);
  QString qs;

  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      qs=it.current()->text(0);
      break;
    }
  } // for

  if (qs.isEmpty()) {
    QMessageBox::critical(this,
                          tr("Selection Error"),
                          tr("Please select the profile you want to use."),
			  QMessageBox::Retry,QMessageBox::NoButton);
    return false;
  }

  _profile=GWEN_DB_GetGroup(_profiles, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                            qs.utf8());
  if (!_profile) {
    QMessageBox::critical(this,
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
			  QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }

  return true;
}



bool QBImporter::undoSelectProfilePage(QWidget *p){
  return true;
}



bool QBImporter::doWorkingPage(QWidget *p){
  bool res;

  res=_readFile(selectFileEdit->text());
  return res;
}



bool QBImporter::undoWorkingPage(QWidget *p){
  AB_ImExporterContext_free(_context);
  _context=AB_ImExporterContext_new();
  return true;
}



bool QBImporter::doImportPage(QWidget *p){
  bool res;

  res=_importData(_context);
  if (!res)
    reject();
  return res;
}




bool QBImporter::_importData(AB_IMEXPORTER_CONTEXT *ctx) {
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
  GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                        qs.utf8());

  res=_app->importContext(ctx, _flags);
  if (res) {
    DBG_INFO(0, "Importing files completed.");
  }
  else {
    QMessageBox::critical(this,
                          tr("Error"),
                          tr("Error importing data into the application."),
			  QMessageBox::Ok,QMessageBox::NoButton);
    return false;

    qs=tr("Error importing files.");
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                          qs.utf8());
  }

  return res;
}



bool QBImporter::_checkFileType(const QString &fname){
  QString qs;
  GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;
  GWEN_PLUGIN_DESCRIPTION *pd;
  AB_IMEXPORTER *importer;
  std::list<std::string> possibles;

  importer=0;
  _logText="";

  GWEN_WaitCallback_Enter(GWEN_WAITCALLBACK_ID_SIMPLE_PROGRESS);
  assert(_importerList);
  checkFileNameLabel->setText(fname);
  checkFileTypeLabel->setText(tr("-- checking --"));

  qs=tr("Checking for file type...");
  GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                        qs.utf8());

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
        qs=QWidget::tr("Trying type \"%1\"")
          .arg(QString::fromUtf8(GWEN_PluginDescription_GetName(pd)));
        GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                              qs.utf8());
        importer=AB_Banking_GetImExporter(_app->getCInterface(),
					  GWEN_PluginDescription_GetName(pd));
        if (!importer) {
          qs=QWidget::tr("Type \"%1\" is not available")
            .arg(QString::fromUtf8(GWEN_PluginDescription_GetName(pd)));
          GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                                qs.utf8());
	}
	else {
	  int rv;
    
	  // let the importer check the file
	  rv=AB_ImExporter_CheckFile(importer, fname.utf8());
          if (rv==0) {
            qs=QWidget::tr("File type is \"%1\"")
              .arg(QString::fromUtf8(GWEN_PluginDescription_GetName(pd)));
            GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                                  qs.utf8());
            break;
          }
          else if (rv==AB_ERROR_UNKNOWN) {
            qs=QWidget::tr("File type might be \"%1\", checking further")
              .arg(QString::fromUtf8(GWEN_PluginDescription_GetName(pd)));
            GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                                  qs.utf8());
            possibles.push_back(GWEN_PluginDescription_GetName(pd));
          }
          else {
            qs=QWidget::tr("File is not of type \"%1\"")
              .arg(QString::fromUtf8(GWEN_PluginDescription_GetName(pd)));
            GWEN_WaitCallback_Log(GWEN_LoggerLevelDebug,
                                  qs.utf8());
	  }
	}
      }
    }
    pd=GWEN_PluginDescription_List2Iterator_Next(it);
  } // while
  GWEN_PluginDescription_List2Iterator_free(it);
  if (!pd) {
    if (possibles.empty()) {
      QMessageBox::critical(this,
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
			    QMessageBox::Ok,QMessageBox::NoButton);
      GWEN_WaitCallback_Leave();
      return false;
    }
    // there are candidates, ask user to choose one of them
    if (possibles.size()==1) {
      std::string s;

      // only one candidate, no need to ask the user here
      s=possibles.front();
      _importer=AB_Banking_GetImExporter(_app->getCInterface(),
                                         s.c_str());
      assert(_importer);
      checkFileTypeLabel->setText(QString::fromUtf8(s.c_str()));
      _importerName=s.c_str();
      GWEN_WaitCallback_Leave();
      return true;
    } // if only one string
    else {
      QStringList sl;
      QBSelectFromList sfl(_app,
                           tr("Select Importer"),
                           tr("<qt>"
                              "<p>"
                              "The correct importer could "
                              "not be autodetected."
                              "</p>"
                              "<p>"
                              "However, the following "
                              "importers claim to support "
                              "the given selected file."
                              "</p>"
                              "<p>"
                              "Please choose the correct "
                              "one."
                              "</p>"
                              "</qt>"),
                           tr("Possible Importers"),
                           1, 1,
                           this,
                           "ChooseImporter",
                           TRUE);
      it=GWEN_PluginDescription_List2_First(_importerList);
      assert(it);
      pd=GWEN_PluginDescription_List2Iterator_Data(it);
      assert(pd);

      // fill list
      while(pd) {
        const char *pdtype;
    
        pdtype=GWEN_PluginDescription_GetType(pd);
        if (pdtype) {
          if (strcasecmp(pdtype, "imexporter")==0) {
            std::list<std::string>::iterator sit;

            for (sit=possibles.begin();
                 sit!=possibles.end();
                 sit++) {
              if (strcasecmp(GWEN_PluginDescription_GetName(pd),
                             (*sit).c_str())==0) {
                sfl.addEntry(QString::fromUtf8(GWEN_PluginDescription_GetName(pd)),
                             QString::fromUtf8(GWEN_PluginDescription_GetShortDescr(pd)));
              }
            } // for stringlist
          } // if imexporter
        }
        pd=GWEN_PluginDescription_List2Iterator_Next(it);
      } // while
      GWEN_PluginDescription_List2Iterator_free(it);
      sfl.init();
      if (sfl.exec()!=QDialog::Accepted) {
        GWEN_WaitCallback_Leave();
        return false;
      }
      sl=sfl.selectedEntries();
      if (sl.isEmpty()) {
        DBG_ERROR(0, "No entry selected");
        GWEN_WaitCallback_Leave();
        return false;
      }
      else {
        QString s;

        // only one candidate, no need to ask the user here
        s=sl.front();
        importer=AB_Banking_GetImExporter(_app->getCInterface(),
                                          s.utf8());
        assert(importer);
        checkFileTypeLabel->setText(s);
        _importerName=s;
        _importer=AB_Banking_GetImExporter(_app->getCInterface(),
                                           s.utf8());
        assert(_importer);
        GWEN_WaitCallback_Leave();
        return true;
      }
    } // if more than one candidates
  } // if no direct hit

  GWEN_WaitCallback_Leave();
  checkFileTypeLabel->setText(QString::fromUtf8(GWEN_PluginDescription_GetName(pd)));
  _importerName=GWEN_PluginDescription_GetName(pd);
  _importer=importer;
  return true;
}



bool QBImporter::_readFile(const QString &fname){
  QString qs;
  int rv;

  _logText="";

  AB_ImExporterContext_free(_context);
  _context=AB_ImExporterContext_new();

  QFile f(fname);

  if (!f.exists()) {
    DBG_NOTICE(0, "File \"%s\" does not exist",
	       fname.local8Bit().data());
    qs=QWidget::tr("File \"%1\" does not exist")
      .arg(fname);
    QMessageBox::critical(this,
			  tr("File not found"),
			  qs,
			  QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }
  else {
    int fd;

    DBG_INFO(0, "Importing file \"%s\"", fname.local8Bit().data());
    // Note: This is not a cast but rather a call of the
    // conversion operator "QCString::operator const char * ()
    // const" that will correctly convert the QCString to the
    // const char*
    fd=open(fname.local8Bit().data(), O_RDONLY);
    if (fd==-1) {
      qs=QWidget::tr("Could not open file \"%1\": %2")
	.arg(fname)
	.arg(QString(strerror(errno)));
      QMessageBox::critical(this,
			    tr("Error"),
			    qs,
			    QMessageBox::Ok,QMessageBox::NoButton);
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
		   fname.local8Bit().data());
        qs=QWidget::tr("Error importing file \"%1\"").arg(fname);
	QMessageBox::critical(this,
			      tr("Error"),
			      qs,
			      QMessageBox::Ok,QMessageBox::NoButton);
	return false;
      }
      DBG_NOTICE(0, "File \"%s\" imported",
		 fname.local8Bit().data());
    } // if open succeeded
  } // if file exists

  DBG_NOTICE(0, "Reading files completed.");
  qs=tr("Reading files completed.");
  DBG_DEBUG(0, "Returning to caller.");
  return true;
}



bool QBImporter::import(QBanking *qb,
			GWEN_TYPE_UINT32 flags,
			QWidget* parent) {
  QBImporter w(qb, flags, parent, "Importer", true);
  bool res;

  if (!w.init()) {
    return false;
  }
  res=(w.exec()==QDialog::Accepted);
  if (res) {
  }
  w.fini();

  return res;
}



void QBImporter::help() {
  QWidget *w;
  const char *s = "none";

  w=currentPage();
  if (w) {
    if (w==startPage)
      s="startPage";
    else if (w==selectSourcePage)
      s="selectSourcePage";
    else if (w==selectImporterPage)
      s="selectImporterPage";
    else if (w==selectProfilePage)
      s="selectProfilePage";
    else if (w==workingPage)
      s="workingPage";
    else if (w==importPage)
      s="importPage";
    else if (w==finishPage)
      s="finishPage";
  }

  _app->invokeHelp("QBImporter", s);
}


#include "qbimporter.moc"


