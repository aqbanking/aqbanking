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

#include "account_p.h"


#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT(AB_ACCOUNT, AD_ACCOUNT)



void AD_Account_Extend(AB_ACCOUNT *a, AB_PROVIDER *pro,
                       AB_PROVIDER_EXTEND_MODE em){
  AD_ACCOUNT *ad;

  if (em==AB_ProviderExtendMode_Create ||
      em==AB_ProviderExtendMode_Extend) {
    GWEN_NEW_OBJECT(AD_ACCOUNT, ad);
    GWEN_INHERIT_SETDATA(AB_ACCOUNT, AD_ACCOUNT, a, ad, AD_Account_FreeData);
  }
}



void AD_Account_FreeData(void *bp, void *p) {
  AD_ACCOUNT *ad;

  ad=(AD_ACCOUNT*)p;
  GWEN_FREE_OBJECT(ad);
}



int AD_Account_GetMaxTransfersPerJob(const AB_ACCOUNT *a){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  return GWEN_DB_GetIntValue(db, "maxTransferJobs", 0, 10);
}



void AD_Account_SetMaxTransfersPerJob(AB_ACCOUNT *a, int i){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "maxTransferJobs", i);
}



int AD_Account_GetMaxPurposeLines(const AB_ACCOUNT *a){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  return GWEN_DB_GetIntValue(db, "maxPurposeLines", 0, 4);
}



void AD_Account_SetMaxPurposeLines(AB_ACCOUNT *a, int i){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "maxPurposeLines", i);
}



int AD_Account_GetDebitAllowed(const AB_ACCOUNT *a){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  return GWEN_DB_GetIntValue(db, "debitAllowed", 0, 0);
}



void AD_Account_SetDebitAllowed(AB_ACCOUNT *a, int i){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "debitAllowed", i);
}



int AD_Account_GetMountAllowed(const AB_ACCOUNT *a){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  return GWEN_DB_GetIntValue(db, "mountAllowed", 0, 0);
}



void AD_Account_SetMountAllowed(AB_ACCOUNT *a, int i){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "mountAllowed", i);
}



const char *AD_Account_GetMountCommand(const AB_ACCOUNT *a) {
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  return GWEN_DB_GetCharValue(db, "mountCommand", 0, 0);
}



void AD_Account_SetMountCommand(AB_ACCOUNT *a, const char *s) {
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "mountCommand", s);
  else
    GWEN_DB_DeleteVar(db, "mountCommand");
}



const char *AD_Account_GetUnmountCommand(const AB_ACCOUNT *a) {
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  return GWEN_DB_GetCharValue(db, "unmountCommand", 0, 0);
}



void AD_Account_SetUnmountCommand(AB_ACCOUNT *a, const char *s) {
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "unmountCommand", s);
  else
    GWEN_DB_DeleteVar(db, "unmountCommand");

}



const char *AD_Account_GetFolder(const AB_ACCOUNT *a) {
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  return GWEN_DB_GetCharValue(db, "folder", 0, 0);
}



void AD_Account_SetFolder(AB_ACCOUNT *a, const char *s) {
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  if (s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "folder", s);
  else
    GWEN_DB_DeleteVar(db, "folder");
}



int AD_Account_GetUseDisc(const AB_ACCOUNT *a){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  return GWEN_DB_GetIntValue(db, "useDisc", 0, 0);
}



void AD_Account_SetUseDisc(AB_ACCOUNT *a, int i){
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "useDisc", i);
}



int AD_Account_GetPrintAllTransactions(const AB_ACCOUNT *a) {
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  return GWEN_DB_GetIntValue(db, "printAllTransactions", 0, 0);
}



void AD_Account_SetPrintAllTransactions(AB_ACCOUNT *a, int b) {
  GWEN_DB_NODE *db;

  db=AB_Account_GetProviderData(a);
  assert(db);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "printAllTransactions", b);
}








