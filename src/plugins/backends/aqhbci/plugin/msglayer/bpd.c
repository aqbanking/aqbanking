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


#include "bpd_p.h"
#include "aqhbci_l.h"
#include <aqhbci/aqhbci.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_LIST_FUNCTIONS(AH_BPD_ADDR, AH_BpdAddr);
GWEN_LIST2_FUNCTIONS(AH_BPD_ADDR, AH_BpdAddr);


AH_BPD *AH_Bpd_new(){
  AH_BPD *bpd;

  GWEN_NEW_OBJECT(AH_BPD, bpd);
  bpd->bpdVersion=1;
  bpd->bpdJobs=GWEN_DB_Group_new("bpdjobs");
  bpd->addrList=AH_BpdAddr_List_new();

  bpd->isDirty=0;
  return bpd;
}



void AH_Bpd_free(AH_BPD *bpd){
  if (bpd) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Destroying AH_BPD");
    free(bpd->bankName);
    free(bpd->bankAddr);
    GWEN_DB_Group_free(bpd->bpdJobs);
    AH_BpdAddr_List_free(bpd->addrList);
    GWEN_FREE_OBJECT(bpd);
  }
}



AH_BPD *AH_Bpd_dup(const AH_BPD *oldBpd){
  AH_BPD *bpd;
  GWEN_DB_NODE *db;

  db=GWEN_DB_Group_new("tmp");
  if (AH_Bpd_ToDb(oldBpd, db)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_DB_Group_free(db);
    return 0;
  }
  bpd=AH_Bpd_FromDb(db);
  GWEN_DB_Group_free(db);
  if (!bpd) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return 0;
  }

  return bpd;
}



AH_BPD *AH_Bpd_FromDb(GWEN_DB_NODE *db){
  GWEN_DB_NODE *gr;
  int i;
  const char *p;

  AH_BPD *bpd;

  GWEN_NEW_OBJECT(AH_BPD, bpd);
  bpd->addrList=AH_BpdAddr_List_new();

  p=GWEN_DB_GetCharValue(db, "bankAddr", 0, 0);
  if (p)
    bpd->bankAddr=strdup(p);
  bpd->bankPort=GWEN_DB_GetIntValue(db, "bankPort", 0, 3000);

  p=GWEN_DB_GetCharValue(db, "addrType", 0, "tcp");
  if (strcasecmp(p, "tcp")==0)
    bpd->addrType=AH_BPD_AddrTypeTCP;
  else if (strcasecmp(p, "btx")==0)
    bpd->addrType=AH_BPD_AddrTypeBTX;
  else if (strcasecmp(p, "ssl")==0)
    bpd->addrType=AH_BPD_AddrTypeSSL;
  else {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Unsupported address type (%s), assuming TCP", p);
    bpd->addrType=AH_BPD_AddrTypeTCP;
  }

  p=GWEN_DB_GetCharValue(db, "bankName", 0, 0);
  if (p)
    bpd->bankName=strdup(p);

  bpd->jobTypesPerMsg=GWEN_DB_GetIntValue(db, "jobtypespermsg", 0, 0);
  bpd->maxMsgSize=GWEN_DB_GetIntValue(db, "maxmsgsize", 0, 0);
  bpd->bpdVersion=GWEN_DB_GetIntValue(db, "bpdversion", 0, 1);
  gr=GWEN_DB_GetGroup(db,
                      GWEN_PATH_FLAGS_PATHMUSTEXIST,
                      "bpdjobs");
  if (gr) {
    bpd->bpdJobs=GWEN_DB_Group_dup(gr);
  }
  else
    bpd->bpdJobs=GWEN_DB_Group_new("bpdjobs");

  /* read supported hbci versions */
  for (i=0; ; i++) {
    int j;

    j=GWEN_DB_GetIntValue(db, "hbciversions", i, 0);
    if (j) {
      if (AH_Bpd_AddHbciVersion(bpd, j)) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "Too many HBCI versions");
        break;
      }
    }
    else
      break;
  } /* for */

  /* read supported languages */
  for (i=0; ; i++) {
    int j;

    j=GWEN_DB_GetIntValue(db, "languages", i, 0);
    if (j) {
      if (AH_Bpd_AddLanguage(bpd, j)) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "Too many languages");
        break;
      }
    }
    else
      break;
  } /* for */

  /* store BPD addresses */
  gr=GWEN_DB_GetGroup(db,
                      GWEN_PATH_FLAGS_PATHMUSTEXIST,
                      "addresses");
  if (gr) {
    gr=GWEN_DB_FindFirstGroup(gr, "addr");
    while(gr) {
      AH_BPD_ADDR *ba;

      ba=AH_BpdAddr_FromDb(gr);
      if (ba)
        AH_BpdAddr_List_Add(ba, bpd->addrList);
      else {
        DBG_WARN(AQHBCI_LOGDOMAIN, "Bad BPD address");
      }
      gr=GWEN_DB_FindNextGroup(gr, "addr");
    }
  }

  bpd->isDirty=0;
  return bpd;
}



int AH_Bpd_ToDb(const AH_BPD *bpd, GWEN_DB_NODE *db){
  GWEN_DB_NODE *gr;
  unsigned int i;
  const char *p;
  AH_BPD_ADDR *ba;

  if (bpd->bankName)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "bankName", bpd->bankName);

  if (bpd->bankAddr)
    GWEN_DB_SetCharValue(db,
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "bankAddr",
                         bpd->bankAddr);
  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "bankPort",
                      bpd->bankPort);

  switch(bpd->addrType) {
  case AH_BPD_AddrTypeTCP: p="tcp"; break;
  case AH_BPD_AddrTypeBTX: p="btx"; break;
  case AH_BPD_AddrTypeSSL: p="ssl"; break;
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unsupported address type (%d)", bpd->addrType);
    return -1;
  }
  GWEN_DB_SetCharValue(db,
                       GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "addrType", p);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "jobtypespermsg", bpd->jobTypesPerMsg);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "maxmsgsize", bpd->maxMsgSize);

  GWEN_DB_SetIntValue(db,
                      GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "bpdversion", bpd->bpdVersion);
  if (bpd->bpdJobs) {
    gr=GWEN_DB_GetGroup(db,
                        GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                        "bpdjobs");
    assert(gr);
    GWEN_DB_AddGroupChildren(gr, bpd->bpdJobs);
  }

  GWEN_DB_DeleteVar(db, "hbciversions");
  for (i=0; i<AH_BPD_MAXHBCIVERSIONS; i++) {
    int j;

    j=bpd->hbciVersions[i];
    if (!j)
      break;
    GWEN_DB_SetIntValue(db,
                        GWEN_DB_FLAGS_DEFAULT,
                        "hbciversions", j);
  }

  GWEN_DB_DeleteVar(db, "languages");
  for (i=0; i<AH_BPD_MAXLANGUAGES; i++) {
    int j;

    j=bpd->languages[i];
    if (!j)
      break;
    GWEN_DB_SetIntValue(db,
                        GWEN_DB_FLAGS_DEFAULT,
                        "languages", j);
  }

  /* store BPD addresses */
  ba=AH_BpdAddr_List_First(bpd->addrList);
  if (ba) {
    gr=GWEN_DB_GetGroup(db,
                        GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                        "addresses");
    while(ba) {
      GWEN_DB_NODE *dbA;

      dbA=GWEN_DB_GetGroup(gr,
                           GWEN_PATH_FLAGS_CREATE_GROUP,
                           "addr");
      assert(dbA);
      if (AH_BpdAddr_ToDb(ba, dbA)) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here");
        return -1;
      }
      ba=AH_BpdAddr_List_Next(ba);
    }
  }

  return 0;
}





int AH_Bpd_GetJobTypesPerMsg(const AH_BPD *bpd){
  assert(bpd);
  return bpd->jobTypesPerMsg;
}



void AH_Bpd_SetJobTypesPerMsg(AH_BPD *bpd, int i){
  assert(bpd);
  bpd->jobTypesPerMsg=i;
  bpd->isDirty=1;
}



int AH_Bpd_GetMaxMsgSize(const AH_BPD *bpd){
  assert(bpd);
  return bpd->maxMsgSize;
}



void AH_Bpd_SetMaxMsgSize(AH_BPD *bpd, int i){
  assert(bpd);
  bpd->maxMsgSize=i;
  bpd->isDirty=1;
}



int AH_Bpd_GetBpdVersion(const AH_BPD *bpd){
  assert(bpd);
  return bpd->bpdVersion;
}



void AH_Bpd_SetBpdVersion(AH_BPD *bpd, int i){
  assert(bpd);
  bpd->bpdVersion=i;
  bpd->isDirty=1;
}



GWEN_DB_NODE *AH_Bpd_GetBpdJobs(const AH_BPD *bpd, int hbciVersion){
  GWEN_DB_NODE *gr;
  char numbuf[16];

  assert(bpd);
  snprintf(numbuf, sizeof(numbuf), "%3d", hbciVersion);
  gr=GWEN_DB_GetGroup(bpd->bpdJobs, GWEN_PATH_FLAGS_NAMEMUSTEXIST, numbuf);
  if (gr)
    return gr;
  return bpd->bpdJobs;
}



void AH_Bpd_SetBpdJobs(AH_BPD *bpd, GWEN_DB_NODE *n){
  assert(bpd);
  GWEN_DB_Group_free(bpd->bpdJobs);
  bpd->bpdJobs=n;
  bpd->isDirty=1;
}



void AH_Bpd_ClearBpdJobs(AH_BPD *bpd){
  assert(bpd);
  GWEN_DB_ClearGroup(bpd->bpdJobs, 0);
  bpd->isDirty=1;
}



const int *AH_Bpd_GetHbciVersions(const AH_BPD *bpd){
  assert(bpd);
  return bpd->hbciVersions;
}



int AH_Bpd_AddHbciVersion(AH_BPD *bpd, int i){
  int j;

  assert(bpd);
  assert(i);

  for (j=0; j<AH_BPD_MAXHBCIVERSIONS; j++) {
    if (bpd->hbciVersions[j]==i) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "HBCI version %d already enlisted", i);
      return -1;
    }
    if (bpd->hbciVersions[j]==0) {
      bpd->hbciVersions[j]=i;
      bpd->isDirty=1;
      return 0;
    }
  }
  DBG_ERROR(AQHBCI_LOGDOMAIN, "Too many HBCI versions.");
  return -1;
}


void AH_Bpd_ClearHbciVersions(AH_BPD *bpd){
  unsigned int i;

  assert(bpd);
  /* note: i<= is correct here, since the field size is k+1 */
  for (i=0; i<=AH_BPD_MAXHBCIVERSIONS; i++)
    bpd->hbciVersions[i]=0;
  bpd->isDirty=1;
}



int AH_Bpd_AddLanguage(AH_BPD *bpd, int i){
  int j;

  assert(bpd);
  assert(i);

  for (j=0; j<AH_BPD_MAXLANGUAGES; j++) {
    if (bpd->languages[j]==i) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Language %d already enlisted", i);
      return -1;
    }
    if (bpd->languages[j]==0) {
      bpd->languages[j]=i;
      bpd->isDirty=1;
      return 0;
    }
  }
  DBG_ERROR(AQHBCI_LOGDOMAIN, "Too many languages.");
  return -1;
}



const int *AH_Bpd_GetLanguages(const AH_BPD *bpd){
  assert(bpd);
  return bpd->languages;
}



void AH_Bpd_ClearLanguages(AH_BPD *bpd){
  unsigned int i;

  assert(bpd);
  /* note: i<= is correct here, since the field size is k+1 */
  for (i=0; i<=AH_BPD_MAXLANGUAGES; i++)
    bpd->languages[i]=0;
  bpd->isDirty=1;
}



int AH_Bpd_IsDirty(const AH_BPD *bpd){
  assert(bpd);
  if (bpd->isDirty)
    return 1;
  return 0;
}



void AH_Bpd_SetIsDirty(AH_BPD *bpd, int dirty){
  assert(bpd);
  bpd->isDirty=dirty;
}



void AH_Bpd_Dump(const AH_BPD *bpd, int insert) {
  uint32_t k;
  unsigned int i;

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Bpd:\n");

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Version : %d\n", bpd->bpdVersion);

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "JobTypesPerMsg : %d\n", bpd->jobTypesPerMsg);

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "MaxMsgSize     : %d\n", bpd->maxMsgSize);

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "HBCI versions  : ");
  for (i=0; i<AH_BPD_MAXHBCIVERSIONS; i++) {
    if (bpd->hbciVersions[i])
      fprintf(stderr, "%d ",bpd->hbciVersions[i]);
  }
  fprintf(stderr, "\n");

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "Languages      : ");
  for (i=0; i<AH_BPD_MAXLANGUAGES; i++) {
    if (bpd->languages[i])
      fprintf(stderr, "%d ",bpd->languages[i]);
  }
  fprintf(stderr, "\n");

  for (k=0; k<insert; k++)
    fprintf(stderr, " ");
  fprintf(stderr, "BPD Jobs      :\n");
  GWEN_DB_Dump(bpd->bpdJobs, insert+17);
}



const char *AH_Bpd_GetBankAddr(const AH_BPD *bpd){
  assert(bpd);
  return bpd->bankAddr;
}



void AH_Bpd_SetBankAddr(AH_BPD *bpd, const char *addr){
  assert(bpd);
  assert(addr);
  free(bpd->bankAddr);
  bpd->bankAddr=strdup(addr);
  bpd->isDirty=1;
}



int AH_Bpd_GetBankPort(const AH_BPD *bpd){
  assert(bpd);
  return bpd->bankPort;
}



void AH_Bpd_SetBankPort(AH_BPD *bpd, int p){
  assert(bpd);
  bpd->bankPort=p;
  bpd->isDirty=1;
}



AH_BPD_ADDR_TYPE AH_Bpd_GetAddrType(const AH_BPD *bpd){
  assert(bpd);
  return bpd->addrType;
}



void AH_Bpd_SetAddrType(AH_BPD *bpd, AH_BPD_ADDR_TYPE i){
  bpd->addrType=i;
  bpd->isDirty=1;
}



const char *AH_Bpd_GetBankName(const AH_BPD *bpd){
  assert(bpd);
  return bpd->bankName;
}



void AH_Bpd_SetBankName(AH_BPD *bpd, const char *s){
  assert(bpd);
  assert(s);
  free(bpd->bankName);
  bpd->bankName=strdup(s);
  bpd->isDirty=1;
}



void AH_Bpd_ClearAddr(AH_BPD *bpd){
  assert(bpd);
  AH_BpdAddr_List_Clear(bpd->addrList);
}



void AH_Bpd_AddAddr(AH_BPD *bpd, AH_BPD_ADDR *ba){
  assert(bpd);
  assert(ba);
  AH_BpdAddr_List_Add(ba, bpd->addrList);
}



AH_BPD_ADDR_LIST *AH_Bpd_GetAddrList(const AH_BPD *bpd){
  assert(bpd);
  return bpd->addrList;
}










AH_BPD_ADDR *AH_BpdAddr_new(){
  AH_BPD_ADDR *ba;

  GWEN_NEW_OBJECT(AH_BPD_ADDR, ba);
  GWEN_LIST_INIT(AH_BPD_ADDR, ba);

  return ba;
}



void AH_BpdAddr_free(AH_BPD_ADDR *ba){
  if (ba) {
    GWEN_LIST_FINI(AH_BPD_ADDR, ba);
    free(ba->addr);
    free(ba->suffix);
    GWEN_FREE_OBJECT(ba);
  }
}



AH_BPD_ADDR *AH_BpdAddr_dup(const AH_BPD_ADDR *ba){
  AH_BPD_ADDR *newBa;

  assert(ba);
  newBa=AH_BpdAddr_new();
  newBa->type=ba->type;
  if (ba->addr)
    newBa->addr=strdup(ba->addr);
  if (ba->suffix)
    newBa->suffix=strdup(ba->suffix);
  newBa->ftype=ba->ftype;
  newBa->fversion=ba->fversion;

  return newBa;
}



AH_BPD_ADDR_TYPE AH_BpdAddr_GetType(const AH_BPD_ADDR *ba){
  assert(ba);
  return ba->type;
}



void AH_BpdAddr_SetType(AH_BPD_ADDR *ba, AH_BPD_ADDR_TYPE t){
  assert(ba);
  ba->type=t;
}



const char *AH_BpdAddr_GetAddr(const AH_BPD_ADDR *ba){
  assert(ba);
  return ba->addr;
}



void AH_BpdAddr_SetAddr(AH_BPD_ADDR *ba, const char *s){
  assert(ba);
  free(ba->addr);
  if (s) ba->addr=strdup(s);
  else ba->addr=0;
}



const char *AH_BpdAddr_GetSuffix(const AH_BPD_ADDR *ba){
  assert(ba);
  return ba->suffix;
}



void AH_BpdAddr_SetSuffix(AH_BPD_ADDR *ba, const char *s){
  assert(ba);
  free(ba->suffix);
  if (s) ba->suffix=strdup(s);
  else ba->suffix=0;
}



AH_BPD_FILTER_TYPE AH_BpdAddr_GetFType(const AH_BPD_ADDR *ba){
  assert(ba);
  return ba->ftype;
}



void AH_BpdAddr_SetFType(AH_BPD_ADDR *ba, AH_BPD_FILTER_TYPE t){
  assert(ba);
  ba->ftype=t;
}



int AH_BpdAddr_GetFVersion(const AH_BPD_ADDR *ba){
  assert(ba);
  return ba->fversion;
}



void AH_BpdAddr_SetFVersion(AH_BPD_ADDR *ba, int i){
  assert(ba);
  ba->fversion=i;
}



AH_BPD_ADDR *AH_BpdAddr_FromDb(GWEN_DB_NODE *db){
  AH_BPD_ADDR *ba;
  const char *p;

  ba=AH_BpdAddr_new();
  p=GWEN_DB_GetCharValue(db, "type", 0, 0);
  if (!p) {
    int i;

    i=GWEN_DB_GetIntValue(db, "type", 0, -1);
    if (i==-1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "bad BPD address");
      GWEN_DB_Dump(db, 2);
      AH_BpdAddr_free(ba);
      return 0;
    }
    switch(i) {
    case 1: ba->type=AH_BPD_AddrTypeBTX; break;
    case 2: ba->type=AH_BPD_AddrTypeTCP; break;
    case 3: ba->type=AH_BPD_AddrTypeSSL; break;
    default:
      DBG_ERROR(AQHBCI_LOGDOMAIN, "bad BPD address: unknown type %i", i);
      ba->type=AH_BPD_AddrTypeUnknown; break;
      GWEN_DB_Dump(db, 2);
      AH_BpdAddr_free(ba);
      return 0;
    }
  }
  else {
    if (strcasecmp(p, "tcp")==0)
      ba->type=AH_BPD_AddrTypeTCP;
    else if (strcasecmp(p, "btx")==0)
      ba->type=AH_BPD_AddrTypeBTX;
    else if (strcasecmp(p, "ssl")==0)
      ba->type=AH_BPD_AddrTypeSSL;
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "bad BPD address: bad type %s", p);
      GWEN_DB_Dump(db, 2);
      AH_BpdAddr_free(ba);
      return 0;
    }
  }

  p=GWEN_DB_GetCharValue(db, "filter", 0, 0);
  if (p) {
    if (strcasecmp(p, "mim")==0)
      ba->ftype=AH_BPD_FilterTypeBase64;
    else if (strcasecmp(p, "uue")==0)
      ba->ftype=AH_BPD_FilterTypeUUE;
    else if (strcasecmp(p, "none")==0)
      ba->ftype=AH_BPD_FilterTypeNone;
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "bad BPD address: bad filter type %s", p);
      GWEN_DB_Dump(db, 2);
      AH_BpdAddr_free(ba);
      return 0;
    }
    ba->fversion=GWEN_DB_GetIntValue(db, "fversion", 0, 0);
  }
  else {
    ba->ftype=AH_BPD_FilterTypeNone;
  }

  p=GWEN_DB_GetCharValue(db, "address", 0, 0);
  if (!p) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "bad BPD address: no address");
    GWEN_DB_Dump(db, 2);
    AH_BpdAddr_free(ba);
    return 0;
  }
  ba->addr=strdup(p);

  p=GWEN_DB_GetCharValue(db, "suffix", 0, 0);
  if (p)
    ba->suffix=strdup(p);


  return ba;
}



int AH_BpdAddr_ToDb(const AH_BPD_ADDR *ba, GWEN_DB_NODE *db){
  assert(ba);
  assert(db);

  if (!ba->addr) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No address in BPD address");
    return -1;
  }

  switch(ba->type) {
  case AH_BPD_AddrTypeTCP:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "type", "tcp");
    break;
  case AH_BPD_AddrTypeBTX:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "type", "btx");
    break;
  case AH_BPD_AddrTypeSSL:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "type", "ssl");
    break;
  default:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "type", "unknown");
    break;
  }

  switch(ba->ftype) {
  case AH_BPD_FilterTypeBase64:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "ftype", "mim");
    break;
  case AH_BPD_FilterTypeUUE:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "ftype", "uue");
    break;
  case AH_BPD_FilterTypeNone:
  default:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "filter", "none");
    break;
  }

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "fversion", ba->fversion);

  if (ba->addr)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "address", ba->addr);
  if (ba->suffix)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "suffix", ba->suffix);

  return 0;
}










