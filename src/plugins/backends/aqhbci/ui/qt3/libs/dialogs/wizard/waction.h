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


#ifndef AQHBCI_WACTION_H
#define AQHBCI_WACTION_H


class Wizard;
class WizardAction;

class QVBoxLayout;

#include <qstring.h>
#include <qwidget.h>


class WizardAction: public QWidget {
private:
  Wizard *_wizard;
  QVBoxLayout *_pageLayout;
  QString _name;
  QString _descr;

public:
  WizardAction(Wizard *w,
               const QString &aname,
               const QString &descr=QString::null,
               QWidget* parent=0, const char* name=0, WFlags fl=0);
  virtual ~WizardAction();

  void addWidget(QWidget *w);

  virtual bool apply();
  virtual bool undo();

  Wizard *getWizard();

  const QString &getName() const;
  const QString &getDescription() const;

};



#endif

