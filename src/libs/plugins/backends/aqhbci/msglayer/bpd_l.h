/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2011 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_BPD_L_H
#define AH_BPD_L_H

#include <aqhbci/aqhbci.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/list2.h>
#include <gwenhywfar/db.h>
#include <stdio.h>


typedef enum {
  AH_BPD_AddrTypeTCP=0,
  AH_BPD_AddrTypeBTX,
  AH_BPD_AddrTypeSSL,
  AH_BPD_AddrTypeUnknown=99
} AH_BPD_ADDR_TYPE;


typedef enum {
  AH_BPD_FilterTypeNone=0,
  AH_BPD_FilterTypeBase64,
  AH_BPD_FilterTypeUUE
} AH_BPD_FILTER_TYPE;


typedef struct AH_BPD AH_BPD;

typedef struct AH_BPD_ADDR AH_BPD_ADDR;

GWEN_LIST_FUNCTION_LIB_DEFS(AH_BPD_ADDR, AH_BpdAddr, AQHBCI_API);
GWEN_LIST2_FUNCTION_LIB_DEFS(AH_BPD_ADDR, AH_BpdAddr, AQHBCI_API);


AH_BPD *AH_Bpd_new();
void AH_Bpd_free(AH_BPD *bpd);
AH_BPD *AH_Bpd_dup(const AH_BPD *oldBpd);

AH_BPD *AH_Bpd_FromDb(GWEN_DB_NODE *db);
int AH_Bpd_ToDb(const AH_BPD *bpd, GWEN_DB_NODE *db);


int AH_Bpd_GetBpdVersion(const AH_BPD *bpd);
void AH_Bpd_SetBpdVersion(AH_BPD *bpd, int i);
GWEN_DB_NODE *AH_Bpd_GetBpdJobs(const AH_BPD *bpd, int hbciVersion);
void AH_Bpd_SetBpdJobs(AH_BPD *bpd,
                       GWEN_DB_NODE *n);
void AH_Bpd_ClearBpdJobs(AH_BPD *bpd);

int AH_Bpd_GetJobTypesPerMsg(const AH_BPD *bpd);
void AH_Bpd_SetJobTypesPerMsg(AH_BPD *bpd, int i);

int AH_Bpd_GetMaxMsgSize(const AH_BPD *bpd);
void AH_Bpd_SetMaxMsgSize(AH_BPD *bpd, int i);

/**
 * Returns a NULL terminated list of HBCI versions supported by the server.
 */
const int *AH_Bpd_GetHbciVersions(const AH_BPD *bpd);
int AH_Bpd_AddHbciVersion(AH_BPD *bpd, int i);
void AH_Bpd_ClearHbciVersions(AH_BPD *bpd);

/**
 * Returns a NULL terminated list of languages supported by the server.
 */
const int *AH_Bpd_GetLanguages(const AH_BPD *bpd);
int AH_Bpd_AddLanguage(AH_BPD *bpd, int i);
void AH_Bpd_ClearLanguages(AH_BPD *bpd);


const char *AH_Bpd_GetBankAddr(const AH_BPD *bpd);
void AH_Bpd_SetBankAddr(AH_BPD *bpd, const char *addr);

int AH_Bpd_GetBankPort(const AH_BPD *bpd);
void AH_Bpd_SetBankPort(AH_BPD *bpd, int p);

AH_BPD_ADDR_TYPE AH_Bpd_GetAddrType(const AH_BPD *bpd);
void AH_Bpd_SetAddrType(AH_BPD *bpd, AH_BPD_ADDR_TYPE i);

const char *AH_Bpd_GetBankName(const AH_BPD *bpd);
void AH_Bpd_SetBankName(AH_BPD *bpd, const char *s);


int AH_Bpd_IsDirty(const AH_BPD *bpd);
void AH_Bpd_SetIsDirty(AH_BPD *bpd,
                       int dirty);

void AH_Bpd_Dump(const AH_BPD *bpd, int insert);

void AH_Bpd_ClearAddr(AH_BPD *bpd);
/** takes over ownership of the given BPD address */
void AH_Bpd_AddAddr(AH_BPD *bpd, AH_BPD_ADDR *ba);

AH_BPD_ADDR_LIST *AH_Bpd_GetAddrList(const AH_BPD *bpd);



AH_BPD_ADDR *AH_BpdAddr_new();
void AH_BpdAddr_free(AH_BPD_ADDR *ba);
AH_BPD_ADDR *AH_BpdAddr_dup(const AH_BPD_ADDR *ba);

AH_BPD_ADDR *AH_BpdAddr_FromDb(GWEN_DB_NODE *db);
int AH_BpdAddr_ToDb(const AH_BPD_ADDR *ba, GWEN_DB_NODE *db);

AH_BPD_ADDR_TYPE AH_BpdAddr_GetType(const AH_BPD_ADDR *ba);
void AH_BpdAddr_SetType(AH_BPD_ADDR *ba, AH_BPD_ADDR_TYPE t);

const char *AH_BpdAddr_GetAddr(const AH_BPD_ADDR *ba);
void AH_BpdAddr_SetAddr(AH_BPD_ADDR *ba, const char *s);

const char *AH_BpdAddr_GetSuffix(const AH_BPD_ADDR *ba);
void AH_BpdAddr_SetSuffix(AH_BPD_ADDR *ba, const char *s);

AH_BPD_FILTER_TYPE AH_BpdAddr_GetFType(const AH_BPD_ADDR *ba);
void AH_BpdAddr_SetFType(AH_BPD_ADDR *ba, AH_BPD_FILTER_TYPE t);

int AH_BpdAddr_GetFVersion(const AH_BPD_ADDR *ba);
void AH_BpdAddr_SetFVersion(AH_BPD_ADDR *ba, int i);


#endif /* AH_BPD_H */



