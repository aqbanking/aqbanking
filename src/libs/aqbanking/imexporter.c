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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>


GWEN_INHERIT_FUNCTIONS(AB_IMEXPORTER)
GWEN_LIST_FUNCTIONS(AB_IMEXPORTER, AB_ImExporter)
GWEN_LIST_FUNCTIONS(AB_IMEXPORTER_ACCOUNTINFO, AB_ImExporterAccountInfo)



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
    DBG_INFO(AQBANKING_LOGDOMAIN, "Destroying AB_IMEXPORTER");
    GWEN_INHERIT_FINI(AB_IMEXPORTER, ie);
    if (ie->libLoader) {
      GWEN_LibLoader_CloseLibrary(ie->libLoader);
      GWEN_LibLoader_free(ie->libLoader);
    }
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
  assert(ctx);
  assert(bio);
  assert(params);

  if (ie->importFn)
    return ie->importFn(ie, ctx, bio, params);
  else
    return AB_ERROR_NOT_SUPPORTED;
}



int AB_ImExporter_Export(AB_IMEXPORTER *ie,
                         AB_IMEXPORTER_CONTEXT *ctx,
                         GWEN_BUFFEREDIO *bio,
                         GWEN_DB_NODE *params){
  assert(ie);
  assert(ctx);
  assert(bio);
  assert(params);

  if (ie->exportFn)
    return ie->exportFn(ie, ctx, bio, params);
  else
    return AB_ERROR_NOT_SUPPORTED;
}



int AB_ImExporter_CheckFile(AB_IMEXPORTER *ie,
                            const char *fname){
  assert(ie);
  assert(fname);

  if (ie->checkFileFn)
    return ie->checkFileFn(ie, fname);
  else
    return AB_ERROR_NOT_SUPPORTED;
}



int AB_ImExporter_ImportFile(AB_IMEXPORTER *ie,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             const char *fname,
                             GWEN_DB_NODE *dbProfile){
  int fd;
  GWEN_BUFFEREDIO *bio;
  int rv;

  assert(ie);
  assert(ctx);
  assert(fname);
  assert(dbProfile);

  fd=open(fname, O_RDONLY);
  if (fd==-1) {
    /* error */
    DBG_ERROR(AQBANKING_LOGDOMAIN, "open(%s): %s", fname, strerror(errno));
    return AB_ERROR_NOT_FOUND;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  rv=AB_ImExporter_Import(ie, ctx, bio, dbProfile);
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);

  return rv;
}



void AB_ImExporter_SetImportFn(AB_IMEXPORTER *ie,
                               AB_IMEXPORTER_IMPORT_FN f){
  assert(ie);
  ie->importFn=f;
}



void AB_ImExporter_SetExportFn(AB_IMEXPORTER *ie,
                               AB_IMEXPORTER_EXPORT_FN f){
  assert(ie);
  ie->exportFn=f;
}



void AB_ImExporter_SetCheckFileFn(AB_IMEXPORTER *ie,
                                  AB_IMEXPORTER_CHECKFILE_FN f){
  assert(ie);
  ie->checkFileFn=f;
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



AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterContext_GetFirstAccountInfo(AB_IMEXPORTER_CONTEXT *iec){
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  assert(iec);
  ai=AB_ImExporterAccountInfo_List_First(iec->accountInfoList);
  if (ai) {
    iec->nextAccountInfo=AB_ImExporterAccountInfo_List_Next(ai);
    return ai;
  }
  iec->nextAccountInfo=0;
  return 0;
}



AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterContext_GetNextAccountInfo(AB_IMEXPORTER_CONTEXT *iec){
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  assert(iec);
  ai=iec->nextAccountInfo;
  if (ai) {
    iec->nextAccountInfo=AB_ImExporterAccountInfo_List_Next(ai);
    return ai;
  }
  iec->nextAccountInfo=0;
  return 0;
}







AB_IMEXPORTER_ACCOUNTINFO *AB_ImExporterAccountInfo_new() {
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  GWEN_NEW_OBJECT(AB_IMEXPORTER_ACCOUNTINFO, iea);
  GWEN_LIST_INIT(AB_IMEXPORTER_ACCOUNTINFO, iea);
  iea->transactions=AB_Transaction_List_new();
  iea->accStatusList=AB_AccountStatus_List_new();
  return iea;
}



void AB_ImExporterAccountInfo_free(AB_IMEXPORTER_ACCOUNTINFO *iea){
  if (iea) {
    free(iea->bankCode);
    free(iea->bankName);
    free(iea->accountNumber);
    free(iea->accountName);
    free(iea->owner);
    AB_Transaction_List_free(iea->transactions);
    AB_AccountStatus_List_free(iea->accStatusList);
    GWEN_LIST_FINI(AB_IMEXPORTER_ACCOUNTINFO, iea);
    GWEN_FREE_OBJECT(iea);
  }
}







void AB_ImExporterAccountInfo_AddTransaction(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                             AB_TRANSACTION *t){
  assert(iea);
  assert(t);

  AB_Transaction_List_Add(t, iea->transactions);
}



const AB_TRANSACTION*
AB_ImExporterAccountInfo_GetFirstTransaction(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_TRANSACTION *t;

  assert(iea);
  t=AB_Transaction_List_First(iea->transactions);
  if (t) {
    iea->nextTransaction=AB_Transaction_List_Next(t);
    return t;
  }
  iea->nextTransaction=0;
  return 0;
}



const AB_TRANSACTION*
AB_ImExporterAccountInfo_GetNextTransaction(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_TRANSACTION *t;

  assert(iea);
  t=iea->nextTransaction;
  if (t) {
    iea->nextTransaction=AB_Transaction_List_Next(t);
    AB_Transaction_List_Del(t);
    return t;
  }
  iea->nextTransaction=0;
  return 0;
}


/*
AB_ACCOUNT*
AB_ImExporterAccountInfo_GetAccount(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->account;
}



void AB_ImExporterAccountInfo_SetAccount(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                         AB_ACCOUNT *a){
  assert(iea);
  iea->account=a;
}
*/


const char*
AB_ImExporterAccountInfo_GetBankCode(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->bankCode;
}



void AB_ImExporterAccountInfo_SetBankCode(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                          const char *s){
  assert(iea);
  free(iea->bankCode);
  if (s) iea->bankCode=strdup(s);
  else iea->bankCode=0;
}



const char*
AB_ImExporterAccountInfo_GetBankName(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->bankName;
}



void AB_ImExporterAccountInfo_SetBankName(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                          const char *s){
  assert(iea);
  free(iea->bankName);
  if (s) iea->bankName=strdup(s);
  else iea->bankName=0;
}



const char*
AB_ImExporterAccountInfo_GetAccountNumber(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->accountNumber;
}



void AB_ImExporterAccountInfo_SetAccountNumber(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                               const char *s){
  assert(iea);
  free(iea->accountNumber);
  if (s) iea->accountNumber=strdup(s);
  else iea->accountNumber=0;
}



const char*
AB_ImExporterAccountInfo_GetAccountName(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->accountName;
}



void AB_ImExporterAccountInfo_SetAccountName(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                             const char *s){
  assert(iea);
  free(iea->accountName);
  if (s) iea->accountName=strdup(s);
  else iea->accountName=0;
}



const char*
AB_ImExporterAccountInfo_GetOwner(const AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iea);
  return iea->owner;
}



void AB_ImExporterAccountInfo_SetOwner(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                       const char *s){
  assert(iea);
  free(iea->owner);
  if (s) iea->owner=strdup(s);
  else iea->owner=0;
}



void AB_ImExporterAccountInfo_AddAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *iea,
                                             AB_ACCOUNT_STATUS *t){
  assert(iea);
  assert(t);
  AB_AccountStatus_List_Add(t, iea->accStatusList);
}



AB_ACCOUNT_STATUS*
AB_ImExporterAccountInfo_GetFirstAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_ACCOUNT_STATUS *t;

  assert(iea);
  t=AB_AccountStatus_List_First(iea->accStatusList);
  if (t) {
    iea->nextAccountStatus=AB_AccountStatus_List_Next(t);
    return t;
  }
  iea->nextAccountStatus=0;
  return 0;
}



AB_ACCOUNT_STATUS*
AB_ImExporterAccountInfo_GetNextAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *iea){
  AB_ACCOUNT_STATUS *t;

  assert(iea);
  t=iea->nextAccountStatus;
  if (t) {
    iea->nextAccountStatus=AB_AccountStatus_List_Next(t);
    AB_AccountStatus_List_Del(t);
    return t;
  }
  iea->nextAccountStatus=0;
  return 0;
}












AB_IMEXPORTER_CONTEXT *AB_ImExporterContext_new(){
  AB_IMEXPORTER_CONTEXT *iec;

  GWEN_NEW_OBJECT(AB_IMEXPORTER_CONTEXT, iec);
  iec->accountInfoList=AB_ImExporterAccountInfo_List_new();

  return iec;
}



void AB_ImExporterContext_free(AB_IMEXPORTER_CONTEXT *iec){
  if (iec) {
    AB_ImExporterAccountInfo_List_free(iec->accountInfoList);
    GWEN_FREE_OBJECT(iec);
  }
}



AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterContext_GetFirstAccount(AB_IMEXPORTER_CONTEXT *iec){
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  assert(iec);
  iea=AB_ImExporterAccountInfo_List_First(iec->accountInfoList);
  if (iea) {
    iec->nextAccountInfo=AB_ImExporterAccountInfo_List_Next(iea);
    return iea;
  }
  iec->nextAccountInfo=0;
  return 0;
}



AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterContext_GetNextAccount(AB_IMEXPORTER_CONTEXT *iec){
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  assert(iec);
  iea=iec->nextAccountInfo;
  if (iea) {
    iec->nextAccountInfo=AB_ImExporterAccountInfo_List_Next(iea);
    AB_ImExporterAccountInfo_List_Del(iea);
    return iea;
  }
  iec->nextAccountInfo=0;
  return 0;
}



void AB_ImExporterContext_AddAccountInfo(AB_IMEXPORTER_CONTEXT *iec,
                                         AB_IMEXPORTER_ACCOUNTINFO *iea){
  assert(iec);
  assert(iea);
  AB_ImExporterAccountInfo_List_Add(iea, iec->accountInfoList);
}



AB_IMEXPORTER_ACCOUNTINFO*
AB_ImExporterContext_GetAccountInfo(AB_IMEXPORTER_CONTEXT *iec,
                                    const char *bankCode,
                                    const char *accountNumber){
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  if (!bankCode)
    bankCode="";
  if (!accountNumber)
    accountNumber="";

  assert(iec);
  iea=AB_ImExporterAccountInfo_List_First(iec->accountInfoList);
  while(iea) {
    if (strcasecmp(AB_ImExporterAccountInfo_GetBankCode(iea),
                   bankCode)==0 &&
        strcasecmp(AB_ImExporterAccountInfo_GetAccountNumber(iea),
                   accountNumber)==0)
      return iea;
    iea=AB_ImExporterAccountInfo_List_Next(iea);
  }

  /* not found, append it */
  iea=AB_ImExporterAccountInfo_new();
  AB_ImExporterAccountInfo_SetBankCode(iea, bankCode);
  AB_ImExporterAccountInfo_SetAccountNumber(iea, accountNumber);
  AB_ImExporterAccountInfo_List_Add(iea, iec->accountInfoList);
  return iea;
}



void AB_ImExporterContext_AddTransaction(AB_IMEXPORTER_CONTEXT *iec,
                                         AB_TRANSACTION *t){
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  iea=AB_ImExporterContext_GetAccountInfo
    (iec,
     AB_Transaction_GetLocalBankCode(t),
     AB_Transaction_GetLocalAccountNumber(t)
    );
  assert(iea);
  AB_ImExporterAccountInfo_AddTransaction(iea, t);
}




void AB_ImExporter_Utf8ToDta(const char *p,
                             int size,
                             GWEN_BUFFER *buf) {
  while(*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    if (c==0xc3) {
      if (size!=-1)
        size--;
      if (!size) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Incomplete UTF-8 sequence");
        break;
      }
      c=(unsigned char)(*(p++));
      switch(c) {
      case 0x84:
      case 0xa4: c=0x5b; break;
      case 0x96:
      case 0xb6: c=0x5c; break;
      case 0x9c:
      case 0xbc: c=0x5d; break;
      case 0x9f: c=0x7e; break;
      default:   c=' '; break;
      } /* switch */
    }
    else {
      c=toupper(c);
      if (!(isdigit(c) ||
	    (c>='A' && c<='Z') ||
	    (c>='a' && c<='z') ||
	    (strchr(" .,&-+*%/$", c))))
        c=' ';
    }
    GWEN_Buffer_AppendByte(buf, c);
    if (size!=-1)
      size--;
  } /* while */
}



void AB_ImExporter_DtaToUtf8(const char *p,
                             int size,
                             GWEN_BUFFER *buf) {
  while(*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    switch(c) {
    case 0x5b: /* AE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x84);
      break;

    case 0x5c: /* OE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x96);
      break;

    case 0x5d: /* UE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9c);
      break;

    case 0x7e: /* sharp s */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9f);
      break;

    default:
      GWEN_Buffer_AppendByte(buf, c);
    }
    if (size!=-1)
      size--;
  } /* while */
}














