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


#include <gwenhywfar/debug.h>





bool Wizard::initMediumPage() {
  bool on;

  on=fileRadio->isOn();
  slotRDHToggled(on);
  setAppropriate(checkCardPage,!on);

  // page 1
  QObject::connect((QObject*)(cardRadio),
                   SIGNAL(toggled(bool)),
                   this,
                   SLOT(slotDDVToggled(bool)));
  QObject::connect((QObject*)(fileRadio),
                   SIGNAL(toggled(bool)),
                   this,
                   SLOT(slotRDHToggled(bool)));

  QObject::connect((QObject*)(fileNameButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotPathClicked()));
  QObject::connect((QObject*)(fileNameEdit),
                   SIGNAL(textChanged(const QString &)),
                   this,
                   SLOT(slotPathChanged(const QString &)));
  return true;
}




void Wizard::slotRDHToggled(bool on){
  fileNameButton->setEnabled(on);
  fileNameEdit->setEnabled(on);

  setAppropriate(checkCardPage, !on);
  setAppropriate(checkFilePage, on);

  if ((on && !fileNameEdit->text().isEmpty())
      || !on)
    setNextEnabled(mediumPage, true);
  else
    setNextEnabled(mediumPage, false);
}



void Wizard::slotDDVToggled(bool on){
  slotRDHToggled(!on);
}



void Wizard::slotPathClicked(){
  QString filename=QFileDialog::getSaveFileName(fileNameEdit->text(),
                                                QString::null,
						this, 
						"slotPathClicked file dialog",
						tr("Enter existing or new medium file name"));
  if (!filename.isEmpty())
    fileNameEdit->setText(filename);
}



void Wizard::slotPathChanged(const QString &t){
  if (t.isEmpty())
    setNextEnabled(mediumPage, false);
  else
    setNextEnabled(mediumPage,true);
}



bool Wizard::doMediumPage(QWidget *p){
  if (fileRadio->isOn()) {
    /* file mode selected */
    setAppropriate(checkFilePage, true);
    setAppropriate(checkCardPage, false);

    _mediumName=fileNameEdit->text().latin1();
    // FIXME: Err... this is the medium filename. If we are in a
    // utf8-locale, things will break if aqbanking tries to access
    // that file. Officially, aqbanking expects this in utf8 so
    // this should be QString::utf8 here. However, we don't
    // convert the filename from utf8 to the local8Bit encoding
    // each time it is used. Therefore the quick solution would be
    // QString::local8Bit, but this will break if the locale's
    // encoding changes. That's a hard one. Eventually, at all
    // fopen()s the aqbanking string needs to be converted from
    // utf8 into the local8Bit encoding, but that is not yet done
    // in aqbanking. That's bad.

    /* file wanted, check whether it exists */
    if (_importMode) {
      DBG_NOTICE(0, "Import mode");
      if (!QFile::exists(fileNameEdit->text())) {
        QMessageBox::critical(this,
                              tr("Medium not found"),
                              tr("<qt><p>The specified file does not exist. </p><p>"
                                 "If you want to import a file, the file needs to exist. "
				 "Please "
				 "go back to the page \"Import or Create\" "
				 "and choose \"Create\" if you want to "
				 "create a new medium.</p></qt>"
                                ),
                              QMessageBox::Ok,QMessageBox::NoButton);
        return false;
      }
      fromMediumButton->setEnabled(true);
      setAppropriate(selectFilePage, false);
      setAppropriate(checkFilePage, true);
      setAppropriate(systemIdPage, true);
      _createFile=false;
    }
    else {
      DBG_NOTICE(0, "Create mode");
      if (QFile::exists(fileNameEdit->text())) {
	if (QMessageBox::warning(this,
				 tr("File already exists"),
				 tr("<qt><p>The specified file already exists. However, you chose to create "
				    "a new medium. In that case the existing file will be overwritten and "
				    "all old data in that file will be lost.</p><p>Do you want to "
				    "overwrite the existing file?</p></qt>"
				    ),
				 QMessageBox::Yes,QMessageBox::No)!= 0)
	  return false;
      }
      setAppropriate(selectFilePage, true);
      setAppropriate(checkFilePage, false);
      setAppropriate(systemIdPage, true);
      _createFile=true;
      fromMediumButton->setEnabled(false);
    }
  }
  else {
    /* chip card mode selected */
    DBG_INFO(0, "Chipcard selected, will check for type");
    _mediumName.erase();
    setAppropriate(checkFilePage, false);
    setAppropriate(checkCardPage, true);
    setAppropriate(selectFilePage, false);
  }

  //setAppropriate(serverKeysPage, !_importMode);
  //setAppropriate(verifyKeysPage, !_importMode);
  //setAppropriate(createKeysPage, !_importMode);
  //setAppropriate(sendKeysPage, !_importMode);

  return true;
}



bool Wizard::undoMediumPage(QWidget *p){
  return true;
}















