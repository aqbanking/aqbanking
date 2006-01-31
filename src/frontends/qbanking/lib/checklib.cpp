#include "qbanking.h"

#include <qapplication.h>
#include <qmessagebox.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <qregexp.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gwenhywfar.h>




int main(int argc, char *argv[]){
  QApplication a(argc, argv);
  QBanking *qb;
  GWEN_ERRORCODE err;
  int rv;

  err=GWEN_Init();
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  // create new object
  qb=new QBanking("testLib", "testfolder");
  rv=qb->init();
  if (rv) {
    DBG_ERROR(0, "Error on QBanking::init: %d", rv);
    return 3;
  }

  // maybe insert QBanking testcases here...

  // and delete it again
  if (qb->fini()) {
    DBG_ERROR(0, "Error on QBanking::fini: %d", rv);
    delete qb;
    return 4;
  }
  delete qb;

  // Just some more testcases:
  if (!QBanking::isPure7BitAscii(QString("abcABC789"))) {
    DBG_ERROR(0, "Error on QBanking::isPure7BitAscii");
    return -1;
  }
  if (QString(QBanking::guiString(QString("bla")).c_str())
      != QString("bla")) {
    DBG_ERROR(0, "Error on QBanking::guiString");
    return -1;
  }
  QString guistring(QBanking::guiString(QString("blub<html>bla</html>")).c_str());
  QRegExp rx("^\\s*<qt>\\s*bla\\s*</qt>\\s*$");
  if (rx.search(guistring) == -1) {
    DBG_ERROR(0, "Error on QBanking::guiString: %s", 
	      (const char*)(guistring.local8Bit()));
    qDebug("returned guistring is: %s", (const char*)(guistring.local8Bit()));
    return -1;
  }
  if (QBanking::QStringToUtf8String(QString("foo bar")) !=
      std::string("foo bar")) {
    DBG_ERROR(0, "Error on QBanking::QStringToUtf8String");
    return -1;
  }

  return 0;
}
