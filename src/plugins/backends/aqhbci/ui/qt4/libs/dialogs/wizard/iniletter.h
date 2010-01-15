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


#ifndef AQHBCI_INILETTER_H
#define AQHBCI_INILETTER_H

#include "iniletter.ui.h"

#include <aqbanking/provider.h>
#include <aqbanking/user.h>

#include <qstring.h>
#include <string>


class Wizard;



class IniLetter : public QWidget, public Ui_IniLetterUi {
  Q_OBJECT
private:
  bool _isUserKey;
  AB_PROVIDER *_provider;
  AB_USER *_user;
  bool _result;

private:
  void _createIniLetter();

public:
  IniLetter(bool userKey,
	    AB_PROVIDER *pro,
            QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);

  ~IniLetter();

  bool init(AB_USER *u);

  void reset();

  bool getResult() const;

public slots:
  void slotGoodHash();
  void slotBadHash();

};



#endif

