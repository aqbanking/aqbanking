/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_MSG_MSG_H
#define AQEBICS_MSG_MSG_H


#include <aqebics/aqebics.h>

#include <gwenhywfar/types.h>
#include <gwenhywfar/path.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/buffer.h>


#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>


typedef struct EB_MSG EB_MSG;

GWEN_INHERIT_FUNCTION_DEFS(EB_MSG)



EB_MSG *EB_Msg_new();
void EB_Msg_free(EB_MSG *m);

EB_MSG *EB_Msg_newRequest(int willSign, const char *hVersion);
EB_MSG *EB_Msg_newResponse(int willSign, const char *rName, const char *hVersion);


EB_MSG *EB_Msg_fromBuffer(const char *buffer, int size);
void EB_Msg_toBuffer(EB_MSG *m, GWEN_BUFFER *buf);

EB_MSG *EB_Msg_fromFile(const char *fname);

xmlDocPtr EB_Msg_GetDoc(const EB_MSG *m);
xmlNodePtr EB_Msg_GetRootNode(EB_MSG *m);
xmlNodeSetPtr EB_Msg_GetNodes(EB_MSG *m, const char *xpathExpr);

DEPRECATED int EB_Msg_BuildHash(EB_MSG *m, GWEN_BUFFER *hbuf);

int EB_Msg_BuildHashSha1(EB_MSG *m, GWEN_BUFFER *hbuf);
int EB_Msg_BuildHashSha256(EB_MSG *m, GWEN_BUFFER *hbuf);
int EB_Msg_BuildHashSha256Sha256(EB_MSG *m, GWEN_BUFFER *hbuf);

int EB_Msg_BuildHashData(EB_MSG *m, GWEN_BUFFER *hbuf);


int EB_Msg_ReadHash(EB_MSG *m, GWEN_BUFFER *hbuf);
int EB_Msg_WriteHash(EB_MSG *m, const unsigned char *hash, int hsize);

int EB_Msg_ReadSignature(EB_MSG *m, GWEN_BUFFER *hbuf);
int EB_Msg_WriteSignature(EB_MSG *m, const unsigned char *hash, int hsize);


int EB_Msg_SetCharValue(EB_MSG *m, const char *path, const char *value);
const char *EB_Msg_GetCharValue(const EB_MSG *m, const char *path,
                                const char *defValue);

int EB_Msg_SetIntValue(EB_MSG *m, const char *path, int value);
int EB_Msg_GetIntValue(const EB_MSG *m, const char *path, int defValue);

EB_RC EB_Msg_GetResultCode(const EB_MSG *m);
EB_RC EB_Msg_GetBodyResultCode(const EB_MSG *m);

const char *EB_Msg_GetHVersion(const EB_MSG *m);
void EB_Msg_SetHVersion(EB_MSG *m, const char *s);

#endif

