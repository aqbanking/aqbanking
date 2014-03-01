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


#include "balance_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif



AB_BALANCE *AB_Balance_new(const AB_VALUE *v, const GWEN_TIME *t){
  AB_BALANCE *b;

  GWEN_NEW_OBJECT(AB_BALANCE, b);
  if (v)
    b->value=AB_Value_dup(v);
  if (t)
    b->time=GWEN_Time_dup(t);
  return b;
}



AB_BALANCE *AB_Balance_dup(const AB_BALANCE *b){
  AB_BALANCE *newB;

  newB=AB_Balance_new(b->value, b->time);
  return newB;
}



AB_BALANCE *AB_Balance_fromDb(GWEN_DB_NODE *db){
  AB_BALANCE *b;
  GWEN_TIME *t;
  AB_VALUE *v;
  GWEN_DB_NODE *tdb;
  uint32_t i;

  tdb=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "value");
  if (tdb)
    v=AB_Value_fromDb(tdb);
  else
    v=0;
  i=GWEN_DB_GetIntValue(db, "time", 0, 0);
  if (i)
    t=GWEN_Time_fromSeconds(i);
  else
    t=0;

  b=AB_Balance_new(v, t);
  AB_Value_free(v);
  GWEN_Time_free(t);
  return b;
}



int AB_Balance_toDb(const AB_BALANCE *b, GWEN_DB_NODE *db){
  if (b->value) {
    GWEN_DB_NODE *tdb;

    tdb=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "value");
    assert(tdb);
    if (AB_Value_toDb(b->value, tdb))
      return -1;
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN, "No value");
  }
  if (b->time)
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "time", GWEN_Time_Seconds(b->time));
  return 0;
}



void AB_Balance_free(AB_BALANCE *b){
  if (b) {
    GWEN_Time_free(b->time);
    AB_Value_free(b->value);
    GWEN_FREE_OBJECT(b);
  }
}



const AB_VALUE *AB_Balance_GetValue(const AB_BALANCE *b){
  assert(b);
  return b->value;
}



void AB_Balance_SetValue(AB_BALANCE *b, const AB_VALUE *v) {
  assert(b);
  assert(v);
  AB_Value_free(b->value);
  b->value=AB_Value_dup(v);
}



const GWEN_TIME *AB_Balance_GetTime(const AB_BALANCE *b){
  assert(b);
  return b->time;
}





