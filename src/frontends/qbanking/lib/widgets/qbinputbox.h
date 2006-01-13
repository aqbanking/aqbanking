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

#ifndef QBANKING_INPUTBOX_H
#define QBANKING_INPUTBOX_H


#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>

#include <qdialog.h>
#include <qvalidator.h>


class QLineEdit;
class QPushButton;


class QBInputBox: public QDialog {
  Q_OBJECT
private:

  class Validator: public QValidator {
  private:
    GWEN_TYPE_UINT32 _flags;
    int _minLen;
    int _maxLen;
  public:
    Validator(QObject *parent, const char *name,
              GWEN_TYPE_UINT32 flags,
              int minLen, int maxLen);
    ~Validator();
    State validate(QString &input, int &pos) const;
  };

  GWEN_TYPE_UINT32 _flags;
  QLineEdit *_edit1;
  QLineEdit *_edit2;
  QPushButton *_okButton;
  QPushButton *_abortButton;
  Validator *_validator;

public:
  QBInputBox(const QString& title,
             const QString& text,
             GWEN_TYPE_UINT32 flags,
             int minLen,
             int maxLen,
             QWidget* parent = 0,
             const char* name = 0,
             bool modal = false,
             WFlags fl = 0);
  ~QBInputBox();

  bool acceptableInput();
  QString getInput();

public slots:
  void returnPressedOn1();
  void returnPressedOn2();
  void accept();
  void textChanged(const QString&);
};


#endif
