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


#ifndef AQHBCI_ACTIONWIDGET_H
#define AQHBCI_ACTIONWIDGET_H

#include "actionwidget.ui.h"


class QPushButton;



class ActionWidget : public QWidget, public Ui_ActionWidgetUi {
public:
  typedef enum {
    StatusNone=0,
    StatusChecking,
    StatusSuccess,
    StatusFailed
  } Status;
private:
  Status _result;
public:
  ActionWidget(const QString &titleText,
               const QString &descrText,
               const QString &buttonText,
               QWidget* parent = 0,
               const char* name = 0,
               Qt::WFlags fl = 0);
  ~ActionWidget();

  Status getStatus() const;
  void setStatus(Status r);

  QPushButton *getButton();
};



#endif

