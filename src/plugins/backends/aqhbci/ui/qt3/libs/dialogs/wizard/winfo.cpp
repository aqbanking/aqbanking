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

#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <unistd.h>




WizardInfo::WizardInfo(AB_PROVIDER *pro)
:_provider(pro)
,_user(0)
,_token(0)
,_context(0)
,_country(280)
,_cryptMode(AH_CryptMode_None)
,_port(3000)
,_flags(0)
,_userFlags(0) {


}



WizardInfo::~WizardInfo() {
}



AB_PROVIDER *WizardInfo::getProvider() const {
  return _provider;
}



GWEN_CRYPT_TOKEN *WizardInfo::getToken() const {
  return _token;
}



void WizardInfo::setToken(GWEN_CRYPT_TOKEN *ct) {
  if (_token) {
    if (ct) {
      DBG_ERROR(0, "Overwriting existing token!")
    }
  }
  _token=ct;
}



AH_CRYPT_MODE WizardInfo::getCryptMode() const {
  return _cryptMode;
}



void WizardInfo::setCryptMode(AH_CRYPT_MODE cm) {
  _cryptMode=cm;
}



uint32_t WizardInfo::getContext() const {
  return _context;
}



void WizardInfo::setContext(uint32_t i) {
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



const std::string &WizardInfo::getPeerId() const {
  return _peerId;
}



void WizardInfo::setPeerId(const std::string &s) {
  _peerId=s;
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



const std::string &WizardInfo::getHttpVersion() const {
  return _httpVersion;
}



void WizardInfo::setHttpVersion(const std::string &s) {
  _httpVersion=s;
}



const std::string &WizardInfo::getMediumType() const {
  return _mediumType;
}



void WizardInfo::setMediumType(const std::string &s) {
  _mediumType=s;
}



const std::string &WizardInfo::getMediumName() const {
  return _mediumName;
}



void WizardInfo::setMediumName(const std::string &s) {
  _mediumName=s;
}



uint32_t WizardInfo::getFlags() const {
  return _flags;
}



void WizardInfo::setFlags(uint32_t fl) {
  _flags=fl;
}



void WizardInfo::addFlags(uint32_t fl) {
  _flags|=fl;
}



void WizardInfo::subFlags(uint32_t fl) {
  _flags&=~fl;
}



uint32_t WizardInfo::getUserFlags() const {
  return _userFlags;
}



void WizardInfo::setUserFlags(uint32_t fl) {
  _userFlags=fl;
}



void WizardInfo::addUserFlags(uint32_t fl) {
  _userFlags|=fl;
}



void WizardInfo::subUserFlags(uint32_t fl) {
  _userFlags&=~fl;
}



void WizardInfo::releaseData() {
  // handle user
  if (_user) {

    if (_flags & WIZARDINFO_FLAGS_USER_CREATED) {
      /* user created, so by removing the user we also remove all other
       * objects below it */
      AB_Banking_DeleteUser(AB_Provider_GetBanking(_provider), _user);
      DBG_INFO(0, "Removing user and all subordinate objects");
      _flags&=~WIZARDINFO_FLAGS_USER_CREATED;
      _user=0;
    } // if _userCreated
  } // if user

  if (_token && (_flags & WIZARDINFO_FLAGS_MEDIUM_CREATED)) {
    DBG_INFO(0, "Deleting medium");
    AB_Banking_ClearCryptTokenList(AB_Provider_GetBanking(_provider), 0);
    _flags&=~WIZARDINFO_FLAGS_MEDIUM_CREATED;
    _token=NULL;
  }

  if (!_mediumName.empty() &&
      (_flags & WIZARDINFO_FLAGS_MEDIUM_FILE_CREATED)) {
    DBG_INFO(0, "Deleting medium file");
    unlink(_mediumName.c_str());
  }

}











