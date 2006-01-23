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

#ifndef QBANKING_PRINTDIALOG_H
#define QBANKING_PRINTDIALOG_H

#include "qbprintdialog.ui.h"
#include "qbanking.h"

#include <qstring.h>

class QPrinter;


class QBANKING_API QBPrintDialog : public QBPrintDialogUi {
  Q_OBJECT
public:
  QBPrintDialog(QBanking *app,
                const char *docTitle,
                const char *docType,
                const char *descr,
                const char *text,
                QWidget* parent=0,
                const char* name=0,
                bool modal=FALSE,
                WFlags fl=0);
  ~QBPrintDialog();

  void accept();

public slots:
  void slotPrint();
  void slotSetup();
  void slotFont();
  void slotHelpClicked();

private:
  QBanking *_banking;
  const char *_docTitle;
  const char *_docType;
  const char *_descr;
  const char *_text;
  QPrinter *_printer;
  QString _fontFamily;
  int _fontSize;
  int _fontWeight;

  void loadPrinterSetup();
  void savePrinterSetup();

  void loadGuiSetup();
  void saveGuiSetup();

};




#endif
