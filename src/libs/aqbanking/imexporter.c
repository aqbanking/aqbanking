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
  iec->transactions=AB_Transaction_List_new();
  iec->accounts=AB_Account_List_new();

  return iec;
}



void AB_ImExporterContext_free(AB_IMEXPORTER_CONTEXT *iec){
  if (iec) {
    AB_Transaction_List_free(iec->transactions);
    AB_Account_List_free(iec->accounts);
    GWEN_FREE_OBJECT(iec);
  }
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



void AB_ImExporterContext_AddAccount(AB_IMEXPORTER_CONTEXT *iec,
                                     AB_ACCOUNT *a){
  assert(iec);
  assert(a);
  AB_Account_List_Add(a, iec->accounts);
}



void AB_ImExporterContext_AddTransaction(AB_IMEXPORTER_CONTEXT *iec,
                                         AB_TRANSACTION *t){
  assert(iec);
  assert(t);
  AB_Transaction_List_Add(t, iec->transactions);
}



AB_TRANSACTION*
AB_ImExporterContext_GetFirstTransaction(AB_IMEXPORTER_CONTEXT *iec){
  AB_TRANSACTION *t;

  assert(iec);
  t=AB_Transaction_List_First(iec->transactions);
  if (t) {
    iec->nextTransaction=AB_Transaction_List_Next(t);
    AB_Transaction_List_Del(t);
    return t;
  }
  iec->nextTransaction=0;
  return 0;
}



AB_TRANSACTION*
AB_ImExporterContext_GetNextTransaction(AB_IMEXPORTER_CONTEXT *iec){
  AB_TRANSACTION *t;

  assert(iec);
  t=iec->nextTransaction;
  if (t) {
    iec->nextTransaction=AB_Transaction_List_Next(t);
    AB_Transaction_List_Del(t);
    return t;
  }
  iec->nextTransaction=0;
  return 0;
}





















