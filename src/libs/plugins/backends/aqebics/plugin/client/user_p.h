/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef EBC_CLIENT_USER_P_H
#define EBC_CLIENT_USER_P_H

#include "user_l.h"


typedef struct EBC_USER EBC_USER;
struct EBC_USER {
  char *serverUrl;

  char *peerId;

  char *tokenType;
  char *tokenName;
  uint32_t tokenContextId;
  char *protoVersion;
  char *signVersion;
  char *cryptVersion;
  char *authVersion;
  char *systemId;

  EBC_USER_STATUS status;

  int httpVMajor;
  int httpVMinor;
  char *httpUserAgent;
  char *httpContentType;

  uint32_t flags;

  AB_USER_READFROMDB_FN readFromDbFn;
  AB_USER_WRITETODB_FN writeToDbFn;
};

static void GWENHYWFAR_CB EBC_User_freeData(void *bp, void *p);

static void EBC_User__ReadDb(AB_USER *u, GWEN_DB_NODE *db);
static void EBC_User__WriteDb(const AB_USER *u, GWEN_DB_NODE *db);


int EBC_User_ReadFromDb(AB_USER *u, GWEN_DB_NODE *db);
static int EBC_User_WriteToDb(const AB_USER *u, GWEN_DB_NODE *db);


#endif /* EBC_CLIENT_USER_P_H */

