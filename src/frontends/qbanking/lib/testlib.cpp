#include "qbanking.h"

#include <qapplication.h>
#include <qmessagebox.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gwenhywfar.h>




int main(int argc, char *argv[]){
  QApplication a(argc, argv);
  QBanking *qb;
  int err;
  int rv;
  const char *s;
  AB_BANKINFO *bi;

  err=GWEN_Init();
  if (err) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  GWEN_Logger_Open(0,
		   "TestLib", 0,
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
               "Overriding loglevel for QBankManager with \"%s\"",
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
  s=getenv("GWEN_LOGLEVEL");
  if (s) {
    GWEN_LOGGER_LEVEL l;

    l=GWEN_Logger_Name2Level(s);
    if (l!=GWEN_LoggerLevel_Unknown)
      GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, l);
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
