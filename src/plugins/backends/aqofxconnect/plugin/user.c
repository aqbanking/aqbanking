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


GWEN_LIST_FUNCTIONS(AO_USER, AO_User)


AO_USER *AO_User_new(AO_BANK *b, const char *userId){
  AO_USER *u;

  GWEN_NEW_OBJECT(AO_USER, u);
  u->bank=b;
  if (userId)
    u->userId=strdup(userId);

  return u;
}



void AO_User_free(AO_USER *u){
  if (u) {
    free(u->userName);
    free(u->userId);
    GWEN_FREE_OBJECT(u);
  }
}



AO_BANK *AO_User_GetBank(const AO_USER *u){
  assert(u);
  return u->bank;
}



void AO_User_SetBank(AO_USER *u, AO_BANK *b) {
  assert(u);
  u->bank=b;
}




const char *AO_User_GetUserId(const AO_USER *u){
  assert(u);
  return u->userId;
}



void AO_User_SetUserId(AO_USER *u, const char *s){
  assert(u);
  free(u->userId);
  if (s) u->userId=strdup(s);
  else u->userId=0;
}



const char *AO_User_GetUserName(const AO_USER *u){
  assert(u);
  return u->userName;
}



void AO_User_SetUserName(AO_USER *u, const char *s){
  assert(u);
  free(u->userName);
  if (s) u->userName=strdup(s);
  else u->userName=0;
}



AO_USER *AO_User_fromDb(AO_BANK *b, GWEN_DB_NODE *db){
  AO_USER *u;
  const char *s;

  s=GWEN_DB_GetCharValue(db, "userId", 0, 0);
  assert(s);
  u=AO_User_new(b, s);
  AO_User_SetUserName(u, GWEN_DB_GetCharValue(db, "userName", 0, 0));

  return u;
}



int AO_User_toDb(const AO_USER *u, GWEN_DB_NODE *db) {
  assert(u);
  assert(db);
  if (u->userId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "userId", u->userId);
  if (u->userName)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "userName", u->userName);

  return 0;
}






