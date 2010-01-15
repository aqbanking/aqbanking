/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: qbinputbox.h 809 2006-01-20 14:15:15Z cstim $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef QGUI_INPUTBOX_H
#define QGUI_INPUTBOX_H


#include <gwenhywfar/types.h>

#include <qdialog.h>
#include <qvalidator.h>


class QLineEdit;
class QPushButton;


class QGuiInputBox: public QDialog {
  Q_OBJECT
private:

  class Validator: public QValidator {
  private:
    uint32_t _flags;
    int _minLen;
    int _maxLen;
  public:
    Validator(QObject *parent, const char *name,
              uint32_t flags,
              int minLen, int maxLen);
    ~Validator();
    State validate(QString &input, int &pos) const;
  };

  uint32_t _flags;
  QLineEdit *_edit1;
  QLineEdit *_edit2;
  QPushButton *_okButton;
  QPushButton *_abortButton;
  Validator *_validator;

public:
  QGuiInputBox(const QString& title,
	       const QString& text,
	       uint32_t flags,
	       int minLen,
	       int maxLen,
	       QWidget* parent = 0,
	       const char* name = 0,
	       bool modal = false,
	       Qt::WFlags fl = 0);
  ~QGuiInputBox();

  bool acceptableInput();
  QString getInput();

public slots:
  void returnPressedOn1();
  void returnPressedOn2();
  void accept();
  void textChanged(const QString&);
};


#endif
