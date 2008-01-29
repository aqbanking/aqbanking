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

#include <aqhbci/provider.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/ct.h>
#include <string>


#define WIZARDINFO_FLAGS_USER_CREATED         0x00000001
#define WIZARDINFO_FLAGS_MEDIUM_CREATED       0x00000002
#define WIZARDINFO_FLAGS_MEDIUM_ADDED         0x00000004
#define WIZARDINFO_FLAGS_MEDIUM_FILE_CREATED  0x00000008


class WizardInfo {
private:
  AB_PROVIDER *_provider;
  AB_USER *_user;
  GWEN_CRYPT_TOKEN *_token;

  uint32_t _context;

  int _country;
  std::string _bankId;
  std::string _userId;
  std::string _userName;
  std::string _customerId;
  std::string _peerId;
  std::string _server;
  std::string _mediumType;
  std::string _mediumName;
  std::string _httpVersion;
  AH_CRYPT_MODE _cryptMode;
  int _port;

  uint32_t _flags;

  uint32_t _userFlags;

public:
  WizardInfo(AB_PROVIDER *pro);
  ~WizardInfo();

  AB_PROVIDER *getProvider() const ;

  GWEN_CRYPT_TOKEN *getToken() const;
  void setToken(GWEN_CRYPT_TOKEN *ct);

  AH_CRYPT_MODE getCryptMode() const;
  void setCryptMode(AH_CRYPT_MODE cm);

  uint32_t getContext() const;
  void setContext(uint32_t i);

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

  const std::string &getPeerId() const;
  void setPeerId(const std::string &s);

  const std::string &getServer() const;
  void setServer(const std::string &s);

  int getPort() const;
  void setPort(int i);

  const std::string &getHttpVersion() const;
  void setHttpVersion(const std::string &s);

  const std::string &getMediumType() const;
  void setMediumType(const std::string &s);

  const std::string &getMediumName() const;
  void setMediumName(const std::string &s);

  uint32_t getFlags() const;
  void setFlags(uint32_t fl);
  void addFlags(uint32_t fl);
  void subFlags(uint32_t fl);

  uint32_t getUserFlags() const;
  void setUserFlags(uint32_t fl);
  void addUserFlags(uint32_t fl);
  void subUserFlags(uint32_t fl);

  void releaseData();

};



#endif
