#include "qbanking.h"

#include <qapplication.h>
#include <qmessagebox.h>
#include <qtranslator.h>
#include <qtextcodec.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gwenhywfar.h>




int main(int argc, char *argv[]){
  QApplication a(argc, argv);
  QBanking *qb;
  QTranslator *translator;
  GWEN_ERRORCODE err;
  int rv;
  const char *s;
  AB_BANKINFO *bi;

  err=GWEN_Init();
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  GWEN_Logger_Open(0,
		   "TestLib", 0,
		   GWEN_LoggerTypeConsole,
		   GWEN_LoggerFacilityUser);

  // set loglevels
  s=getenv("LOGLEVEL");
  if (s) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(s);
    if (ll!=GWEN_LoggerLevelUnknown) {
      GWEN_Logger_SetLevel(0, ll);
      DBG_WARN(0,
               "Overriding loglevel for QBankManager with \"%s\"",
               s);
    }
    else {
      DBG_ERROR(0, "Unknown loglevel \"%s\"",
                s);
    }
  }
  else {
    GWEN_Logger_SetLevel(0, GWEN_LoggerLevelNotice);
  }
  s=getenv("GWEN_LOGLEVEL");
  if (s) {
    GWEN_LOGGER_LEVEL l;

    l=GWEN_Logger_Name2Level(s);
    if (l!=GWEN_LoggerLevelUnknown)
      GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, l);
  }

  translator=new QTranslator(0);
  QString pkgdatadir(PKGDATADIR);
  if (translator->load(QTextCodec::locale()+QString(".qm"),
                       QString(pkgdatadir + "/" "i18n" "/"))) {
    DBG_INFO(0, "I18N available for your language");
    qApp->installTranslator(translator);
  }
  else {
    DBG_WARN(0, "Internationalisation is not available for your language");
    delete translator;
    translator=0;
  }

  qb=new QBanking("testLib", "testfolder");
  rv=qb->init();
  if (rv) {
    DBG_ERROR(0, "Error on QBanking::init: %d", rv);
    // splash->finish(mw); -- mw not yet assigned
    QMessageBox::critical(0,
                          QWidget::tr("Initialization Error"),
                          QWidget::tr("<qt>"
                                      "Could not initialize QBanking."
                                      "</qt>"
                                     ),
                          QWidget::tr("Dismiss"));
    delete qb;
    return 2;
  }

  if (true)
  {
      char mybuffer[50];
      int rv = qb->inputBox(0, "Test for password InputBox", 
			    "This is a test. Just enter something.",
			    mybuffer,
			    1, 40);
      qDebug("QBanking::inputBox: rv=%d; Resulting password: \"%s\"",
	     rv, mybuffer);
  }

  bi=qb->selectBank();
  if (bi==0) {
    QMessageBox::critical(0,
			  QWidget::tr("Error"),
			  QWidget::tr("<qt>"
				      "No bank selected."
				      "</qt>"
				     ),
			  QWidget::tr("Dismiss"));
    delete qb;
    return 3;
  }
  AB_BankInfo_free(bi);

#if 0
  if (qb->fini()) {
    QMessageBox::critical(0,
                          QWidget::tr("Deinitialization Error"),
                          QWidget::tr("<qt>"
                                      "Could not deinitialize QBanking."
                                      "</qt>"
                                     ),
                          QWidget::tr("Dismiss"));
    delete qb;
    return 3;
  }
#endif

  delete qb;

  return 0;
}
