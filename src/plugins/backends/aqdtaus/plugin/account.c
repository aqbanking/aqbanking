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



AB_ACCOUNT *AD_Account_new(AB_BANKING *ab,
                           AB_PROVIDER *pro,
                           const char *idForProvider){
  AB_ACCOUNT *a;
  AD_ACCOUNT *ad;

  a=AB_Account_new(ab, pro, idForProvider);
  GWEN_NEW_OBJECT(AD_ACCOUNT, ad);
  GWEN_INHERIT_SETDATA(AB_ACCOUNT, AD_ACCOUNT, a, ad, AD_Account_FreeData);

  return a;
}



void AD_Account_FreeData(void *bp, void *p) {
  AD_ACCOUNT *ad;

  ad=(AD_ACCOUNT*)p;
  free(ad->folder);
  free(ad->unmountCommand);
  free(ad->mountCommand);
  GWEN_FREE_OBJECT(ad);
}



AB_ACCOUNT *AD_Account_fromDb(AB_BANKING *ab,
                              GWEN_DB_NODE *db){
  AB_ACCOUNT *acc;
  AD_ACCOUNT *ad;
  GWEN_DB_NODE *dbBase;
  const char *s;

  dbBase=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "base");
  assert(dbBase);
  acc=AB_Account_fromDb(ab, dbBase);
  assert(acc);

  GWEN_NEW_OBJECT(AD_ACCOUNT, ad);
  GWEN_INHERIT_SETDATA(AB_ACCOUNT, AD_ACCOUNT, acc, ad, AD_Account_FreeData);

  /* read own data */
  ad->maxTransfersPerJob=GWEN_DB_GetIntValue(db, "maxTransfersPerJob", 0, 0);
  ad->maxPurposeLines=GWEN_DB_GetIntValue(db, "maxPurposeLines", 0, 4);
  ad->debitAllowed=GWEN_DB_GetIntValue(db, "debitAllowed", 0, 0);

  ad->useDisc=GWEN_DB_GetIntValue(db, "useDisc", 0, 0);
  ad->printAllTransactions=
    GWEN_DB_GetIntValue(db, "printAllTransactions", 0, 0);
  ad->mountAllowed=GWEN_DB_GetIntValue(db, "mountAllowed", 0, 0);
  s=GWEN_DB_GetCharValue(db, "folder", 0, 0);
  if (s)
    ad->folder=strdup(s);
  s=GWEN_DB_GetCharValue(db, "mountCommand", 0, 0);
  if (s)
    ad->mountCommand=strdup(s);
  s=GWEN_DB_GetCharValue(db, "unmountCommand", 0, 0);
  if (s)
    ad->unmountCommand=strdup(s);


  return acc;
}



int AD_Account_toDb(const AB_ACCOUNT *acc, GWEN_DB_NODE *db){
  AD_ACCOUNT *ad;
  GWEN_DB_NODE *dbBase;
  int rv;

  DBG_DEBUG(AQDTAUS_LOGDOMAIN, "Saving Account");
  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  dbBase=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "base");
  assert(dbBase);
  rv=AB_Account_toDb(acc, dbBase);
  if (rv) {
    DBG_INFO(AQDTAUS_LOGDOMAIN, "here");
    return rv;
  }


  /* write own data */
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "maxTransfersPerJob", ad->maxTransfersPerJob);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "maxPurposeLines", ad->maxPurposeLines);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "debitAllowed", ad->debitAllowed);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "printAllTransactions", ad->printAllTransactions);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "useDisc", ad->useDisc);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "mountAllowed", ad->mountAllowed);
  if (ad->folder)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "folder", ad->folder);
  if (ad->mountCommand)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "mountCommand", ad->mountCommand);
  if (ad->unmountCommand)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "unmountCommand", ad->unmountCommand);

  return 0;
}



int AD_Account_GetMaxTransfersPerJob(const AB_ACCOUNT *acc){
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  return ad->maxTransfersPerJob;
}



void AD_Account_SetMaxTransfersPerJob(AB_ACCOUNT *acc, int i){
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  ad->maxTransfersPerJob=i;
}


int AD_Account_GetMaxPurposeLines(const AB_ACCOUNT *acc){
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  return ad->maxPurposeLines;
}



void AD_Account_SetMaxPurposeLines(AB_ACCOUNT *acc, int i){
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  ad->maxPurposeLines=i;
}



int AD_Account_GetDebitAllowed(const AB_ACCOUNT *acc){
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  return ad->debitAllowed;
}



void AD_Account_SetDebitAllowed(AB_ACCOUNT *acc, int i){
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  ad->debitAllowed=i;
}



int AD_Account_GetMountAllowed(const AB_ACCOUNT *acc){
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  return ad->mountAllowed;
}



void AD_Account_SetMountAllowed(AB_ACCOUNT *acc, int i){
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  ad->mountAllowed=i;
}



const char *AD_Account_GetMountCommand(const AB_ACCOUNT *acc) {
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);
  return ad->mountCommand;
}



void AD_Account_SetMountCommand(AB_ACCOUNT *acc, const char *s) {
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  free(ad->mountCommand);
  if (s) ad->mountCommand=strdup(s);
  else ad->mountCommand=0;
}



const char *AD_Account_GetUnmountCommand(const AB_ACCOUNT *acc) {
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);
  return ad->unmountCommand;
}



void AD_Account_SetUnmountCommand(AB_ACCOUNT *acc, const char *s) {
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  free(ad->unmountCommand);
  if (s) ad->unmountCommand=strdup(s);
  else ad->mountCommand=0;
}



const char *AD_Account_GetFolder(const AB_ACCOUNT *acc) {
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  return ad->folder;
}



void AD_Account_SetFolder(AB_ACCOUNT *acc, const char *s) {
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  free(ad->folder);
  if (s) ad->folder=strdup(s);
  else ad->folder=0;
}



int AD_Account_GetUseDisc(const AB_ACCOUNT *acc){
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  return ad->useDisc;
}



void AD_Account_SetUseDisc(AB_ACCOUNT *acc, int i){
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  ad->useDisc=i;
}



int AD_Account_GetPrintAllTransactions(const AB_ACCOUNT *acc) {
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  return ad->printAllTransactions;
}



void AD_Account_SetPrintAllTransactions(AB_ACCOUNT *acc, int b) {
  AD_ACCOUNT *ad;

  assert(acc);
  ad=GWEN_INHERIT_GETDATA(AB_ACCOUNT, AD_ACCOUNT, acc);
  assert(ad);

  ad->printAllTransactions=b;
}








