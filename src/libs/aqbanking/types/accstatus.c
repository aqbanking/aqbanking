/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Apr 05 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "accstatus_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif



AB_ACCOUNT_STATUS *AB_AccountStatus_new(){
  AB_ACCOUNT_STATUS *as;

  GWEN_NEW_OBJECT(AB_ACCOUNT_STATUS, as);
  return as;
}



AB_ACCOUNT_STATUS *AB_AccountStatus_dup(const AB_ACCOUNT_STATUS *as){
  AB_ACCOUNT_STATUS *newAs;

  GWEN_NEW_OBJECT(AB_ACCOUNT_STATUS, newAs);
  if (as->time)
    newAs->time=GWEN_Time_dup(as->time);
  if (as->bankLine)
    newAs->bankLine=AB_Value_dup(as->bankLine);
  if (as->disposable)
    newAs->disposable=AB_Value_dup(as->disposable);
  if (as->disposed)
    newAs->disposed=AB_Value_dup(as->disposed);
  if (as->bookedBalance)
    newAs->bookedBalance=AB_Balance_dup(as->bookedBalance);
  if (as->notedBalance)
    newAs->notedBalance=AB_Balance_dup(as->notedBalance);

  return newAs;
}



AB_ACCOUNT_STATUS *AB_AccountStatus_fromDb(GWEN_DB_NODE *db){
  AB_ACCOUNT_STATUS *as;
  GWEN_TYPE_UINT32 i;
  GWEN_DB_NODE *tdb;

  GWEN_NEW_OBJECT(AB_ACCOUNT_STATUS, as);
  i=GWEN_DB_GetIntValue(db, "time", 0, 0);
  if (i)
    as->time=GWEN_Time_fromSeconds(i);
  tdb=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bankLine");
  if (tdb)
    as->bankLine=AB_Value_FromDb(tdb);
  tdb=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "disposable");
  if (tdb)
    as->disposable=AB_Value_FromDb(tdb);
  tdb=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "disposed");
  if (tdb)
    as->disposed=AB_Value_FromDb(tdb);
  tdb=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bookedBalance");
  if (tdb)
    as->bookedBalance=AB_Balance_FromDb(tdb);
  tdb=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "notedBalance");
  if (tdb)
    as->notedBalance=AB_Balance_FromDb(tdb);

  return as;
}



int AB_AccountStatus_toDb(const AB_ACCOUNT_STATUS *as, GWEN_DB_NODE *db){
  GWEN_DB_NODE *tdb;

  assert(as);
  assert(db);

  if (as->time) {
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "time", GWEN_Time_Seconds(as->time));
  }

  if (as->bankLine) {
    tdb=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "bankLine");
    assert(tdb);
    if (AB_Value_ToDb(as->bankLine, tdb))
      return -1;
  }

  if (as->disposable) {
    tdb=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "disposable");
    assert(tdb);
    if (AB_Value_ToDb(as->disposable, tdb))
      return -1;
  }
  if (as->disposed) {
    tdb=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "disposed");
    assert(tdb);
    if (AB_Value_ToDb(as->disposed, tdb))
      return -1;
  }

  if (as->bookedBalance) {
    tdb=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "bookedBalance");
    assert(tdb);
    if (AB_Balance_ToDb(as->bookedBalance, tdb))
      return -1;
  }
  if (as->notedBalance) {
    tdb=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "notedBalance");
    assert(tdb);
    if (AB_Balance_ToDb(as->notedBalance, tdb))
      return -1;
  }

  return 0;
}



void AB_AccountStatus_free(AB_ACCOUNT_STATUS *as){
  if (as) {
    GWEN_Time_free(as->time);
    AB_Value_free(as->disposable);
    AB_Value_free(as->disposed);
    AB_Value_free(as->bankLine);
    AB_Balance_free(as->bookedBalance);
    AB_Balance_free(as->notedBalance);
    GWEN_FREE_OBJECT(as);
  }
}



const GWEN_TIME*
AB_AccountStatus_GetTime(const AB_ACCOUNT_STATUS *as){
  assert(as);
  return as->time;
}



void AB_AccountStatus_SetTime(AB_ACCOUNT_STATUS *as,
                              const GWEN_TIME *t){
  assert(as);
  GWEN_Time_free(as->time);
  as->time=GWEN_Time_dup(t);
}



const AB_VALUE*
AB_AccountStatus_GetBankLine(const AB_ACCOUNT_STATUS *as){
  assert(as);
  return as->bankLine;
}



void AB_AccountStatus_SetBankLine(AB_ACCOUNT_STATUS *as,
                                  const AB_VALUE *v){
  assert(as);
  AB_Value_free(as->bankLine);
  as->bankLine=AB_Value_dup(v);
}



const AB_VALUE*
AB_AccountStatus_GetDisposable(const AB_ACCOUNT_STATUS *as){
  assert(as);
  return as->disposable;
}



void AB_AccountStatus_SetDisposable(AB_ACCOUNT_STATUS *as,
                                    const AB_VALUE *v){
  assert(as);
  AB_Value_free(as->disposable);
  as->disposable=AB_Value_dup(v);
}



const AB_VALUE*
AB_AccountStatus_GetDisposed(const AB_ACCOUNT_STATUS *as){
  assert(as);
  return as->disposed;
}



void AB_AccountStatus_SetDisposed(AB_ACCOUNT_STATUS *as,
                                  const AB_VALUE *v){
  assert(as);
  AB_Value_free(as->disposed);
  as->disposed=AB_Value_dup(v);
}



const AB_BALANCE*
AB_AccountStatus_GetBookedBalance(const AB_ACCOUNT_STATUS *as){
  assert(as);
  return as->bookedBalance;
}



void AB_AccountStatus_SetBookedBalance(AB_ACCOUNT_STATUS *as,
                                       const AB_BALANCE *b){
  assert(as);
  AB_Balance_free(as->bookedBalance);
  as->bookedBalance=AB_Balance_dup(b);
}



const AB_BALANCE*
AB_AccountStatus_GetNotedBalance(const AB_ACCOUNT_STATUS *as){
  assert(as);
  return as->notedBalance;
}



void AB_AccountStatus_SetNotedBalance(AB_ACCOUNT_STATUS *as,
                                      const AB_BALANCE *b){
  assert(as);
  AB_Balance_free(as->notedBalance);
  as->notedBalance=AB_Balance_dup(b);
}















