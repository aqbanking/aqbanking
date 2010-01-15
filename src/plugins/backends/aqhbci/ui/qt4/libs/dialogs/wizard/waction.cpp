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

#include "waction.h"
#include "wizard.h"

#include <qwidget.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3VBoxLayout>


WizardAction::WizardAction(Wizard *w,
                           const QString &aname,
                           const QString &descr,
                           QWidget* parent,
                           const char* wname, Qt::WFlags fl)
:QWidget(parent, wname, fl)
,_wizard(w)
,_name(aname)
,_descr(descr) {
  _pageLayout=new Q3VBoxLayout(this);

}



WizardAction::~WizardAction() {
}



void WizardAction::addWidget(QWidget *w) {
  _pageLayout->addWidget(w);
}



Wizard *WizardAction::getWizard() {
  return _wizard;
}



const QString &WizardAction::getName() const {
  return _name;
}



const QString &WizardAction::getDescription() const {
  return _descr;
}



bool WizardAction::apply() {
  return true;
}



bool WizardAction::undo() {
  return true;
}



void WizardAction::enter() {
}



void WizardAction::leave(bool backward) {
}



void WizardAction::setNextEnabled(bool b) {
  _wizard->setNextEnabled(this, b);
}



void WizardAction::setBackEnabled(bool b) {
  _wizard->setBackEnabled(this, b);
}










