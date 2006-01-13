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

#include "actionwidget.h"

#include <gwenhywfar/debug.h>

#include <qmessagebox.h>
#include <qstring.h>
#include <qlabel.h>
#include <qtextview.h>
#include <qpushbutton.h>



ActionWidget::ActionWidget(const QString &titleText,
                           const QString &descrText,
                           const QString &buttonText,
                           QWidget* parent,
                           const char* name,
                           WFlags fl)
:ActionWidgetUi(parent, name, fl), _result(StatusNone) {
  _titleLabel->setText(titleText);
  _descrView->setText(descrText);
  _button->setText(buttonText);
  _resultLabel->setText("");
  setStatus(StatusNone);
}



ActionWidget::~ActionWidget() {
}



ActionWidget::Status ActionWidget::getStatus() const {
  return _result;
}



void ActionWidget::setStatus(Status r) {
  QString failed=QString("<qt><font colour=\"red\">"
			 "%1</font></qt>").arg(tr("Failed"));
  QString success=QString("<qt><font colour=\"green\">"
			  "%1</font></qt>").arg(tr("Success"));
  QString checking=QString("<qt><font colour=\"blue\">"
			   "%1</font></qt>").arg(tr("Checking..."));

  _result=r;
  switch(r) {
  case StatusNone:     _resultLabel->setText("");       break;
  case StatusChecking: _resultLabel->setText(checking); break;
  case StatusSuccess:  _resultLabel->setText(success);  break;
  case StatusFailed:   _resultLabel->setText(failed);   break;
  }
}



QPushButton *ActionWidget::getButton() {
  return _button;
}




