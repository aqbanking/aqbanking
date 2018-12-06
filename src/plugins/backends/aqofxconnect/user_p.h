/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
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

  AB_USER_READFROMDB_FN readFromDbFn;
  AB_USER_WRITETODB_FN writeToDbFn;
};

static void GWENHYWFAR_CB AO_User_freeData(void *bp, void *p);

static void AO_User__ReadDb(AB_USER *u, GWEN_DB_NODE *db);
static void AO_User__WriteDb(const AB_USER *u, GWEN_DB_NODE *db);



static int AO_User_ReadFromDb(AB_USER *u, GWEN_DB_NODE *db);
static int AO_User_WriteToDb(const AB_USER *u, GWEN_DB_NODE *db);


#endif
