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

#include "imexporter_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT_FUNCTIONS(AB_IMEXPORTER)
GWEN_LIST_FUNCTIONS(AB_IMEXPORTER, AB_ImExporter)

GWEN_LIST_FUNCTIONS(AB_IMEXPORTER_DAY, AB_ImExporterDay)



AB_IMEXPORTER *AB_ImExporter_new(AB_BANKING *ab, const char *name){
  AB_IMEXPORTER *ie;

  assert(ab);
  assert(name);
  GWEN_NEW_OBJECT(AB_IMEXPORTER, ie);
  GWEN_LIST_INIT(AB_IMEXPORTER, ie);
  GWEN_INHERIT_INIT(AB_IMEXPORTER, ie);

  ie->banking=ab;
  ie->name=strdup(name);

  return ie;
}


void AB_ImExporter_free(AB_IMEXPORTER *ie){
  if (ie) {
    GWEN_INHERIT_FINI(AB_IMEXPORTER, ie);
    free(ie->name);
    GWEN_LIST_FINI(AB_IMEXPORTER, ie);
    GWEN_FREE_OBJECT(ie);
  }
}



int AB_ImExporter_Import(AB_IMEXPORTER *ie,
                         AB_IMEXPORTER_CONTEXT *ctx,
                         GWEN_BUFFEREDIO *bio,
                         GWEN_DB_NODE *params){
  assert(ie);
  if (ie->importFn)
    return ie->importFn(ie, ctx, bio, params);
  else
    return AB_ERROR_NOT_SUPPORTED;
}



void AB_ImExporter_SetImportFn(AB_IMEXPORTER *ie,
                               AB_IMEXPORTER_IMPORT_FN f){
  assert(ie);
  ie->importFn=f;
}



AB_BANKING *AB_ImExporter_GetBanking(const AB_IMEXPORTER *ie){
  assert(ie);
  return ie->banking;
}



const char *AB_ImExporter_GetName(const AB_IMEXPORTER *ie){
  assert(ie);
  return ie->name;
}



void AB_ImExporter_SetLibLoader(AB_IMEXPORTER *ie, GWEN_LIBLOADER *ll) {
  assert(ie);
  ie->libLoader=ll;
}










AB_IMEXPORTER_CONTEXT *AB_ImExporterContext_new(){
  AB_IMEXPORTER_CONTEXT *iec;

  GWEN_NEW_OBJECT(AB_IMEXPORTER_CONTEXT, iec);
  iec->days=AB_ImExporterDay_List_new();
  iec->accounts=AB_Account_List_new();

  return iec;
}



void AB_ImExporterContext_free(AB_IMEXPORTER_CONTEXT *iec){
  if (iec) {
    AB_ImExporterDay_List_free(iec->days);
    AB_Account_List_free(iec->accounts);
    GWEN_FREE_OBJECT(iec);
  }
}



AB_IMEXPORTER_DAY*
AB_ImExporterContext_GetFirstDay(AB_IMEXPORTER_CONTEXT *iec){
  AB_IMEXPORTER_DAY *ied;

  assert(iec);
  ied=AB_ImExporterDay_List_First(iec->days);
  if (ied) {
    iec->nextDay=AB_ImExporterDay_List_Next(ied);
    AB_ImExporterDay_List_Del(ied);
    return ied;
  }
  iec->nextDay=0;
  return 0;
}



AB_IMEXPORTER_DAY*
AB_ImExporterContext_GetNextDay(AB_IMEXPORTER_CONTEXT *iec){
  AB_IMEXPORTER_DAY *ied;

  assert(iec);
  ied=iec->nextDay;
  if (ied) {
    iec->nextDay=AB_ImExporterDay_List_Next(ied);
    AB_ImExporterDay_List_Del(ied);
    return ied;
  }
  iec->nextDay=0;
  return 0;
}



AB_ACCOUNT *AB_ImExporterContext_GetFirstAccount(AB_IMEXPORTER_CONTEXT *iec){
  AB_ACCOUNT *a;

  assert(iec);
  a=AB_Account_List_First(iec->accounts);
  if (a) {
    iec->nextAccount=AB_Account_List_Next(a);
    AB_Account_List_Del(a);
    return a;
  }
  iec->nextAccount=0;
  return 0;
}



AB_ACCOUNT *AB_ImExporterContext_GetNextAccount(AB_IMEXPORTER_CONTEXT *iec){
  AB_ACCOUNT *a;

  assert(iec);
  a=iec->nextAccount;
  if (a) {
    iec->nextAccount=AB_Account_List_Next(a);
    AB_Account_List_Del(a);
    return a;
  }
  iec->nextAccount=0;
  return 0;
}



AB_IMEXPORTER_DAY *AB_ImExporterContext_GetDay(AB_IMEXPORTER_CONTEXT *iec,
                                               const GWEN_TIME *ti,
                                               int crea){
  AB_IMEXPORTER_DAY *ied;
  int year, month, day;

  assert(iec);
  assert(ti);

  if (GWEN_Time_GetBrokenDownUtcDate(ti, &day, &month, &year)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad time argument, aborting");
    abort();
  }

  ied=AB_ImExporterDay_List_First(iec->days);
  while(ied) {
    int dyear, dmonth, dday;

    if (GWEN_Time_GetBrokenDownUtcDate(ied->date,
                                       &dday, &dmonth, &dyear)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad time in day, aborting");
      abort();
    }
    if (year==dyear && month==dmonth && day==dday)
      return ied;

    ied=AB_ImExporterDay_List_Next(ied);
  }
  if (crea) {
    ied=AB_ImExporterDay_new(ti);
    AB_ImExporterDay_List_Add(ied, iec->days);
  }

  return ied;
}



void AB_ImExporterContext_AddAccount(AB_IMEXPORTER_CONTEXT *iec,
                                     AB_ACCOUNT *a){
  assert(iec);
  AB_Account_List_Add(a, iec->accounts);
}



void AB_ImExporterContext_AddTransaction(AB_IMEXPORTER_CONTEXT *iec,
                                         AB_TRANSACTION *t){
  AB_IMEXPORTER_DAY *ied;
  const GWEN_TIME *ti;

  ti=AB_Transaction_GetDate(t);
  if (!ti)
    ti=AB_Transaction_GetValutaDate(t);
  assert(ti);
  /* get day, create if necessary */
  ied=AB_ImExporterContext_GetDay(iec, ti, 1);
  assert(ied);
  AB_ImExporterDay_AddTransaction(ied, t);
}







AB_IMEXPORTER_DAY *AB_ImExporterDay_new(const GWEN_TIME *ti){
  AB_IMEXPORTER_DAY *ied;

  GWEN_NEW_OBJECT(AB_IMEXPORTER_DAY, ied);
  ied->date=GWEN_Time_dup(ti);
  ied->transactions=AB_Transaction_List_new();

  return ied;
}



void AB_ImExporterDay_free(AB_IMEXPORTER_DAY *ied){
  if (ied) {
    AB_Transaction_List_free(ied->transactions);
    GWEN_Time_free(ied->date);
    GWEN_FREE_OBJECT(ied);
  }
}



const GWEN_TIME *AB_ImExporterDay_GetDate(const AB_IMEXPORTER_DAY *ied){
  assert(ied);
  return ied->date;
}



AB_TRANSACTION *AB_ImExporterDay_GetFirstTransaction(AB_IMEXPORTER_DAY *ied){
  AB_TRANSACTION *t;

  assert(ied);
  t=AB_Transaction_List_First(ied->transactions);
  if (t) {
    ied->nextTransaction=AB_Transaction_List_Next(t);
    AB_Transaction_List_Del(t);
    return t;
  }
  ied->nextTransaction=0;
  return 0;
}



AB_TRANSACTION *AB_ImExporterDay_GetNextTransaction(AB_IMEXPORTER_DAY *ied){
  AB_TRANSACTION *t;

  assert(ied);
  t=ied->nextTransaction;
  if (t) {
    ied->nextTransaction=AB_Transaction_List_Next(t);
    AB_Transaction_List_Del(t);
    return t;
  }
  ied->nextTransaction=0;
  return 0;
}



void AB_ImExporterDay_AddTransaction(AB_IMEXPORTER_DAY *ied,
                                     AB_TRANSACTION *t){
  assert(ied);
  assert(t);
  AB_Transaction_List_Add(t, ied->transactions);
}





















