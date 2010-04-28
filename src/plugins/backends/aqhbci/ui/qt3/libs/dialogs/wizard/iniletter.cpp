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

#include "iniletter.h"

#include <qbanking/qbanking.h>

#include <aqhbci/provider.h>

#include <gwenhywfar/debug.h>

#include <qmessagebox.h>
#include <qstring.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextview.h>
#include <qlabel.h> // for qt4 setWordWrap(true)



IniLetter::IniLetter(bool useUserKey,
		     AB_PROVIDER *pro,
                     QWidget* parent,
                     const char* name,
                     WFlags fl)
:IniLetterUi(parent, name, fl)
,_isUserKey(useUserKey)
,_provider(pro)
,_user(NULL)
,_result(false){
  if (_isUserKey) {
    textLabel->setText( tr( "<qt>\n"
"This is the Ini-Letter of you, the user. "
"Please print out a paper copy by pressing \"Print\". "
"Then sign this paper copy and send it to your bank.\n"
			    "</qt>" ) );
    serverLabel->hide();
    goodHashButton->hide();
    badHashButton->hide();
  }
  else {
    goodHashButton->setEnabled(true);
    badHashButton->setEnabled(true);
    connect(goodHashButton, SIGNAL(clicked()), this, SLOT(slotGoodHash()));
    connect(badHashButton, SIGNAL(clicked()), this, SLOT(slotBadHash()));
  }
#if (QT_VERSION >= 0x040000)
  // In qt4, QLabel has word-wrap disabled by default
  textLabel->setWordWrap(true);
#endif // QT_VERSION >= 4
}



IniLetter::~IniLetter() {
}



bool IniLetter::init(AB_USER *u) {
  _user=u;
  _createIniLetter();
  return true;
}



void IniLetter::reset() {
  iniBrowser->setText("");
  if (!_isUserKey) {
    goodHashButton->setEnabled(true);
    badHashButton->setEnabled(true);
  }
}





bool IniLetter::getResult() const {
  return _result;
}



void IniLetter::slotGoodHash() {
  _result=true;
  goodHashButton->setEnabled(false);
  badHashButton->setEnabled(false);
}



void IniLetter::slotBadHash() {
  _result=false;
  goodHashButton->setEnabled(false);
  badHashButton->setEnabled(false);
}



void IniLetter::_createIniLetter() {
  GWEN_BUFFER *buf;
  int rv;

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AH_Provider_GetIniLetterHtml(_provider, _user,
                                  _isUserKey?0:1,
                                  0,
				  buf,
				  1);
  if (rv) {
    QMessageBox::critical(this,
			  tr("Error"),
			  tr("Could not create ini letter"),
			  QMessageBox::Ok,QMessageBox::NoButton);
    GWEN_Buffer_free(buf);
    return;
  }

  iniBrowser->setText(QString::fromUtf8(GWEN_Buffer_GetStart(buf)));
  GWEN_Buffer_free(buf);
}


#include "iniletter.moc"


