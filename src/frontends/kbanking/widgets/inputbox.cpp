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


#include <qlabel.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qtextbrowser.h>
#include <qlineedit.h>
#include <qsimplerichtext.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <qtimer.h>

#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>

#include <ctype.h>

#include "inputbox.h"


KBInputBox::Validator::Validator(QObject *parent, const char *name,
                                 GWEN_TYPE_UINT32 flags,
                                 int minLen, int maxLen)
:QValidator(parent, name), _flags(flags), _minLen(minLen), _maxLen(maxLen) {
}



KBInputBox::Validator::~Validator(){
}



QValidator::State
KBInputBox::Validator::validate(QString &input, int &pos) const{
  int i;

  if (_flags & AB_BANKING_INPUT_FLAGS_NUMERIC) {
    const char *p;

    p=input.latin1();
    if (p) {
      while(*p) {
        if (!isdigit(*(p++))) {
          DBG_DEBUG(0, "Not a digit.\n");
          return QValidator::Invalid;
        }
      }
    } /* if there is input */
  }
  i=input.length();
  if (i>=_minLen && i<=_maxLen)
    return Acceptable;
  else {
    DBG_DEBUG(0, "Bad length (%d).\n", i);
    return Intermediate;
  }
}





KBInputBox::KBInputBox(const QString& title,
                       const QString& text,
                       GWEN_TYPE_UINT32 flags,
                       int minLen,
                       int maxLen,
                       QWidget* parent,
                       const char* name,
                       bool modal,
                       WFlags fl)
:QDialog(parent, name, modal, fl)
,_flags(flags)
,_edit1(0)
,_edit2(0)
{
  QSimpleRichText rt(text, font());
  int width;
  int height;
  int max_textwidth=400;
  int max_textheight=400;
  QLabel *l;

  _validator=new Validator(this, "Validator", flags, minLen, maxLen);
  rt.setWidth(max_textwidth);
  width=rt.widthUsed();
  height=rt.height();

  setCaption(title);

  QBoxLayout *layout = new QVBoxLayout( this, 10, 6, "layout" );

  if (width > max_textwidth || height > max_textheight) {
    QTextEdit *t;

    /* use QTextBrowser instead of QTextLabel */
    t=new QTextEdit(this, "TextBox");
    t->setText(text);
    t->setReadOnly(true);
    t->setPaper(this->backgroundBrush());
    //t->setFrameStyle(QFrame::Box | QFrame::Sunken );
    layout->addWidget(t);
  }
  else {
    QLabel *t;

    /* use QLabel for short text */
    t=new QLabel(text, this, "TextBox");
    t->setAlignment(Qt::WordBreak); // the others were already default
    layout->addWidget(t);
  }

  /* The first label and input box: add label */
  QGridLayout *gridlayout = new QGridLayout( layout, 1, 2, 6, "gridlayout" );
  _edit1=new QLineEdit(this, "EditBox1");
  _edit1->setValidator(_validator);
  QObject::connect((QObject*)_edit1, SIGNAL(returnPressed()),
                   this, SLOT(returnPressedOn1()));
  QObject::connect((QObject*)_edit1, SIGNAL(textChanged(const QString&)),
                   this, SLOT(textChanged(const QString&)));
  if (flags & AB_BANKING_INPUT_FLAGS_SHOW)
    _edit1->setEchoMode(QLineEdit::Normal);
  else
    _edit1->setEchoMode(QLineEdit::Password);
  gridlayout->addWidget(_edit1, 0, 1);

  l=new QLabel(_edit1, tr("&Input")+":", this, "input_Label");
  gridlayout->addWidget(l, 0, 0);

  if (flags & AB_BANKING_INPUT_FLAGS_CONFIRM) {
    /* add QLineEdit */
    _edit2=new QLineEdit(this, "EditBox2");
    _edit2->setValidator(_validator);
    QObject::connect((QObject*)_edit2, SIGNAL(returnPressed()),
                     this, SLOT(returnPressedOn2()));
    QObject::connect((QObject*)_edit2, SIGNAL(textChanged(const QString&)),
                     this, SLOT(textChanged(const QString&)));
    if (flags & AB_BANKING_INPUT_FLAGS_SHOW)
      _edit2->setEchoMode(QLineEdit::Normal);
    else
      _edit2->setEchoMode(QLineEdit::Password);
    gridlayout->addWidget(_edit2, 1, 1);

    /* add label */
    l=new QLabel(_edit2, tr("&Confirm")+":", this, "Label2");
    gridlayout->addWidget(l, 1, 0);
  }

  // Separator between input boxes and buttons
  QFrame* line1 = new QFrame( this, "line1" );
  line1->setFrameShape( QFrame::HLine );
  line1->setFrameShadow( QFrame::Sunken );
  layout->addWidget( line1 );

  // Buttons
  QBoxLayout *buttonlayout = new QHBoxLayout( layout, -1, "buttonlayout" );
  buttonlayout->addStretch();
  _okButton=new QPushButton(tr("&Ok"), this, "OkButton");
  _abortButton=new QPushButton(tr("&Abort"), this, "AbortButton");
  
  // Force buttons to be of same size. Copied from
  // QInputDialog::getText() code.
  QSize bs = _okButton->sizeHint().expandedTo( _abortButton->sizeHint() );
  _okButton->setFixedSize( bs );
  _abortButton->setFixedSize( bs );
  buttonlayout->addWidget(_okButton);
  buttonlayout->addWidget(_abortButton);

  QObject::connect((QObject*)_okButton, SIGNAL(clicked()),
                   this, SLOT(accept()));
  QObject::connect((QObject*)_abortButton, SIGNAL(clicked()),
                   this, SLOT(reject()));

  _edit1->setFocus();
  _okButton->setEnabled(false);

  show();
  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



KBInputBox::~KBInputBox(){
}



bool KBInputBox::acceptableInput() {
  if (_edit1->hasAcceptableInput()) {
    if (_flags & AB_BANKING_INPUT_FLAGS_CONFIRM) {
      if (!_edit2->hasAcceptableInput())
        return false;
      if (_edit1->text().compare(_edit2->text())==0)
        return true;
      else
        return false;
    }
    return true;
  }
  return false;
}



QString KBInputBox::getInput() {
  return _edit1->text();
}



void KBInputBox::returnPressedOn1(){
  if (!(_flags & AB_BANKING_INPUT_FLAGS_CONFIRM)) {
    accept();
  }
  else
    _edit2->setFocus();
}



void KBInputBox::returnPressedOn2(){
  accept();
}



void KBInputBox::accept() {
  if (acceptableInput())
    QDialog::accept();
}



void KBInputBox::textChanged(const QString &t) {
  _okButton->setEnabled(acceptableInput());
}










