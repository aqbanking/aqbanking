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

#include "a_selectfile.h"
#include "selectfile.ui.h"
#include "wizard.h"

#include <qbanking/qbanking.h>
#include <aqhbci/provider.h>
#include <gwenhywfar/debug.h>

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qlabel.h>




ActionSelectFile::ActionSelectFile(Wizard *w, bool mustExist,
                                   const QString &title,
                                   const QString &descr)
  :WizardAction(w, "SelectFile", title)
,_mustExist(mustExist)
,_realDialog(0){

  _realDialog=new SelectFileUi(this);
  _realDialog->descrLabel->setText(descr);
  addWidget(_realDialog);
  _realDialog->show();
  setNextEnabled(false);
  connect(_realDialog->fileNameButton, SIGNAL(clicked()),
          this, SLOT(slotFileButtonClicked()));
  connect(_realDialog->fileNameEdit, SIGNAL(textChanged(const QString&)),
          this, SLOT(slotFileNameChanged(const QString&)));

}



ActionSelectFile::~ActionSelectFile() {
}



void ActionSelectFile::slotFileButtonClicked() {
  QString filename;
  QString title;

  if (_mustExist)
    title=tr("Enter existing medium file name");
  else
    title=tr("Enter new medium file name");

  filename=QFileDialog::getSaveFileName(_realDialog->fileNameEdit->text(),
                                        QString::null,
                                        this,
                                        "slotFileButtonClicked",
                                        title);
  if (!filename.isEmpty())
    _realDialog->fileNameEdit->setText(filename);
}



void ActionSelectFile::slotFileNameChanged(const QString &qs) {
  if (qs.isEmpty())
    setNextEnabled(false);
  else {
    if (QFile::exists(qs) ^ _mustExist)
      setNextEnabled(false);
    else
      setNextEnabled(true);
  }
}



void ActionSelectFile::enter() {
  std::string s;

  s=getWizard()->getWizardInfo()->getMediumName();
  if (!s.empty())
    _realDialog->fileNameEdit->setText(QString::fromUtf8(s.c_str()));
  slotFileNameChanged(_realDialog->fileNameEdit->text());
}



bool ActionSelectFile::apply() {
  std::string s;

  s=QBanking::QStringToUtf8String(_realDialog->fileNameEdit->text());
  if (s.empty())
    return false;
  getWizard()->getWizardInfo()->setMediumName(s);
  return true;
}












