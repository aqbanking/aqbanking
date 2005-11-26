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
#include <gwenhywfar/crypt.h>

#include <qstring.h>
#include <string>


class Wizard;



class IniLetter : public IniLetterUi {
  Q_OBJECT
private:
  Wizard *_wizard;
  const GWEN_CRYPTKEY *_key;
  bool _isUserKey;
  bool _result;
  QString _userName;
  QString _userId;
  QString _appName;

private:
  void _createIniLetter();
  std::string _getModulusData(const GWEN_CRYPTKEY *key) const;
  std::string _getExpData(const GWEN_CRYPTKEY *key) const;
  std::string _getIniLetterModulus(const GWEN_CRYPTKEY *key) const;
  std::string _getIniLetterExponent(const GWEN_CRYPTKEY *key) const;
  std::string _getIniLetterHash(const GWEN_CRYPTKEY *key) const;
  std::string _ripe(const std::string &src) const;
  std::string _dumpHexString(const std::string &s, int size=32);

public:
  IniLetter(bool userKey,
            QWidget* parent = 0, const char* name = 0, WFlags fl = 0);

  ~IniLetter();

  bool init(const QString &userName,
            const QString &userId,
            const QString &appName,
            const GWEN_CRYPTKEY *key);

  bool init(const QString &bankId,
            const GWEN_CRYPTKEY *key);

  void reset();

  bool getResult() const;

public slots:
  void slotGoodHash();
  void slotBadHash();
  void slotPrint();

};



#endif

