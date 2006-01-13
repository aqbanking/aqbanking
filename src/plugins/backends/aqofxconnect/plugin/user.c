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

#include "user_p.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



AO_USER_SERVERTYPE AO_User_ServerType_fromString(const char *s) {
  assert(s);
  if (strcasecmp(s, "http")==0)
    return AO_User_ServerTypeHTTP;
  else if (strcasecmp(s, "https")==0)
    return AO_User_ServerTypeHTTPS;
  return AO_User_ServerTypeUnknown;
}



const char *AO_User_ServerType_toString(AO_USER_SERVERTYPE t) {
  switch(t) {
  case AO_User_ServerTypeHTTP:  return "http";
  case AO_User_ServerTypeHTTPS: return "https";
  default:                      return "unknown";
  }
}



void AO_User_Extend(AB_USER *u, AB_PROVIDER *pro) {
}



const char *AO_User_GetBrokerId(const AB_USER *u) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  return GWEN_DB_GetCharValue(db, "brokerId", 0, 0);
}



void AO_User_SetBrokerId(AB_USER *u, const char *s) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "brokerId", s);
  else
    GWEN_DB_DeleteVar(db, "brokerId");
}



const char *AO_User_GetOrg(const AB_USER *u) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  return GWEN_DB_GetCharValue(db, "org", 0, 0);
}



void AO_User_SetOrg(AB_USER *u, const char *s) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "org", s);
  else
    GWEN_DB_DeleteVar(db, "org");
}




const char *AO_User_GetFid(const AB_USER *u) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  return GWEN_DB_GetCharValue(db, "fid", 0, 0);
}



void AO_User_SetFid(AB_USER *u, const char *s) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "fid", s);
  else
    GWEN_DB_DeleteVar(db, "fid");
}



AO_USER_SERVERTYPE AO_User_GetServerType(const AB_USER *u) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  return AO_User_ServerType_fromString(GWEN_DB_GetCharValue(db,
                                                            "serverType", 0,
                                                            "unknown"));
}



void AO_User_SetServerType(AB_USER *u, AO_USER_SERVERTYPE t) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "serverType",
                       AO_User_ServerType_toString(t));
}



const char *AO_User_GetServerAddr(const AB_USER *u) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  return GWEN_DB_GetCharValue(db, "serverAddr", 0, 0);
}



void AO_User_SetServerAddr(AB_USER *u, const char *s) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "serverAddr", s);
  else
    GWEN_DB_DeleteVar(db, "serverAddr");
}



int AO_User_GetServerPort(const AB_USER *u) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  return GWEN_DB_GetIntValue(db, "serverPort", 0, 0);
}



void AO_User_SetServerPort(AB_USER *u, int i) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "serverPort", i);
}



int AO_User_GetHttpVMajor(const AB_USER *u) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  return GWEN_DB_GetIntValue(db, "httpVMajor", 0, 0);
}



void AO_User_SetHttpVMajor(AB_USER *u, int i) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "httpVMajor", i);
}



int AO_User_GetHttpVMinor(const AB_USER *u) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  return GWEN_DB_GetIntValue(db, "httpVMinor", 0, 0);
}



void AO_User_SetHttpVMinor(AB_USER *u, int i) {
  GWEN_DB_NODE *db;

  assert(u);
  db=AB_User_GetProviderData(u);
  assert(db);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "httpVMinor", i);
}









