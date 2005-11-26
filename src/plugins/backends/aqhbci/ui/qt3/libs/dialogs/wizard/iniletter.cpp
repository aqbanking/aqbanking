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

#include <aqhbci/outbox.h>
#include <aqhbci/adminjobs.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/md.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/text.h>

#include <qmessagebox.h>
#include <qstring.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextview.h>



IniLetter::IniLetter(bool isUser,
                     QWidget* parent,
                     const char* name,
                     WFlags fl)
:IniLetterUi(parent, name, fl)
,_key(0)
,_isUserKey(isUser)
,_result(false){
  if (_isUserKey) {
    serverLabel->hide();
    goodHashButton->hide();
    badHashButton->hide();
    connect(printButton, SIGNAL(clicked()), this, SLOT(slotPrint()));
  }
  else {
    goodHashButton->setEnabled(true);
    badHashButton->setEnabled(true);
    connect(goodHashButton, SIGNAL(clicked()), this, SLOT(slotGoodHash()));
    connect(badHashButton, SIGNAL(clicked()), this, SLOT(slotBadHash()));
  }
}



IniLetter::~IniLetter() {
}



bool IniLetter::init(const QString &userName,
                     const QString &userId,
                     const QString &appName,
                     const GWEN_CRYPTKEY *key) {
  _userName=userName;
  _userId=userId;
  _appName=appName;
  _key=key;

  _createIniLetter();
  return true;
}



bool IniLetter::init(const QString &bankId,
                     const GWEN_CRYPTKEY *key) {
  _userId=bankId;
  _key=key;


  _createIniLetter();

  return true;
}



void IniLetter::reset() {
  iniBrowser->setText("");
  if (!_isUserKey) {
    goodHashButton->setEnabled(true);
    badHashButton->setEnabled(true);
    _key=0;
  }
}





bool IniLetter::getResult() const {
  return _result;
}



void IniLetter::slotGoodHash() {
}



void IniLetter::slotBadHash() {
}



void IniLetter::_createIniLetter() {
  std::string hash;
  std::string modulus;
  std::string exponent;
  int keyNum;
  int keyVer;
  QString result;

  modulus=_getIniLetterModulus(_key);
  exponent=_getIniLetterExponent(_key);
  hash=_getIniLetterHash(_key);
  keyNum=GWEN_CryptKey_GetNumber(_key);
  keyVer=GWEN_CryptKey_GetVersion(_key);

  if (_isUserKey) {
    result+="<qt>";
    result+="<h3>"+tr("User Data (Ini Letter)")+"</h3>";

    result+="<table>";
    result+="<tr><td>" + tr("User Name") + "</td><td>";
    result+=_userName;
    result+="</td></tr>";

    result+="<tr><td>" + tr("Date") + "</td><td>";
    result+=QDate::currentDate().toString();
    result+="</td></tr>";

    result+="<tr><td>"+tr("Time")+"</td><td>";
    result+=QTime::currentTime().toString();
    result+="</td></tr>";

    result+="<tr><td>" + tr("User Id")+"</td><td>";
    result+=_userId;
    result+="</td></tr>";

    result+="<tr><td>"+tr("Key Number")+"</td><td>";
    result+=QString::number(keyNum);
    result+="</td></tr>";

    result+="<tr><td>"+tr("Key Version")+"</td><td>";
    result+=QString::number(keyVer);
    result+="</td></tr>";
  
    result+="<tr><td>"+tr("Application Name")+"</td><td>";
    result+=_appName;
    result+="</td></tr>";

    result+="</table>\n";
  
    result+="<h3>"+tr("Public Key for Electronic Signature")+"</h3>";
    result+="<h4>"+tr("Exponent")+"</h4>";
    if (exponent.length()<192) {
      DBG_ERROR(0, "Bad exponent");
      iniBrowser->setText(tr("Bad exponent"));
      return;
    }
    result+="<font face=fixed>" +
        QString::fromUtf8(_dumpHexString(exponent).c_str()) +
        "</font>"
        "<br>";
  
    result+="<h4>"+tr("Modulus")+"</h4>";
    if (exponent.length()<192) {
      iniBrowser->setText(tr("Bad modulus"));
      return;
    }
    result+="<font face=fixed>" +
        QString::fromUtf8(_dumpHexString(modulus).c_str()) +
        "</font>"
        "<br>";
  
    result+="<h4>"+tr("Hash")+"</h4>";
    result+="<font face=fixed>" +
        QString::fromUtf8(_dumpHexString(hash, 40).c_str()) +
        "</font>";
  
    result+="<br><br><br>"
        "<hr>" +
        tr("Location/Date/Signature") +
        "</qt>";
  }
  else {
    result+="<qt>";
    result+="<h3>"+tr("Ini Letter of Server Key)")+"</h3>";

    result+="<table>";

    result+="<tr><td>" + tr("Date") + "</td><td>";
    result+=QDate::currentDate().toString();
    result+="</td></tr>";

    result+="<tr><td>"+tr("Time")+"</td><td>";
    result+=QTime::currentTime().toString();
    result+="</td></tr>";

    result+="<tr><td>" + tr("Server Id")+"</td><td>";
    result += _userId;
    result+="</td></tr>";

    result+="<tr><td>"+tr("Key Number")+"</td><td>";
    result+=QString::number(keyNum);
    result+="</td></tr>";

    result+="<tr><td>"+tr("Key Version")+"</td><td>";
    result+=QString::number(keyVer);
    result+="</td></tr>";

    result+="</table>\n";

    result+="<h3>"+tr("Public Key")+"</h3>";
    result+="<h4>"+tr("Exponent")+"</h4>";
    if (exponent.length()<192) {
      iniBrowser->setText(tr("Bad exponent"));
      return;
    }
    result+="<font face=fixed>" +
        QString::fromUtf8(_dumpHexString(exponent).c_str()) +
        "</font>"
        "<br>";
  
    result+="<h4>"+tr("Modulus")+"</h4>";
    if (exponent.length()<192) {
      iniBrowser->setText(tr("Bad modulus"));
      return;
    }
    result+="<font face=fixed>" +
        QString::fromUtf8(_dumpHexString(modulus).c_str()) +
        "</font>"
        "<br>";
  
    result+="<h4>"+tr("Hash")+"</h4>";
    result+="<font face=fixed>" +
        QString::fromUtf8(_dumpHexString(hash, 40).c_str()) +
        "</font>";
  
    result+="<br><br><br>"
        "<hr>" +
        tr("Location/Date/Signature") +
        "</qt>";

  }
  iniBrowser->setText(result);

}



std::string IniLetter::_getModulusData(const GWEN_CRYPTKEY *key) const {
  GWEN_DB_NODE *n;
  const void *p;
  unsigned int l;
  std::string result;

  n=GWEN_DB_Group_new("keydata");
  if (GWEN_CryptKey_toDb(key, n, 1)) {
    GWEN_DB_Group_free(n);
    return "";
  }

  p=GWEN_DB_GetBinValue(n,
                        "data/n",
                        0,
                        0,0,
                        &l);
  if (!p) {
    GWEN_DB_Group_free(n);
    return "";
  }
  result=std::string((const char*)p, l);
  GWEN_DB_Group_free(n);
  return result;
}



std::string IniLetter::_getExpData(const GWEN_CRYPTKEY *key) const {
  GWEN_DB_NODE *n;
  const void *p;
  unsigned int l;
  std::string result;

  n=GWEN_DB_Group_new("keydata");
  if (GWEN_CryptKey_toDb(key, n, 1)) {
    GWEN_DB_Group_free(n);
    return "";
  }

  p=GWEN_DB_GetBinValue(n,
                        "data/e",
                        0,
                        0,0,
                        &l);
  if (!p) {
    GWEN_DB_Group_free(n);
    return "";
  }
  result=std::string((const char*)p, l);
  GWEN_DB_Group_free(n);
  return result;
}



std::string IniLetter::_getIniLetterModulus(const GWEN_CRYPTKEY *key) const {
  char buffer[256];
  std::string modulus;

  modulus=_getModulusData(key);
  if (modulus.length()<96)
    modulus=std::string(96 - modulus.length(), 0x0) + modulus;
  if (GWEN_Text_ToHex((const char*)modulus.data(), modulus.length(),
		      buffer, sizeof(buffer))==0) {
    return "";
  }
  else
    return buffer;
}



std::string IniLetter::_getIniLetterExponent(const GWEN_CRYPTKEY *key) const {
  char buffer[256];
  std::string expo;

  expo=_getExpData(key);
  if (expo.length()<96)
    expo=std::string(96 - expo.length(), 0x0) + expo;
  if (GWEN_Text_ToHex((const char*)expo.data(), expo.length(),
                      buffer, sizeof(buffer))==0) {
    return "";
  }
  else
    return buffer;
}



std::string IniLetter::_getIniLetterHash(const GWEN_CRYPTKEY *key) const {
  std::string expo;
  std::string modulus;
  std::string result;
  char buffer[64];

  expo=_getExpData(key);
  modulus=_getModulusData(key);
  result = std::string(128 - expo.length(), 0x0) + expo;
  result += std::string(128 - modulus.length(), 0x0) + modulus;
  result = _ripe(result);
  if (GWEN_Text_ToHex((const char*)result.data(), result.length(),
		      buffer, sizeof(buffer))==0) {
    return "";
  }
  else
    return buffer;
}




std::string IniLetter::_ripe(const std::string &src) const {
  std::string result;
  char buffer[32];
  unsigned int bsize;

  /* hash data */
  DBG_DEBUG(0, "Hash data");
  bsize=sizeof(buffer);
  if (GWEN_MD_Hash("RMD160",
                   (const char*)src.data(),
                   src.length(),
                   buffer,
                   &bsize)) {
    DBG_ERROR(0, "Could not hash");
    return "";
  }

  result=std::string(buffer, bsize);
  return result;
}



std::string IniLetter::_dumpHexString(const std::string &s, int size) {
  std::string result;
  unsigned int pos;

  result+="   ";
  for (pos=0; pos<s.length(); pos++) {
    if ((pos%size)==0)
      result+="<br>";
    else if ((pos & 1)==0)
      result+=" ";
    result+=s.at(pos);
  } // for
  result+="<br>";
  return result;
}






