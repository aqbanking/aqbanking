/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Apr 05 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
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


GWEN_LIST_FUNCTIONS(AB_ACCOUNT_STATUS,AB_AccountStatus)
GWEN_LIST2_FUNCTIONS(AB_ACCOUNT_STATUS,AB_AccountStatus)


AB_ACCOUNT_STATUS *AB_AccountStatus_new(void){
  AB_ACCOUNT_STATUS *as;

  GWEN_NEW_OBJECT(AB_ACCOUNT_STATUS, as);
  GWEN_LIST_INIT(AB_ACCOUNT_STATUS, as);
  return as;
}



AB_ACCOUNT_STATUS *AB_AccountStatus_dup(const AB_ACCOUNT_STATUS *as){
  AB_ACCOUNT_STATUS *newAs;

  GWEN_NEW_OBJECT(AB_ACCOUNT_STATUS, newAs);
  GWEN_LIST_INIT(AB_ACCOUNT_STATUS, newAs);
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
  uint32_t i;
  GWEN_DB_NODE *tdb;

  GWEN_NEW_OBJECT(AB_ACCOUNT_STATUS, as);
  GWEN_LIST_INIT(AB_ACCOUNT_STATUS, as);
  i=GWEN_DB_GetIntValue(db, "time", 0, 0);
  if (i)
    as->time=GWEN_Time_fromSeconds(i);
  tdb=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bankLine");
  if (tdb)
    as->bankLine=AB_Value_fromDb(tdb);
  tdb=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "disposable");
  if (tdb)
    as->disposable=AB_Value_fromDb(tdb);
  tdb=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "disposed");
  if (tdb)
    as->disposed=AB_Value_fromDb(tdb);
  tdb=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bookedBalance");
  if (tdb)
    as->bookedBalance=AB_Balance_fromDb(tdb);
  tdb=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "notedBalance");
  if (tdb)
    as->notedBalance=AB_Balance_fromDb(tdb);

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
    if (AB_Value_toDb(as->bankLine, tdb))
      return -1;
  }

  if (as->disposable) {
    tdb=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "disposable");
    assert(tdb);
    if (AB_Value_toDb(as->disposable, tdb))
      return -1;
  }
  if (as->disposed) {
    tdb=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "disposed");
    assert(tdb);
    if (AB_Value_toDb(as->disposed, tdb))
      return -1;
  }

  if (as->bookedBalance) {
    tdb=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "bookedBalance");
    assert(tdb);
    if (AB_Balance_toDb(as->bookedBalance, tdb))
      return -1;
  }
  if (as->notedBalance) {
    tdb=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "notedBalance");
    assert(tdb);
    if (AB_Balance_toDb(as->notedBalance, tdb))
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
    GWEN_LIST_FINI(AB_ACCOUNT_STATUS, as);
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
  if (t) as->time=GWEN_Time_dup(t);
  else as->time=NULL;
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
  if (v) as->bankLine=AB_Value_dup(v);
  else as->bankLine=NULL;
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
  if (v) as->disposable=AB_Value_dup(v);
  else as->disposable=NULL;
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
  if (v) as->disposed=AB_Value_dup(v);
  else as->disposed=NULL;
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
  if (b) as->bookedBalance=AB_Balance_dup(b);
  else as->bookedBalance=NULL;
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
  if (b) as->notedBalance=AB_Balance_dup(b);
  else as->notedBalance=NULL;
}



AB_ACCOUNT_STATUS_LIST*
AB_AccountStatus_List_dup(const AB_ACCOUNT_STATUS_LIST *asl) {
  if (asl) {
    AB_ACCOUNT_STATUS_LIST *nl;
    AB_ACCOUNT_STATUS *e;

    nl=AB_AccountStatus_List_new();
    e=AB_AccountStatus_List_First(asl);
    while(e) {
      AB_ACCOUNT_STATUS *ne;

      ne=AB_AccountStatus_dup(e);
      assert(ne);
      AB_AccountStatus_List_Add(ne, nl);
      e=AB_AccountStatus_List_Next(e);
    } /* while (e) */
    return nl;
  }
  else
    return 0;
}



AB_ACCOUNT_STATUS *AB_AccountStatus_List2__freeAll_cb(AB_ACCOUNT_STATUS *as,
                                                      void *user_data) {
  AB_AccountStatus_free(as);
  return 0;
}



void AB_AccountStatus_List2_freeAll(AB_ACCOUNT_STATUS_LIST2 *asl) {
  if (asl) {
    AB_AccountStatus_List2_ForEach(asl, AB_AccountStatus_List2__freeAll_cb,
                                   0);
    AB_AccountStatus_List2_free(asl);
  }
}









