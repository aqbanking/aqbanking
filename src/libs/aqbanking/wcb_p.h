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

#ifndef AB_WCB_P_H
#define AB_WCB_P_H

#include "wcb_l.h"


typedef struct AB_WAITCALLBACK AB_WAITCALLBACK;
struct AB_WAITCALLBACK {
  AB_BANKING *banking;
  GWEN_TYPE_UINT64 lastProgress;
};
static void GWENHYWFAR_CB AB_WaitCallback_FreeData(void *bp, void *p);


static GWEN_WAITCALLBACK_RESULT
  AB_WaitCallback_CheckAbort(GWEN_WAITCALLBACK *ctx,
                             unsigned int level);
static GWEN_WAITCALLBACK*
  AB_WaitCallback_Instantiate(GWEN_WAITCALLBACK *ctx);
static void AB_WaitCallback_Log(GWEN_WAITCALLBACK *ctx,
                                unsigned int level,
                                GWEN_LOGGER_LEVEL loglevel,
                                const char *s);







#endif /* AB_WCB_P_H */








