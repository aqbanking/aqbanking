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


#ifndef AQBANKING_MSGENGINE_P_H
#define AQBANKING_MSGENGINE_P_H


#include "msgengine.h"


typedef struct AB_MSGENGINE AB_MSGENGINE;

struct AB_MSGENGINE {
};



void AB_MsgEngine_FreeData(void *bp, void *p);

int AB_MsgEngine_TypeRead(GWEN_MSGENGINE *e,
                          GWEN_BUFFER *msgbuf,
                          GWEN_XMLNODE *node,
                          GWEN_BUFFER *vbuf,
                          char escapeChar,
                          const char *delimiters);

int AB_MsgEngine_TypeWrite(GWEN_MSGENGINE *e,
                           GWEN_BUFFER *gbuf,
                           GWEN_BUFFER *data,
                           GWEN_XMLNODE *node);

GWEN_DB_VALUETYPE AB_MsgEngine_TypeCheck(GWEN_MSGENGINE *e,
                                         const char *tname);

const char *AB_MsgEngine_GetCharValue(GWEN_MSGENGINE *e,
                                      const char *name,
                                      const char *defValue);

int AB_MsgEngine_GetIntValue(GWEN_MSGENGINE *e,
                             const char *name,
                             int defValue);


int AB_MsgEngine_BinTypeRead(GWEN_MSGENGINE *e,
                             GWEN_XMLNODE *node,
                             GWEN_DB_NODE *gr,
                             GWEN_BUFFER *vbuf);

int AB_MsgEngine_BinTypeWrite(GWEN_MSGENGINE *e,
                              GWEN_XMLNODE *node,
                              GWEN_DB_NODE *gr,
                              GWEN_BUFFER *dbuf);


GWEN_TYPE_UINT32 AB_MsgEngine__FromBCD(GWEN_TYPE_UINT32 value);
GWEN_TYPE_UINT32 AB_MsgEngine__ToBCD(GWEN_TYPE_UINT32 value);



#endif /* CHIPCARD_CLIENT_MSGENGINE_P_H */


