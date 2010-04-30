/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "logmanager.h"
#include "loganalyzer.h"

#include <aqhbci/msgengine.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/path.h>
#include <gwenhywfar/text.h>

#ifndef WIN32
# include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <qmessagebox.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qtextbrowser.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qdir.h>

#include <qbanking/qbanking.h>

#ifdef WIN32
# define AH_PATH_SEP "\\"
#else
# define AH_PATH_SEP "/"
#endif

#ifdef WIN32
# include <io.h>
# include <direct.h>
//# define S_ISDIR(x) ((x) & _S_IFDIR) -- mingw-3.4.4 now has that defined.
#endif

LogManager::LogManager(const char *baseDir,
                       QWidget* parent,
                       const char* name,
                       bool modal,
                       WFlags fl)
:LogManagerUi(parent, name, modal, fl)
, _trustLevel(0) {
  GWEN_XMLNODE *defs;
  list<string>::iterator sit;
  const char *xmlfilename = AQHBCI_DATAFOLDER 
    AH_PATH_SEP "hbci.xml";

  if (baseDir)
    _baseDir=baseDir;
  _msgEngine=AH_MsgEngine_new();

  defs=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"defs");
  DBG_DEBUG(0, "Reading XML file");

  if (GWEN_XML_ReadFile(defs, xmlfilename,
                        GWEN_XML_FLAGS_DEFAULT)){
    DBG_ERROR(0,"Error parsing XML file");
    QMessageBox::critical(this,
                          tr("Could not parse HBCI XML file"),
                          tr("<qt>"
                             "The HBCI XML file \"%1\" could not be parsed."
                             "</qt>"
                            ).arg(QString::fromLocal8Bit(xmlfilename)),
                          QMessageBox::Ok,QMessageBox::NoButton);
    GWEN_XMLNode_free(defs);
  }
  else {
    GWEN_MsgEngine_AddDefinitions(_msgEngine, defs);
    GWEN_XMLNode_free(defs);
  }

  _scanBanks();
  for (sit=_banks.begin(); sit!=_banks.end(); sit++)
    bankSelector->insertItem(QString::fromUtf8((*sit).c_str()));

  QObject::connect((bankSelector),
                   SIGNAL(activated(const QString&)),
                   this,
                   SLOT(bankActivated(const QString&)));
  QObject::connect((trustSelector),
                   SIGNAL(activated(int)),
                   this,
                   SLOT(trustActivated(int)));
  bankSelector->setCurrentItem(0);
  bankActivated(bankSelector->currentText());

  QObject::connect((fileList),
                   SIGNAL(selectionChanged(QListViewItem*)),
                   this,
                   SLOT(fileSelected(QListViewItem*)));
  QObject::connect((saveButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(saveFile()));
}



LogManager::~LogManager(){
}



int LogManager::_scanBanks() {
  GWEN_DIRECTORY *dlogs;
  string dname;

  dname=_baseDir;
  dname+="/backends/aqhbci/data/banks/de/";
  if (!dname.empty()) {
    dlogs=GWEN_Directory_new();
    if (!GWEN_Directory_Open(dlogs, dname.c_str())) {
      char nbuffer[256];

      while(!GWEN_Directory_Read(dlogs,
                                 nbuffer,
                                 sizeof(nbuffer))) {
        if (strcmp(nbuffer, "..")!=0 &&
            strcmp(nbuffer, ".")!=0) {
          string fname;
          struct stat st;

          fname=dname+"/"+nbuffer;
          if (stat(fname.c_str(), &st)) {
            DBG_ERROR(0, "Could not stat entry \"%s\"", fname.c_str());
          }
          else {
	    if (S_ISDIR(st.st_mode)) {
	      DBG_NOTICE(0, "Added folder \"%s\"", fname.c_str());
              _banks.push_back(nbuffer);
            }
	  }
        }
      } // while read
      if (GWEN_Directory_Close(dlogs)) {
        DBG_ERROR(0, "Error closing folder \"%s\"", dname.c_str());
        GWEN_Directory_free(dlogs);
        return -1;
      }
    } // if open succeeds
    GWEN_Directory_free(dlogs);
  } // if !empty
  return 0;
}



int LogManager::_scanBank(const string &bankCode) {
  GWEN_DIRECTORY *dlogs;
  string dname;

  dname=_baseDir;
  dname+="/backends/aqhbci/data/banks/de/";
  dname+=bankCode;
  dname+="/logs";
  DBG_NOTICE(0, "Scanning folder \"%s\"", dname.c_str());
  if (!dname.empty()) {
    dlogs=GWEN_Directory_new();
    if (!GWEN_Directory_Open(dlogs, dname.c_str())) {
      char nbuffer[256];

      while(!GWEN_Directory_Read(dlogs,
                                 nbuffer,
                                 sizeof(nbuffer))) {
        int i;

        i=strlen(nbuffer);
        if (i>4) {
          if (strcmp(nbuffer+i-4, ".log")==0) {
            string fname;

            fname=dname+"/"+nbuffer;
            DBG_NOTICE(0, "Added file \"%s\"", fname.c_str());
            _logFiles.push_back(nbuffer);
          }
        }
      } // while read
      if (GWEN_Directory_Close(dlogs)) {
        DBG_ERROR(0, "Error closing folder \"%s\"", dname.c_str());
        GWEN_Directory_free(dlogs);
        return -1;
      }
    } // if open succeeds
    GWEN_Directory_free(dlogs);
  } // if !empty
  return 0;
}



string LogManager::_anonymize(const string &bankCode,
                              const string &fname,
                              int trustLevel){
  //list<string>::const_iterator it;
  Pointer<LogAnalyzer::LogFile> logfile;
  list<Pointer<LogAnalyzer::LogFile::LogMessage> > lmsgs;
  list<Pointer<LogAnalyzer::LogFile::LogMessage> >::iterator lmit;
  GWEN_MSGENGINE_TRUSTEDDATA *trustedData;
  GWEN_MSGENGINE_TRUSTEDDATA *ntd;
  GWEN_DB_NODE *allgr;
  string lfname;
  string result;

  lfname=_baseDir;
  lfname+="/backends/aqhbci/data/banks/de/";
  lfname+=bankCode;
  lfname+="/logs/";
  lfname+=fname;

  try {
    logfile=new LogAnalyzer::LogFile(lfname);
  }
  catch (Error xerr) {
    DBG_ERROR(0, "LogAnalyzer::LogFile error: %s", xerr.errorString().c_str());
    return "";
  }

  allgr=GWEN_DB_Group_new("messages");
  lmsgs=logfile.ref().logMessages();
  for (lmit=lmsgs.begin(); lmit!=lmsgs.end(); lmit++) {
    GWEN_DB_NODE *gr;
    GWEN_DB_NODE *hd;
    GWEN_DB_NODE *repl;
    GWEN_BUFFER *mbuf;
    int rv;
    string lstr;
    string mode;
    string tstr;
    int pversion;

    DBG_NOTICE(0, "Handling log message");
    gr=GWEN_DB_Group_new("message");
    lstr=(*lmit).ref().message();
    hd=(*lmit).ref().header();
    mode=GWEN_DB_GetCharValue(hd, "mode",0, "RDH");
    if (strcasecmp(mode.c_str(), "PINTAN")==0)
      pversion=220;
    else
      pversion=210;
    pversion=GWEN_DB_GetIntValue(hd, "pversion", 0, pversion);
    GWEN_MsgEngine_SetMode(_msgEngine, mode.c_str());
    GWEN_MsgEngine_SetProtocolVersion(_msgEngine, pversion);
    mbuf=GWEN_Buffer_new(0, lstr.length(), 0, 1);
    GWEN_Buffer_AppendBytes(mbuf,
                            lstr.data(),
                            lstr.length());
    GWEN_Buffer_Rewind(mbuf);

    DBG_INFO(0, "Reading message");
    rv=GWEN_MsgEngine_ReadMessage(_msgEngine, "SEG", mbuf, gr,
                                  GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO);
    if (rv) {
      DBG_ERROR(0, "Could not read message");
      GWEN_Text_DumpString(lstr.data(), lstr.length(), stderr, 2);
      GWEN_Buffer_free(mbuf);
      return QBanking::QStringToUtf8String(QWidget::tr("Error: Could not read logfile."));
    }
    GWEN_Buffer_free(mbuf);

    // work on trust data
    DBG_NOTICE(0, "Handling trustinfo");
    trustedData=GWEN_MsgEngine_TakeTrustInfo(_msgEngine);
    if (trustedData) {
      if (GWEN_MsgEngine_TrustedData_CreateReplacements(trustedData)) {
        DBG_ERROR(0, "Could not anonymize log (createReplacements)");
        GWEN_MsgEngine_TrustedData_free(trustedData);
        return QBanking::QStringToUtf8String(QWidget::tr("Error: Could not anonymize logfile."));
      }
    }

    // anonymize
    DBG_NOTICE(0, "Anonymizing data");
    ntd=trustedData;
    repl=GWEN_DB_GetGroup(hd, GWEN_DB_FLAGS_DEFAULT, "replacements");
    assert(repl);
    while(ntd) {
      if (GWEN_MsgEngine_TrustedData_GetTrustLevel(ntd)>trustLevel) {
        int pos;
        unsigned int size;
        char rbuffer[2];
        const char *rpstr;

        rpstr=GWEN_MsgEngine_TrustedData_GetReplacement(ntd);
        assert(rpstr);
        assert(*rpstr);
        size=strlen(rpstr);
        if (size==1) {
          rbuffer[0]=rpstr[0];
          rbuffer[1]=0;
        }
        else {
          rbuffer[0]=rpstr[0];
          rbuffer[1]=rpstr[1];
          rbuffer[2]=0;
        }
        GWEN_DB_SetCharValue(repl,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_VAR,
                             rbuffer,
                             GWEN_MsgEngine_TrustedData_GetDescription(ntd));
        size=GWEN_MsgEngine_TrustedData_GetSize(ntd);
        pos=GWEN_MsgEngine_TrustedData_GetFirstPos(ntd);
        while(pos>=0) {
          DBG_INFO(0, "Replacing %d bytes at %d", size, pos);
          lstr.replace(pos, size,
                       GWEN_MsgEngine_TrustedData_GetReplacement(ntd));
          pos=GWEN_MsgEngine_TrustedData_GetNextPos(ntd);
        } // while pos
      }
      ntd=GWEN_MsgEngine_TrustedData_GetNext(ntd);
    } // while ntd

    // log anonymized message
    LogAnalyzer::LogFile::LogMessage anonMsg(GWEN_DB_Group_dup(hd),
                                             lstr);
    tstr=anonMsg.toString();
    if (tstr.empty()) {
      QMessageBox::critical(this,
                            tr("Error"),
                            tr("<qt>"
                               "<p>"
                               "An error occurred while anonymizing the "
                               "logfile \"%1\"."
                               "</p>"
                               "<p>"
                               "This is very unlikely, in most cases this "
                               "is a setup error. Please make sure that "
                               "AqHBCI is properly installed."
                               "</p>"
                               "</qt>"
                              ).arg(QString::fromLocal8Bit(lfname.c_str())),
                            QMessageBox::Ok,QMessageBox::NoButton);
      return QBanking::QStringToUtf8String(QWidget::tr("Error: Could not anonymize logfile."));
    }
    result+=tstr;
  }
  return result;
}



void LogManager::bankActivated(const QString &qs){
  list<string>::iterator sit;
  QString nqs;
  std::string s;

  fileList->clear();
  _logFiles.clear();
  if (!qs.isEmpty())
    s=QBanking::QStringToUtf8String(qs);
  _scanBank(s);
  for (sit=_logFiles.begin(); sit!=_logFiles.end(); sit++) {
    QListViewItem *qv;

    qv=new QListViewItem(fileList, QString::fromUtf8((*sit).c_str()));
  }

}



void LogManager::trustActivated(int idx){
  if (_trustLevel!=idx) {
    _trustLevel=idx;
    if (!_currentFile.isEmpty()) {
      string s;

      fileView->setText(QString::null);
      s=_anonymize(bankSelector->currentText().ascii(),
                   _currentFile.ascii(),
                   _trustLevel);
      _currentLog=s;
      fileView->setText(QString::fromUtf8(_dump(s).c_str()));
    }
  }
}



string LogManager::_dump(const string &s) {
  string result;
  const unsigned char *p;
  unsigned int i;

  p=(const unsigned char*)s.data();
  for (i=0; i<s.size(); i++) {
    if (*p==13 || *p==10)
      result+=*p;
    else if (*p<32 || *p>126)
      result+='.';
    else
      result+=*p;
    p++;
  }

  return result;
}



void LogManager::fileSelected(QListViewItem *qv) {
  string s;

  _currentFile=qv->text(0);
  fileView->setText(QString::null);
  s=_anonymize(bankSelector->currentText().ascii(),
               _currentFile.ascii(),
               _trustLevel);
  _currentLog=s;
  fileView->setText(QString::fromUtf8(_dump(s).c_str()));
}



void LogManager::saveFile() {
  while(1) {
    QFileDialog fd(this,
                   "saveFile file dialog");
    fd.setCaption(tr("Enter file name"));
    fd.setMode(QFileDialog::AnyFile);
    if (!_lastDir.isEmpty())
      fd.setDir(QDir(_lastDir));

    if (fd.exec()==QDialog::Accepted) {
      QString filename=fd.selectedFile();
      string s;
      const char *p;
      unsigned int left;
      QFile f(filename);

      _lastDir=fd.dirPath();
      if (f.exists()) {
        int rv=QMessageBox::warning(this,
                                tr("Warning"),
                                tr("<qt>"
                                   "<p>"
                                   "File \"%1\" already exists. "
                                   "</p>"
                                   "<p>"
                                   "Do you want me to overwrite it?"
                                   "</p>"
                                   "</qt>"
                                  ).arg(filename),
                                QMessageBox::Yes,QMessageBox::No,
				QMessageBox::Abort);
        if (rv == 2 || rv == QMessageBox::Abort)
          return;
        else if (rv == 1 || rv == QMessageBox::No)
          continue;
      }

      // IO_Raw doesn't exist in qt4, but in latest qt4 this is
      // correctly replaced by QIODevice::Unbuffered.
      if (!f.open(IO_WriteOnly | IO_Truncate
		  | IO_Raw
		 )) {
	QMessageBox::critical(this,
			      tr("File Error"),
                              tr("<qt>"
                                 "<p>"
                                 "Could not create file \"%1\""
                                 "</p>"
                                 "</qt>"
                                ).arg(filename),
                              QMessageBox::Ok,QMessageBox::NoButton);
        return;
      }
      s=_currentLog;
      p=s.c_str();
      left=s.length();
      while(left) {
        Q_LONG l;

        l=f.writeBlock(p, left);
        if (l<1) {
          QMessageBox::critical(this,
                                tr("File Error"),
                                tr("<qt>"
                                   "<p>"
                                   "Could not write to file \"%1\""
                                   "</p>"
                                   "</qt>"
                                  ).arg(filename),
                                QMessageBox::Ok,QMessageBox::NoButton);
          f.close();
          return;
        }
        left-=l;
      }
      f.close();
      return;
    }
    else
      break;
  } /* while */
}



#include "logmanager.moc"




