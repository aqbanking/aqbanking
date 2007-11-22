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

#ifndef AH_MSGENGINE_P_H
#define AH_MSGENGINE_P_H


#include "msgengine_l.h"


struct AH_MSGENGINE {
  AB_USER *user;
};


static int AH_MsgEngine_TypeRead(GWEN_MSGENGINE *e,
                                 GWEN_BUFFER *msgbuf,
                                 GWEN_XMLNODE *node,
                                 GWEN_BUFFER *vbuf,
                                 char escapeChar,
                                 const char *delimiters);
static int AH_MsgEngine_TypeWrite(GWEN_MSGENGINE *e,
                                  GWEN_BUFFER *gbuf,
                                  GWEN_BUFFER *data,
                                  GWEN_XMLNODE *node);
static GWEN_DB_NODE_TYPE AH_MsgEngine_TypeCheck(GWEN_MSGENGINE *e,
						const char *tname);

static int AH_MsgEngine_BinTypeWrite(GWEN_MSGENGINE *e,
                                     GWEN_XMLNODE *node,
                                     GWEN_DB_NODE *gr,
                                     GWEN_BUFFER *dbuf);

static const char *AH_MsgEngine_GetCharValue(GWEN_MSGENGINE *e,
                                             const char *name,
                                             const char *defValue);
static int AH_MsgEngine_GetIntValue(GWEN_MSGENGINE *e,
                                    const char *name,
                                    int defValue);
static AH_MSGENGINE *AH_MsgEngine_Data_new();
static void AH_MsgEngine_Data_free(AH_MSGENGINE *x);

static void GWENHYWFAR_CB AH_MsgEngine_FreeData(void *bp, void *p);



#endif /* AH_MSGENGINE_P_H */

