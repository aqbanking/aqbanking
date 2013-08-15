/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AO_USER_P_H
#define AO_USER_P_H

#include <aqofxconnect/user.h>


typedef struct AO_USER AO_USER;
struct AO_USER {
  uint32_t flags;

  char *bankName;
  char *brokerId;
  char *org;
  char *fid;
  char *serverAddr;
  char *appId;
  char *appVer;
  char *headerVer;
  char *clientUid;

  char *securityType;

  int httpVMajor;
  int httpVMinor;
  char *httpUserAgent;
};

static void GWENHYWFAR_CB AO_User_FreeData(void *bp, void *p);

static void AO_User_ReadDb(AB_USER *u, GWEN_DB_NODE *db);


#endif
