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


#ifndef AH_JOB_L_H
#define AH_JOB_L_H

#define AH_JOB_FLAGS_TANUSED        0x00004000
#define AH_JOB_FLAGS_NOSYSID        0x00008000
#define AH_JOB_FLAGS_NEEDCRYPT      0x00010000
#define AH_JOB_FLAGS_NEEDSIGN       0x00020000
#define AH_JOB_FLAGS_ATTACHABLE     0x00040000
#define AH_JOB_FLAGS_SINGLE         0x00080000
#define AH_JOB_FLAGS_DLGJOB         0x00100000
#define AH_JOB_FLAGS_CRYPT          0x00200000
#define AH_JOB_FLAGS_SIGN           0x00400000
#define AH_JOB_FLAGS_MULTIMSG       0x00800000
#define AH_JOB_FLAGS_HASATTACHPOINT 0x01000000
#define AH_JOB_FLAGS_HASMOREMSGS    0x02000000
#define AH_JOB_FLAGS_HASWARNINGS    0x04000000
#define AH_JOB_FLAGS_HASERRORS      0x08000000
#define AH_JOB_FLAGS_PROCESSED      0x10000000
#define AH_JOB_FLAGS_COMMITTED      0x20000000
#define AH_JOB_FLAGS_NEEDTAN        0x40000000
#define AH_JOB_FLAGS_OUTBOX         0x80000000

#include <aqhbci/job.h>

GWEN_LIST_FUNCTION_DEFS(AH_JOB, AH_Job);

AH_JOB *AH_Job_new(const char *name,
                   AB_USER *u,
                   const char *accountId);


GWEN_TYPE_UINT32 AH_Job_GetFirstSegment(const AH_JOB *j);
void AH_Job_SetFirstSegment(AH_JOB *j, GWEN_TYPE_UINT32 i);

GWEN_TYPE_UINT32 AH_Job_GetLastSegment(const AH_JOB *j);
void AH_Job_SetLastSegment(AH_JOB *j, GWEN_TYPE_UINT32 i);

int AH_Job_HasSegment(const AH_JOB *j, int seg);

/**
 * Takes over ownership of the given group.
 */
void AH_Job_AddResponse(AH_JOB *j, GWEN_DB_NODE *db);

void AH_Job_SetStatus(AH_JOB *j, AH_JOB_STATUS st);

GWEN_XMLNODE *AH_Job_GetXmlNode(const AH_JOB *j);


const GWEN_STRINGLIST *AH_Job_GetSigners(const AH_JOB *j);

int AH_Job_PrepareNextMessage(AH_JOB *j);

void AH_Job_SetMsgNum(AH_JOB *j, unsigned int i);
void AH_Job_SetDialogId(AH_JOB *j, const char *s);

GWEN_TYPE_UINT32 AH_Job_GetFlags(const AH_JOB *j);
void AH_Job_SetFlags(AH_JOB *j, GWEN_TYPE_UINT32 f);
void AH_Job_AddFlags(AH_JOB *j, GWEN_TYPE_UINT32 f);
void AH_Job_SubFlags(AH_JOB *j, GWEN_TYPE_UINT32 f);


GWEN_TYPE_UINT32 AH_Job_GetId(const AH_JOB *j);
void AH_Job_SetId(AH_JOB *j, GWEN_TYPE_UINT32 i);

AH_HBCI *AH_Job_GetHbci(const AH_JOB *j);
AB_BANKING *AH_Job_GetBankingApi(const AH_JOB *j);


const char *AH_Job_GetExpectedSigner(const AH_JOB *j);
void AH_Job_SetExpectedSigner(AH_JOB *j, const char *s);
const char *AH_Job_GetExpectedCrypter(const AH_JOB *j);
void AH_Job_SetExpectedCrypter(AH_JOB *j, const char *s);

int AH_Job_CheckEncryption(AH_JOB *j, GWEN_DB_NODE *dbRsp);
int AH_Job_CheckSignature(AH_JOB *j, GWEN_DB_NODE *dbRsp);

const char *AH_Job_GetUsedTan(const AH_JOB *j);
void AH_Job_SetUsedTan(AH_JOB *j, const char *s);

void AH_Job_Log(AH_JOB *j, AB_BANKING_LOGLEVEL ll, const char *txt);


#endif /* AH_JOB_L_H */




