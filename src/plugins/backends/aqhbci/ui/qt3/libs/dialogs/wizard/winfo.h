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


#ifndef AQHBCI_WINFO_H
#define AQHBCI_WINFO_H

#include <aqhbci/hbci.h>
#include <aqhbci/medium.h>
#include <gwenhywfar/types.h>
#include <string>


#define WIZARDINFO_FLAGS_USER_CREATED         0x00000001
#define WIZARDINFO_FLAGS_MEDIUM_CREATED       0x00000002
#define WIZARDINFO_FLAGS_MEDIUM_ADDED         0x00000004
#define WIZARDINFO_FLAGS_MEDIUM_FILE_CREATED  0x00000008


class WizardInfo {
private:
  AH_HBCI *_hbci;
  AB_USER *_user;
  AH_MEDIUM *_medium;

  int _context;

  int _country;
  std::string _bankId;
  std::string _userId;
  std::string _userName;
  std::string _customerId;
  std::string _server;
  std::string _mediumName;
  AH_CRYPT_MODE _cryptMode;
  int _port;

  GWEN_TYPE_UINT32 _flags;

public:
  WizardInfo(AH_HBCI *hbci);
  ~WizardInfo();

  AH_HBCI *getHbci() const ;

  AH_MEDIUM *getMedium() const;
  void setMedium(AH_MEDIUM *m);

  AH_CRYPT_MODE getCryptMode() const;
  void setCryptMode(AH_CRYPT_MODE cm);

  int getContext() const;
  void setContext(int i);

  AB_USER *getUser() const;
  void setUser(AB_USER *u);

  int getCountry() const;
  void setCountry(int i);

  const std::string &getBankId() const;
  void setBankId(const std::string &s);

  const std::string &getUserId() const;
  void setUserId(const std::string &s);

  const std::string &getUserName() const;
  void setUserName(const std::string &s);

  const std::string &getCustomerId() const;
  void setCustomerId(const std::string &s);

  const std::string &getServer() const;
  void setServer(const std::string &s);

  int getPort() const;
  void setPort(int i);

  const std::string &getMediumName() const;
  void setMediumName(const std::string &s);

  GWEN_TYPE_UINT32 getFlags() const;
  void setFlags(GWEN_TYPE_UINT32 fl);
  void addFlags(GWEN_TYPE_UINT32 fl);
  void subFlags(GWEN_TYPE_UINT32 fl);

  void releaseData();

};



#endif
