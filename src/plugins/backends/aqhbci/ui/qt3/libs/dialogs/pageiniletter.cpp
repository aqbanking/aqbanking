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


#include "wizard.h"

#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qwizard.h>
#include <qcombobox.h>
#include <qtextbrowser.h>

#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qpalette.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qtextview.h>
#include <qlabel.h>

#include <qpalette.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qprinter.h>


#include <gwenhywfar/debug.h>



bool Wizard::initIniLetterPage() {
  QObject::connect((QObject*)(printButton),
		   SIGNAL(clicked()),
		   this,
		   SLOT(slotPrintIniLetter()));
  return true;
}



bool Wizard::enterIniLetterPage(QWidget *p) {
  std::string hash;
  std::string modulus;
  std::string exponent;
  QString result;
  GWEN_CRYPTKEY *key;
  int keyNum;
  int keyVer;

  if (!iniBrowser->text().isEmpty())
    return true;

  assert(_medium);

  if (!AH_Medium_IsMounted(_medium)) {
    if (AH_Medium_Mount(_medium)) {
      DBG_ERROR(0, "Could not mount medium");
      return false;
    }
  }

  if (AH_Medium_SelectContext(_medium, AH_User_GetContextIdx(_user))) {
    DBG_ERROR(0, "Could not select user");
    QMessageBox::critical(this,
			  tr("Medium Error"),
			  tr("Could not select user context on medium.\n"
			     "Please check the logs."
			    ),
			  QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }

  key=AH_Medium_GetLocalPubSignKey(_medium);
  if (!key) {
    DBG_ERROR(0, "No sign key");
    return false;
  }

  modulus=_getIniLetterModulus(key);
  exponent=_getIniLetterExponent(key);
  hash=_getIniLetterHash(key);
  keyNum=GWEN_CryptKey_GetNumber(key);
  keyVer=GWEN_CryptKey_GetVersion(key);
  GWEN_CryptKey_free(key);
  key=0;

  result+="<qt>"
      "<h3>"+QWidget::tr("User Data (Ini Letter)")+"</h3>"
      "<table>"
      "<tr><td>" + QWidget::tr("User Name") + "</td><td>";
  if (nameEdit->text().isEmpty())
    result += QString::fromUtf8(AH_Customer_GetFullName(_customer));
  else
    result += nameEdit->text();
  result+="</td></tr>"

      "<tr><td>" + QWidget::tr("Date") + "</td><td>";
  result+=QDate::currentDate().toString();
  result+="</td></tr>"

      "<tr><td>"+QWidget::tr("Time")+"</td><td>";
  result+=QTime::currentTime().toString();
  result+="</td></tr>"

      "<tr><td>" + QWidget::tr("User Id")+"</td><td>";
  if (userIdEdit->text().isEmpty())
    result += QString::fromUtf8(AH_User_GetUserId(_user));
  else
    result += userIdEdit->text();
  result+="</td></tr>"

      "<tr><td>"+QWidget::tr("Key Number")+"</td><td>";
  result+=QString::number(keyNum);
  result+="</td></tr>"

      "<tr><td>"+QWidget::tr("Key Version")+"</td><td>";
  result+=QString::number(keyVer);
  result+="</td></tr>"

      "<tr><td>"+QWidget::tr("Application Name")+"</td><td>";
  const char *s=AH_HBCI_GetProductName(_hbci);
  if (s) {
    result+=QString::fromUtf8(s) + " ";
  }
  result+=QString("</td></tr>"
		  "</table>\n");

  result+="<h3>"+QWidget::tr("Public Key for Electronic Signature")+"</h3>";
  result+="<h4>"+QWidget::tr("Exponent")+"</h4>";
  if (exponent.length()<192) {
    DBG_ERROR(0, "Bad exponent");
    return false;
  }
  result+="<font face=fixed>" +
      QString::fromUtf8(_dumpHexString(exponent).c_str()) +
      "</font>"
      "<br>";

  result+="<h4>"+QWidget::tr("Modulus")+"</h4>";
  if (exponent.length()<192) {
    DBG_ERROR(0, "Bad modulus");
    return false;
  }
  result+="<font face=fixed>" +
      QString::fromUtf8(_dumpHexString(modulus).c_str()) +
      "</font>"
      "<br>";

  result+="<h4>"+QWidget::tr("Hash")+"</h4>";
  result+="<font face=fixed>" +
      QString::fromUtf8(_dumpHexString(hash, 40).c_str()) +
      "</font>";

  result+="<br><br><br>"
      "<hr>" +
      QWidget::tr("Location/Date/Signature") +
      "</qt>";

  iniBrowser->setText(result);

  return true;
}



void Wizard::slotPrintIniLetter(){
  QPrinter printer;
  const int XMargin=20;
  const int YMargin=50;

  if (printer.setup(this)) {
    QPainter p;
    QFont fnt(QString("times"), 12);

    if (!p.begin(&printer))
      return;
    QPaintDeviceMetrics metrics(&printer);
    QSimpleRichText txt(iniBrowser->text(), fnt);
    if (txt.height()+YMargin>metrics.height()-YMargin) {
      QMessageBox::critical(this,
                            tr("Print Ini letter"),
                            tr("Ini letter does not fit on the page."),
                            QMessageBox::Abort,QMessageBox::NoButton);
      return;
    }
    txt.draw(&p,XMargin,YMargin,
             QRegion(XMargin,YMargin,
                     metrics.width()-XMargin*2,
                     metrics.height()-YMargin*2),
             QColorGroup(QBrush("black"), // foreground
                         QBrush("white"), // button (unused)
                         QBrush("white"), // light (unused)
                         QBrush("white"), // dark (unused)
                         QBrush("white"), // mid (unused)
                         QBrush("black"), // text
                         QBrush("black"), // bright-text
                         QBrush("white"), // base (unused)
                         QBrush("white")) // background
            );
    p.end();
  }
}



bool Wizard::doIniLetterPage(QWidget *p){
  return true;
}



bool Wizard::undoIniLetterPage(QWidget *p){
  return true;
}





