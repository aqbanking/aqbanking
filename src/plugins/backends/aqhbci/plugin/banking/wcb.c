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

#define GWEN_EXTEND_WAITCALLBACK


#include "wcb_p.h"
#include "aqhbci_l.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/debug.h>


GWEN_INHERIT(GWEN_WAITCALLBACK, AH_WAITCALLBACK);


GWEN_WAITCALLBACK *AH_WaitCallback_new(AH_HBCI *hbci, const char *id){
  GWEN_WAITCALLBACK *ctx;
  AH_WAITCALLBACK *wcb;

  assert(hbci);
  ctx=GWEN_WaitCallback_new(id);
  GWEN_NEW_OBJECT(AH_WAITCALLBACK, wcb);
  wcb->hbci=hbci;
  wcb->banking=AH_HBCI_GetBankingApi(hbci);

  GWEN_INHERIT_SETDATA(GWEN_WAITCALLBACK, AH_WAITCALLBACK,
                       ctx, wcb,
                       AH_WaitCallback_FreeData);

  GWEN_WaitCallback_SetCheckAbortFn(ctx, AH_WaitCallback_CheckAbort);
  GWEN_WaitCallback_SetInstantiateFn(ctx, AH_WaitCallback_Instantiate);
  GWEN_WaitCallback_SetLogFn(ctx, AH_WaitCallback_Log);

  return ctx;
}



void AH_WaitCallback_FreeData(void *bp, void *p){
  AH_WAITCALLBACK *wcb;

  wcb=(AH_WAITCALLBACK*)p;
  GWEN_FREE_OBJECT(wcb);
}




GWEN_WAITCALLBACK_RESULT AH_WaitCallback_CheckAbort(GWEN_WAITCALLBACK *ctx,
                                                    unsigned int level){
  AH_WAITCALLBACK *wcb;
  int rv;

  assert(ctx);
  wcb=GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK, AH_WAITCALLBACK, ctx);
  assert(wcb);

  DBG_VERBOUS(0, "WaitCallback: %s (level %d)",
              GWEN_WaitCallback_GetId(ctx), level);

  if (level!=0) {
    DBG_VERBOUS(AQHBCI_LOGDOMAIN, "Level: %d", level);
    rv=AB_Banking_ProgressAdvance(wcb->banking, 0, AB_BANKING_PROGRESS_NONE);
  }
  else {
    if (wcb->lastProgress==GWEN_WaitCallback_GetProgressPos(ctx))
      rv=AB_Banking_ProgressAdvance(wcb->banking, 0, AB_BANKING_PROGRESS_NONE);
    else {
      wcb->lastProgress=GWEN_WaitCallback_GetProgressPos(ctx);
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Progress changed to %lld", wcb->lastProgress);
      rv=AB_Banking_ProgressAdvance(wcb->banking, 0, wcb->lastProgress);
    }
  }
  if (rv==AB_ERROR_USER_ABORT)
    return GWEN_WaitCallbackResult_Abort;
  return GWEN_WaitCallbackResult_Continue;
}



GWEN_WAITCALLBACK *AH_WaitCallback_Instantiate(GWEN_WAITCALLBACK *ctx){
  AH_WAITCALLBACK *wcb;

  assert(ctx);
  wcb=GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK, AH_WAITCALLBACK, ctx);
  assert(wcb);

  return AH_WaitCallback_new(wcb->hbci, GWEN_WaitCallback_GetId(ctx));
}



void AH_WaitCallback_Log(GWEN_WAITCALLBACK *ctx,
                         unsigned int level,
                         unsigned int loglevel,
                         const char *s){
  AH_WAITCALLBACK *wcb;
  AB_BANKING_LOGLEVEL nl;

  assert(ctx);
  wcb=GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK, AH_WAITCALLBACK, ctx);
  assert(wcb);

  switch(loglevel) {
  case 0: nl=AB_Banking_LogLevelError; break;
  case 1: nl=AB_Banking_LogLevelWarn; break;
  case 2: nl=AB_Banking_LogLevelNotice; break;
  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Loglevel %d -> info", loglevel);
    nl=AB_Banking_LogLevelInfo;
    break;
  }

  AB_Banking_ProgressLog(wcb->banking, 0, nl, s);
}








