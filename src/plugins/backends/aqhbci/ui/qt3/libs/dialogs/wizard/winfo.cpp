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

#include <gwenhywfar/debug.h>




WizardInfo::WizardInfo(AH_HBCI *hbci)
:_hbci(hbci)
,_bank(0)
,_user(0)
,_customer(0)
,_medium(0)
,_context(0)
,_country(280)
,_port(3000)
,_flags(0) {


}



WizardInfo::~WizardInfo() {
}



AH_HBCI *WizardInfo::getHbci() const {
  return _hbci;
}



AH_MEDIUM *WizardInfo::getMedium() const {
  return _medium;
}



void WizardInfo::setMedium(AH_MEDIUM *m) {
  _medium=m;
}



int WizardInfo::getContext() const {
  return _context;
}



void WizardInfo::setContext(int i) {
  _context=i;
}



AH_BANK *WizardInfo::getBank() const {
  return _bank;
}



void WizardInfo::setBank(AH_BANK *b) {
  _bank=b;
}



AH_USER *WizardInfo::getUser() const {
  return _user;
}



void WizardInfo::setUser(AH_USER *u) {
  _user=u;
}



AH_CUSTOMER *WizardInfo::getCustomer() const {
  return _customer;
}



void WizardInfo::setCustomer(AH_CUSTOMER *cu) {
  _customer=cu;
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
  // handle bank, user, customer
  if (_bank) {
    if (_flags & WIZARDINFO_FLAGS_BANK_CREATED) {
      /* bank created, so by removing the bank we also remove all other
       * objects below it */
      DBG_INFO(0, "Removing bank and all subordinate objects");
      AH_HBCI_RemoveBank(_hbci, _bank);
      _flags&=~WIZARDINFO_FLAGS_BANK_CREATED;
      AH_Bank_free(_bank);
      _bank=0;
    } // if bank created
    else {
      if (_user) {
        if (_flags & WIZARDINFO_FLAGS_USER_CREATED) {
          /* user created, so by removing the user we also remove all other
           * objects below it */
          DBG_INFO(0, "Removing user and all subordinate objects");
          AH_Bank_RemoveUser(_bank, _user);
          _flags&=~WIZARDINFO_FLAGS_USER_CREATED;
          AH_User_free(_user);
          _user=0;
        } // if _userCreated
        else {
          if (_customer) {
            if (_flags & WIZARDINFO_FLAGS_CUST_CREATED) {
              DBG_INFO(0, "Removing customer");
              AH_User_RemoveCustomer(_user, _customer);
              _flags&=~WIZARDINFO_FLAGS_CUST_CREATED;
              AH_Customer_free(_customer);
              _customer=0;
            } // if customer created
          } // if customer
        } // if user not created
      } // if user
    } // if bank not created
  } // if bank

  if (_medium && (_flags & WIZARDINFO_FLAGS_MEDIUM_CREATED)) {
    if (_flags & WIZARDINFO_FLAGS_MEDIUM_ADDED) {
      AH_HBCI_RemoveMedium(_hbci, _medium);
      _flags&=~WIZARDINFO_FLAGS_MEDIUM_ADDED;
    }
    AH_Medium_free(_medium);
    _flags&=~WIZARDINFO_FLAGS_MEDIUM_CREATED;
    _medium=0;
  }
}











