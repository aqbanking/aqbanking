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

#ifndef AH_WCB_P_H
#define AH_WCB_P_H

#include "wcb_l.h"


typedef struct AH_WAITCALLBACK AH_WAITCALLBACK;
struct AH_WAITCALLBACK {
  AH_HBCI *hbci;
  AB_BANKING *banking;
  GWEN_TYPE_UINT64 lastProgress;
};
void AH_WaitCallback_FreeData(void *bp, void *p);


GWEN_WAITCALLBACK_RESULT AH_WaitCallback_CheckAbort(GWEN_WAITCALLBACK *ctx,
                                                    unsigned int level);
GWEN_WAITCALLBACK *AH_WaitCallback_Instantiate(GWEN_WAITCALLBACK *ctx);
void AH_WaitCallback_Log(GWEN_WAITCALLBACK *ctx,
                         unsigned int level,
                         unsigned int loglevel,
                         const char *s);







#endif /* AH_WCB_P_H */








