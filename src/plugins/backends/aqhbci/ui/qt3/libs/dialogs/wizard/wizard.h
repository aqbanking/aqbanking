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


#ifndef AQHBCI_WIZARD_H
#define AQHBCI_WIZARD_H

class QBanking;
class Wizard;
class WizardAction;
class WizardInfo;

#include "wizard.ui.h"
#include "waction.h"
#include "winfo.h"

#include <qstring.h>
#include <qwizard.h>
#include <aqbanking/banking.h>

#include <list>


class Wizard: protected WizardUi {
  Q_OBJECT
private:
  QBanking *_app;
  WizardInfo *_wInfo;
  QWidget *_lastActionWidget;
  QString _logtext;

public:
  Wizard(QBanking *qb,
         WizardInfo *wInfo,
         const QString &caption=QString::null,
         QWidget* parent=0, const char* name=0,
         bool modal=FALSE);
  virtual ~Wizard();

  QBanking *getBanking();

  void addAction(WizardAction *a);
  WizardInfo *getWizardInfo();

  void setNextEnabled(WizardAction *a, bool b);
  void setBackEnabled(WizardAction *a, bool b);

  void log(GWEN_LOGGER_LEVEL level, const QString &s);
  void setDescription(const QString &s);

  QWidget *getWidgetAsParent();

  virtual int exec();

protected slots:
  virtual void back();
  virtual void next();
};



#endif

