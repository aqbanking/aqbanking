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

#ifndef AO_BANK_P_H
#define AO_BANK_P_H

#include <aqofxconnect/bank.h>


struct AO_BANK {
  GWEN_LIST_ELEMENT(AO_BANK)
  AB_PROVIDER *provider;
  char *country;
  char *bankId;
  char *bankName;
  char *fid;
  char *org;
  char *brokerId;
  char *serverAddr;
  int serverPort;
  AO_BANK_SERVERTYPE serverType;
  char *httpHost;
  int httpVMajor;
  int httpVMinor;

  AB_ACCOUNT_LIST *accounts;
  AO_USER_LIST *users;
};



#endif
