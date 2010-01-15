//Added by qt3to4:
#include <Q3VBoxLayout>
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

class Q3VBoxLayout;

#include <qstring.h>
#include <qwidget.h>


class WizardAction: public QWidget {
private:
  Wizard *_wizard;
  Q3VBoxLayout *_pageLayout;
  QString _name;
  QString _descr;

public:
  WizardAction(Wizard *w,
               const QString &aname,
               const QString &descr=QString::null,
               QWidget* parent=0, const char* name=0, Qt::WFlags fl=0);
  virtual ~WizardAction();

  void addWidget(QWidget *w);

  virtual void enter();
  virtual void leave(bool backward);
  virtual bool apply();
  virtual bool undo();

  Wizard *getWizard();

  void setNextEnabled(bool b);
  void setBackEnabled(bool b);

  const QString &getName() const;
  const QString &getDescription() const;

};



#endif

