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

#include "winfo.h"
#include <aqhbci/provider.h>

#include <gwenhywfar/debug.h>
#include <unistd.h>




WizardInfo::WizardInfo(AB_PROVIDER *pro)
:_provider(pro)
,_user(0)
,_medium(0)
,_context(0)
,_country(280)
,_cryptMode(AH_CryptMode_None)
,_port(3000)
,_flags(0) {


}



WizardInfo::~WizardInfo() {
}



AB_PROVIDER *WizardInfo::getProvider() const {
  return _provider;
}



AH_MEDIUM *WizardInfo::getMedium() const {
  return _medium;
}



void WizardInfo::setMedium(AH_MEDIUM *m) {
  if (_medium) {
    if (m) {
      DBG_ERROR(0, "Overwriting existing medium!")
    }
    else {
      DBG_ERROR(0, "Resetting medium")
    }
  }
  _medium=m;
}



AH_CRYPT_MODE WizardInfo::getCryptMode() const {
  return _cryptMode;
}



void WizardInfo::setCryptMode(AH_CRYPT_MODE cm) {
  _cryptMode=cm;
}



int WizardInfo::getContext() const {
  return _context;
}



void WizardInfo::setContext(int i) {
  _context=i;
}



AB_USER *WizardInfo::getUser() const {
  return _user;
}



void WizardInfo::setUser(AB_USER *u) {
  _user=u;
}



int WizardInfo::getCountry() const {
  return _country;
}



void WizardInfo::setCountry(int i) {
  _country=i;
}



const std::string &WizardInfo::getBankId() const {
  return _bankId;
}



void WizardInfo::setBankId(const std::string &s) {
  _bankId=s;
}



const std::string &WizardInfo::getUserId() const {
  return _userId;
}



void WizardInfo::setUserId(const std::string &s) {
  _userId=s;
}



const std::string &WizardInfo::getUserName() const {
  return _userName;
}



void WizardInfo::setUserName(const std::string &s) {
  _userName=s;
}



const std::string &WizardInfo::getCustomerId() const {
  return _customerId;
}



void WizardInfo::setCustomerId(const std::string &s) {
  _customerId=s;
}



const std::string &WizardInfo::getServer() const {
  return _server;
}



void WizardInfo::setServer(const std::string &s) {
  _server=s;
}



int WizardInfo::getPort() const {
  return _port;
}



void WizardInfo::setPort(int i) {
  _port=i;
}



const std::string &WizardInfo::getMediumName() const {
  return _mediumName;
}



void WizardInfo::setMediumName(const std::string &s) {
  _mediumName=s;
}



GWEN_TYPE_UINT32 WizardInfo::getFlags() const {
  return _flags;
}



void WizardInfo::setFlags(GWEN_TYPE_UINT32 fl) {
  _flags=fl;
}



void WizardInfo::addFlags(GWEN_TYPE_UINT32 fl) {
  _flags|=fl;
}



void WizardInfo::subFlags(GWEN_TYPE_UINT32 fl) {
  _flags&=~fl;
}



void WizardInfo::releaseData() {
  // handle user
  if (_user) {
    if (_flags & WIZARDINFO_FLAGS_USER_CREATED) {
      /* user created, so by removing the user we also remove all other
       * objects below it */
      DBG_INFO(0, "Removing user and all subordinate objects");
      _flags&=~WIZARDINFO_FLAGS_USER_CREATED;
      AB_User_free(_user);
      _user=0;
    } // if _userCreated
  } // if user

  if (_medium && (_flags & WIZARDINFO_FLAGS_MEDIUM_CREATED)) {
    if (_flags & WIZARDINFO_FLAGS_MEDIUM_ADDED) {
      DBG_INFO(0, "Unlisting medium");
      AH_Provider_RemoveMedium(_provider, _medium);
      _flags&=~WIZARDINFO_FLAGS_MEDIUM_ADDED;
    }
    DBG_INFO(0, "Deleting medium");
    AH_Medium_free(_medium);
    _flags&=~WIZARDINFO_FLAGS_MEDIUM_CREATED;
    _medium=0;
  }

  if (!_mediumName.empty() &&
      (_flags & WIZARDINFO_FLAGS_MEDIUM_FILE_CREATED)) {
    DBG_INFO(0, "Deleting medium file");
    unlink(_mediumName.c_str());
  }

}











