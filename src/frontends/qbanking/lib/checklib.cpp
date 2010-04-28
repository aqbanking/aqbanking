#include "qbanking.h"

#include <qapplication.h>
#include <qmessagebox.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <qregexp.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gwenhywfar.h>

#include <qdir.h>


int main(int argc, char *argv[]){
  QApplication a(argc, argv);
  QBanking *qb;
  int err;
  int rv;

  err=GWEN_Init();
  if (err) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  // AB_Banking will create that folder and a file in there
  const char *testfolder = "testfolder";
  // AB_BANKING will create this test file
  const char *settings_conf = "settings.conf";

  // create new object
  qb=new QBanking("testLib", testfolder);
  rv=qb->init();
  if (rv) {
    qFatal("Error on QBanking::init: %d", rv);
  } 
  rv=qb->onlineInit();
  if (rv) {
    qFatal("Error on AB_Banking::onlineInit: %d", rv);
  } 
  // else qDebug("QBanking::init successful.");

  // maybe insert QBanking testcases here...

  // and delete it again
  if (qb->onlineFini()) {
    qFatal("Error on AB_Banking::onlineFini: %d", rv);
  }
  if (qb->fini()) {
    qFatal("Error on QBanking::fini: %d", rv);
  }
  delete qb;

  // Remove the test directory again
  QDir testdir(testfolder);
  if ( !testdir.exists() )
    qFatal("Error: Testfolder \"%s\" was not created by aqbanking", testfolder);
  // count() returns the number of files, including the
  // directory entries "." and ".."
  if ( testdir.count() != 3 )
    qFatal("Error: Testfolder contains %d files instead of one", 
	   testdir.count()-2);
  if ( ! testdir.remove(settings_conf) )
    qFatal("Error on removing the test file");
  if ( ! QDir().rmdir( testdir.path() ) )
    qFatal("Error on removing the test directory");

  // Just some more testcases:
  if (!QBanking::isPure7BitAscii(QString("abcABC789"))) {
    qFatal("Error on QBanking::isPure7BitAscii");
  }
  if (QString(QBanking::guiString(QString("bla")).c_str())
      != QString("bla")) {
    qFatal("Error on QBanking::guiString");
  }
  QString guistring(QBanking::guiString(QString("blub<html>bla</html>")).c_str());
  QRegExp rx("^\\s*<qt>\\s*bla\\s*</qt>\\s*$");
  if (rx.search(guistring) == -1) {
    qWarning("Error on QBanking::guiString: %s", 
	      (const char*)(guistring.local8Bit()));
    qFatal("returned guistring is: %s", (const char*)(guistring.local8Bit()));
  }
  if (QBanking::QStringToUtf8String(QString("foo bar")) !=
      std::string("foo bar")) {
    qFatal("Error on QBanking::QStringToUtf8String");
  }

  return 0;
}
