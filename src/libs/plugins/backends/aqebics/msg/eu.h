/***************************************************************************
    begin       : Sat Mar 08 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_MSG_EU_H
#define AQEBICS_MSG_EU_H


typedef struct EB_EU EB_EU;

#include <aqebics/aqebics.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gwentime.h>


GWEN_LIST_FUNCTION_DEFS(EB_EU, EB_Eu)


EB_EU *EB_Eu_new();
void EB_Eu_free(EB_EU *eu);

EB_EU *EB_Eu_dup(const EB_EU *oldEu);

EB_EU *EB_Eu_fromBuffer(const uint8_t *p, uint32_t l);
void EB_Eu_toBuffer(const EB_EU *eu, GWEN_BUFFER *buf);

int EB_Eu_toDb(const EB_EU *eu, GWEN_DB_NODE *db);
EB_EU *EB_Eu_fromDb(GWEN_DB_NODE *db);

const char *EB_Eu_GetVersion(const EB_EU *eu);
void EB_Eu_SetVersion(EB_EU *eu, const char *s);

int EB_Eu_GetModLen(const EB_EU *eu);
void EB_Eu_SetModLen(EB_EU *eu, int i);

const char *EB_Eu_GetJobType(const EB_EU *eu);
void EB_Eu_SetJobType(EB_EU *eu, const char *s);

const uint8_t *EB_Eu_GetSignaturePtr(const EB_EU *eu);
uint32_t EB_Eu_GetSignatureLen(const EB_EU *eu);
void EB_Eu_SetSignature(EB_EU *eu, const uint8_t *p, uint32_t l);

const char *EB_Eu_GetUserId(const EB_EU *eu);
void EB_Eu_SetUserId(EB_EU *eu, const char *s);

const char *EB_Eu_GetOriginalFileName(const EB_EU *eu);
void EB_Eu_SetOriginalFileName(EB_EU *eu, const char *s);

const GWEN_TIME *EB_Eu_GetCreationTime(const EB_EU *eu);
void EB_Eu_SetCreationTime(EB_EU *eu, const GWEN_TIME *ti);

const GWEN_TIME *EB_Eu_GetSignatureTime(const EB_EU *eu);
void EB_Eu_SetSignatureTime(EB_EU *eu, const GWEN_TIME *ti);



#endif

