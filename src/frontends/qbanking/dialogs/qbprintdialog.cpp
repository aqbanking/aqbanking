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


#include "qbprintdialog.h"

#include <qlabel.h>
#include <qtextbrowser.h>
#include <qpushbutton.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qsimplerichtext.h>
#include <qmessagebox.h>
#include <qfontdialog.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>



QBPrintDialog::QBPrintDialog(QBanking *app,
                             const char *docTitle,
                             const char *docType,
                             const char *descr,
                             const char *text,
                             QWidget* parent,
                             const char* name,
                             bool modal,
                             WFlags fl)
:QBPrintDialogUi(parent, name, modal, fl)
,_banking(app)
,_docTitle(docTitle)
,_docType(docType)
,_descr(descr)
,_text(text)
,_printer(new QPrinter(QPrinter::PrinterResolution))
,_fontFamily("Arial")
,_fontSize(11)
,_fontWeight(QFont::Normal)
{
  setCaption(QString::fromUtf8(docTitle));
  descrLabel->setText(QString::fromUtf8(descr));

  QObject::connect((QObject*)printButton, SIGNAL(clicked()),
                   this, SLOT(slotPrint()));
  QObject::connect((QObject*)setupButton, SIGNAL(clicked()),
                   this, SLOT(slotSetup()));
  QObject::connect((QObject*)fontButton, SIGNAL(clicked()),
                   this, SLOT(slotFont()));
  QObject::connect((QObject*)closeButton, SIGNAL(clicked()),
                   this, SLOT(accept()));
  QObject::connect((QObject*)abortButton, SIGNAL(clicked()),
                   this, SLOT(reject()));

  loadGuiSetup();
  loadPrinterSetup();

  DBG_ERROR(0, "Setting text...");
  textBrowser->setText(QString::fromUtf8(text));
  DBG_ERROR(0, "Setting text... done");

  //GWEN_Text_DumpString(text, strlen(text), stderr, 2);
}



QBPrintDialog::~QBPrintDialog(){
}



void QBPrintDialog::loadGuiSetup(){
  GWEN_DB_NODE *db;
  GWEN_BUFFER *dbuf;
  int x, y;

  db=_banking->getAppData();
  assert(db);

  db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      "gui/dlgs/printdialog");
  if (!db)
    return;
  dbuf=GWEN_Buffer_new(0, 64, 0, 1);
  if (GWEN_Text_EscapeToBuffer(_docType, dbuf)) {
    DBG_ERROR(0, "Internal error.");
    abort();
  }
  db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      GWEN_Buffer_GetStart(dbuf));
  GWEN_Buffer_free(dbuf);
  if (!db)
    return;
  db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      "gui");
  if (!db)
    return;

  x=GWEN_DB_GetIntValue(db, "width", 0, -1);
  y=GWEN_DB_GetIntValue(db, "height", 0, -1);
  if (x!=-1 && y!=-1) {
    DBG_ERROR(0, "Resizing to %d/%d", x, y);
    resize(x, y);
  }
  x=GWEN_DB_GetIntValue(db, "x", 0, -1);
  y=GWEN_DB_GetIntValue(db, "y", 0, -1);
  if (x!=-1 && y!=-1)
    move(x, y);
}



void QBPrintDialog::saveGuiSetup(){
  GWEN_DB_NODE *db;
  GWEN_BUFFER *dbuf;

  db=_banking->getAppData();
  assert(db);

  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
                      "gui/dlgs/printdialog");
  assert(db);
  dbuf=GWEN_Buffer_new(0, 64, 0, 1);
  if (GWEN_Text_EscapeToBuffer(_docType, dbuf)) {
    DBG_ERROR(0, "Internal error.");
    abort();
  }
  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
                      GWEN_Buffer_GetStart(dbuf));
  assert(db);
  GWEN_Buffer_free(dbuf);
  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                      "gui");
  assert(db);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "x", x());
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "y", y());
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "width", width());
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "height", height());
}



void QBPrintDialog::loadPrinterSetup(){
  GWEN_DB_NODE *db;
  GWEN_BUFFER *dbuf;
  const char *s;
  uint top, left, bottom, right;
  int i;

  db=_banking->getAppData();
  assert(db);

  db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      "gui/dlgs/printdialog");
  if (!db)
    return;
  dbuf=GWEN_Buffer_new(0, 64, 0, 1);
  if (GWEN_Text_EscapeToBuffer(_docType, dbuf)) {
    DBG_ERROR(0, "Internal error.");
    abort();
  }
  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
                      GWEN_Buffer_GetStart(dbuf));
  GWEN_Buffer_free(dbuf);
  if (!db)
    return;
  db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      "printer");
  if (!db)
    return;

  s=GWEN_DB_GetCharValue(db, "FontFamily", 0, "Arial");
  if (s)
    _fontFamily=QString::fromUtf8(s);
  _fontSize=GWEN_DB_GetIntValue(db, "FontSize", 0, 11);
  _fontWeight=QFont::Normal;
  s=GWEN_DB_GetCharValue(db, "FontWeight", 0, "Normal");
  _fontWeight=QFont::Normal;
  if (s) {
    if (strcasecmp(s, "Light")==0)
      _fontWeight=QFont::Light;
    else if (strcasecmp(s, "Normal")==0)
      _fontWeight=QFont::Normal;
    else if (strcasecmp(s, "DemiBold")==0)
      _fontWeight=QFont::DemiBold;
    else if (strcasecmp(s, "Bold")==0)
      _fontWeight=QFont::Bold;
    else if (strcasecmp(s, "Black")==0)
      _fontWeight=QFont::Black;
    else {
      DBG_WARN(0, "Unknown FontWeight \"%s\"", s);
    }
  }
  textBrowser->setFont(QFont(_fontFamily, _fontSize, _fontWeight));

  s=GWEN_DB_GetCharValue(db, "PageSize", 0, 0);
  if (s) {
    QPrinter::PageSize ps=QPrinter::A4;
    bool doSet;

    doSet=true;

    if (strcasecmp(s, "A0")==0)
      ps=QPrinter::A0;
    else if (strcasecmp(s, "A1")==0)
      ps=QPrinter::A1;
    else if (strcasecmp(s, "A2")==0)
      ps=QPrinter::A2;
    else if (strcasecmp(s, "A3")==0)
      ps=QPrinter::A3;
    else if (strcasecmp(s, "A4")==0)
      ps=QPrinter::A4;
    else if (strcasecmp(s, "A5")==0)
      ps=QPrinter::A5;
    else if (strcasecmp(s, "A6")==0)
      ps=QPrinter::A6;
    else if (strcasecmp(s, "A7")==0)
      ps=QPrinter::A7;
    else if (strcasecmp(s, "A8")==0)
      ps=QPrinter::A8;
    else if (strcasecmp(s, "A9")==0)
      ps=QPrinter::A9;
    else if (strcasecmp(s, "B0")==0)
      ps=QPrinter::B0;
    else if (strcasecmp(s, "B1")==0)
      ps=QPrinter::B1;
    else if (strcasecmp(s, "B2")==0)
      ps=QPrinter::B2;
    else if (strcasecmp(s, "B3")==0)
      ps=QPrinter::B3;
    else if (strcasecmp(s, "B4")==0)
      ps=QPrinter::B4;
    else if (strcasecmp(s, "B5")==0)
      ps=QPrinter::B5;
    else if (strcasecmp(s, "B6")==0)
      ps=QPrinter::B6;
    else if (strcasecmp(s, "B7")==0)
      ps=QPrinter::B7;
    else if (strcasecmp(s, "B8")==0)
      ps=QPrinter::B8;
    else if (strcasecmp(s, "B9")==0)
      ps=QPrinter::B9;
    else if (strcasecmp(s, "B10")==0)
      ps=QPrinter::B10;
    else if (strcasecmp(s, "C5E")==0)
      ps=QPrinter::C5E;
    else if (strcasecmp(s, "DLE")==0)
      ps=QPrinter::DLE;
    else if (strcasecmp(s, "Comm10E")==0)
      ps=QPrinter::Comm10E;
    else if (strcasecmp(s, "Executive")==0)
      ps=QPrinter::Executive;
    else if (strcasecmp(s, "Folio")==0)
      ps=QPrinter::Folio;
    else if (strcasecmp(s, "Ledger")==0)
      ps=QPrinter::Ledger;
    else if (strcasecmp(s, "Legal")==0)
      ps=QPrinter::Legal;
    else if (strcasecmp(s, "Letter")==0)
      ps=QPrinter::Letter;
    else if (strcasecmp(s, "Tabloid")==0)
      ps=QPrinter::Tabloid;
    else
      doSet=false;

    if (doSet)
      _printer->setPageSize(ps);
  }

  s=GWEN_DB_GetCharValue(db, "Orientation", 0, 0);
  if (s) {
    if (strcasecmp(s, "Portrait")==0)
      _printer->setOrientation(QPrinter::Portrait);
    else if (strcasecmp(s, "LandScape")==0)
      _printer->setOrientation(QPrinter::Landscape);
  }

  i=GWEN_DB_GetIntValue(db, "Resolution", 0, -1);
  if (i!=-1)
    _printer->setResolution(i);

  top=(uint)GWEN_DB_GetIntValue(db, "Top", 0, -1);
  left=(uint)GWEN_DB_GetIntValue(db, "Left", 0, -1);
  bottom=(uint)GWEN_DB_GetIntValue(db, "Bottom", 0, -1);
  right=(uint)GWEN_DB_GetIntValue(db, "Right", 0, -1);
#if (QT_VERSION < 0x040000)
  // the current Qt4 snapshots don't have QPrinter::setMargins
  if (top!=(uint)-1 && left!=(uint)-1 && bottom!=(uint)-1 && right!=(uint)-1)
    _printer->setMargins(top, left, bottom, right);
#endif

  s=GWEN_DB_GetCharValue(db, "ColorMode", 0, 0);
  if (s) {
    if (strcasecmp(s, "Color")==0)
      _printer->setColorMode(QPrinter::Color);
    else if (strcasecmp(s, "GrayScale")==0)
      _printer->setColorMode(QPrinter::GrayScale);
  }

  s=GWEN_DB_GetCharValue(db, "outputFileName", 0, 0);
  if (s)
    _printer->setOutputFileName(QString::fromUtf8(s));
  _printer->setOutputToFile(GWEN_DB_GetIntValue(db, "outputToFile", 0, 0));

}



void QBPrintDialog::savePrinterSetup() {
  GWEN_DB_NODE *db;
  GWEN_BUFFER *dbuf;
  const char *s;
  uint top, left, bottom, right;
  std::string fname;

  db=_banking->getAppData();
  assert(db);

  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
                      "gui/dlgs/printdialog");
  assert(db);
  dbuf=GWEN_Buffer_new(0, 64, 0, 1);
  if (GWEN_Text_EscapeToBuffer(_docType, dbuf)) {
    DBG_ERROR(0, "Internal error.");
    abort();
  }
  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT,
                      GWEN_Buffer_GetStart(dbuf));
  assert(db);
  GWEN_Buffer_free(dbuf);
  db=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                      "printer");
  assert(db);

  if (!_fontFamily.isEmpty())
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "FontFamily",
                         QBanking::QStringToUtf8String(_fontFamily).c_str());
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "FontSize",
                      _fontSize);
  switch(_fontWeight) {
  case QFont::Light:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "FontWeight", "Light");
    break;
  case QFont::Normal:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "FontWeight", "Normal");
    break;
  case QFont::DemiBold:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "FontWeight", "DemiBold");
    break;
  case QFont::Bold:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "FontWeight", "Bold");
    break;
  case QFont::Black:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "FontWeight", "Black");
    break;
  default:
    break;
  }

  QPrinter::Orientation orient=_printer->orientation();
  if (orient==QPrinter::Portrait)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "orientation", "portrait");
  else if (orient==QPrinter::Landscape)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "orientation", "landscape");
  switch(_printer->pageSize()) {
  case QPrinter::A0: s="A0"; break;
  case QPrinter::A1: s="A1"; break;
  case QPrinter::A2: s="A2"; break;
  case QPrinter::A3: s="A3"; break;
  case QPrinter::A4: s="A4"; break;
  case QPrinter::A5: s="A5"; break;
  case QPrinter::A6: s="A6"; break;
  case QPrinter::A7: s="A7"; break;
  case QPrinter::A8: s="A8"; break;
  case QPrinter::A9: s="A9"; break;
  case QPrinter::B0: s="B0"; break;
  case QPrinter::B1: s="B1"; break;
  case QPrinter::B2: s="B2"; break;
  case QPrinter::B3: s="B3"; break;
  case QPrinter::B4: s="B4"; break;
  case QPrinter::B5: s="B5"; break;
  case QPrinter::B6: s="B6"; break;
  case QPrinter::B7: s="B7"; break;
  case QPrinter::B8: s="B8"; break;
  case QPrinter::B9: s="B9"; break;
  case QPrinter::B10: s="B10"; break;
  case QPrinter::C5E: s="C5E"; break;
  case QPrinter::DLE: s="DLE"; break;
  case QPrinter::Comm10E: s="Comm10E"; break;
  case QPrinter::Executive: s="Executive"; break;
  case QPrinter::Folio: s="Folio"; break;
  case QPrinter::Ledger: s="Ledger"; break;
  case QPrinter::Legal: s="Legal"; break;
  case QPrinter::Letter: s="Letter"; break;
  case QPrinter::Tabloid: s="Tabloid"; break;
  case QPrinter::Custom:
    s="Custom";
    break;
  default:
    s=0;
    break;
  }
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "PageSize", s);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "Resolution", _printer->resolution());
  _printer->margins(&top, &left, &bottom, &right);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Top", top);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Left", left);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Bottom", bottom);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Right", right);

  switch(_printer->colorMode()) {
  case QPrinter::Color: s="Color"; break;
  case QPrinter::GrayScale: s="GrayScale"; break;
  default: s=0; break;
  }
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "ColorMode", s);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "outputToFile",
                      (_printer->outputToFile())?1:0);
  fname=QBanking::QStringToUtf8String(_printer->outputFileName());
  if (!fname.empty())
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "outputFileName", fname.c_str());

}


void QBPrintDialog::accept(){
  savePrinterSetup();
  saveGuiSetup();
  QDialog::accept();
}



void QBPrintDialog::slotSetup(){
  _printer->setup();
}


void QBPrintDialog::slotFont() {
  bool ok;
  QFont fnt=QFontDialog::getFont(&ok,
                                 QFont(_fontFamily,
                                       _fontSize,
                                       _fontWeight),
                                 this);

  if (ok) {
    _fontFamily=fnt.family();
    _fontSize=fnt.pointSize();
    _fontWeight=fnt.weight();
    textBrowser->setFont(fnt);
    //textBrowser->setText(QString::fromUtf8(_text));
  }
}



void QBPrintDialog::slotPrint(){
  QPainter p;
  QFont fnt(_fontFamily, _fontSize, _fontWeight);
  int XMargin;
  int YMargin;
  std::string swText;
  std::string swUnits;

  if (!p.begin(_printer)) {
    QMessageBox::critical(this,
                          tr("Print"),
                          tr("Printing aborted."),
                          tr("Dismiss"),0,0,0);
    return;
  }

  p.setFont(fnt);

  QPaintDeviceMetrics metrics(p.device());
  XMargin = 0; //(int)((2/2.54)*metrics.logicalDpiX()); // 2 cm margins
  YMargin = 0; //(int)((2/2.54)*metrics.logicalDpiY()); // 2 cm margins

  QRect body(XMargin, YMargin,
             metrics.width() - 2*XMargin,
             metrics.height() - 2*YMargin);
  QSimpleRichText txt(textBrowser->text(), fnt,
                      QString::null,
                      textBrowser->styleSheet(),
                      textBrowser->mimeSourceFactory(),
                      body.height());

  txt.setWidth(&p, body.width());

  if (txt.widthUsed()>body.width()) {
    if (QMessageBox::critical(this,
                              tr("Print"),
                              tr("Text does not fit on the page.\n"
                                 "Do you want to print it anyway?"),
                              tr("Yes"),tr("Abort"),0,0)!=0)
      return;
  }

  swText=QBanking::QStringToUtf8String(tr("Printing, please wait..."));
  swUnits=QBanking::QStringToUtf8String(tr("page(s)"));
  GWEN_WaitCallback_EnterWithText(GWEN_WAITCALLBACK_ID_SIMPLE_PROGRESS,
                                  swText.c_str(),
                                  swUnits.c_str(),
                                  0);
  GWEN_WaitCallback_SetProgressTotal(GWEN_WAITCALLBACK_PROGRESS_NONE);

  QRect view(body);
  int page = 1;
  do {
    DBG_ERROR(0, "Printing page %d", page);
    txt.draw(&p, body.left(), body.top(), view, colorGroup());
    view.moveBy(0, body.height());
    p.translate(0 , -body.height());
    p.drawText(view.right() - p.fontMetrics().width(QString::number(page)),
               view.bottom() + p.fontMetrics().ascent() + 5,
               QString::number(page));
    if (view.top()>=txt.height())
      break;
    _printer->newPage();
    if (GWEN_WaitCallbackProgress(page)==
        GWEN_WaitCallbackResult_Abort) {
      if (QMessageBox::critical(this,
                                tr("Aborted"),
                                tr("Do you really want to abort?"),
                                tr("Yes"),tr("No"),0)!=0) {
        GWEN_WaitCallback_Leave();
        return;
      }
    }
    page++;
  } while (TRUE);
  GWEN_WaitCallback_Leave();

  p.end();
}







