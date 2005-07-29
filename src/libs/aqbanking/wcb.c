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
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/debug.h>


GWEN_INHERIT(GWEN_WAITCALLBACK, AB_WAITCALLBACK);


GWEN_WAITCALLBACK *AB_WaitCallback_new(AB_BANKING *ab, const char *id){
  GWEN_WAITCALLBACK *ctx;
  AB_WAITCALLBACK *wcb;

  assert(ab);
  ctx=GWEN_WaitCallback_new(id);
  GWEN_NEW_OBJECT(AB_WAITCALLBACK, wcb);
  wcb->banking=ab;

  GWEN_INHERIT_SETDATA(GWEN_WAITCALLBACK, AB_WAITCALLBACK,
                       ctx, wcb,
                       AB_WaitCallback_FreeData);

  GWEN_WaitCallback_SetCheckAbortFn(ctx, AB_WaitCallback_CheckAbort);
  GWEN_WaitCallback_SetInstantiateFn(ctx, AB_WaitCallback_Instantiate);
  GWEN_WaitCallback_SetLogFn(ctx, AB_WaitCallback_Log);

  return ctx;
}



void AB_WaitCallback_FreeData(void *bp, void *p){
  AB_WAITCALLBACK *wcb;

  wcb=(AB_WAITCALLBACK*)p;
  GWEN_FREE_OBJECT(wcb);
}




GWEN_WAITCALLBACK_RESULT AB_WaitCallback_CheckAbort(GWEN_WAITCALLBACK *ctx,
                                                    unsigned int level){
  AB_WAITCALLBACK *wcb;
  int rv;

  assert(ctx);
  wcb=GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK, AB_WAITCALLBACK, ctx);
  assert(wcb);

  DBG_VERBOUS(0, "WaitCallback %p: %s (level %d)",
	      ctx, GWEN_WaitCallback_GetId(ctx), level);

  if (level!=0) {
    DBG_VERBOUS(AQBANKING_LOGDOMAIN, "Level: %d", level);
    rv=AB_Banking_ProgressAdvance(wcb->banking, 0, AB_BANKING_PROGRESS_NONE);
  }
  else {
    if (wcb->lastProgress==GWEN_WaitCallback_GetProgressPos(ctx))
      rv=AB_Banking_ProgressAdvance(wcb->banking, 0, AB_BANKING_PROGRESS_NONE);
    else {
      wcb->lastProgress=GWEN_WaitCallback_GetProgressPos(ctx);
      DBG_NOTICE(AQBANKING_LOGDOMAIN,
		 "Progress changed to %lld",
		 wcb->lastProgress);
      rv=AB_Banking_ProgressAdvance(wcb->banking, 0, wcb->lastProgress);
    }
  }

  if (rv==AB_ERROR_USER_ABORT) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Aborted");
    return GWEN_WaitCallbackResult_Abort;
  }
  return GWEN_WaitCallbackResult_Continue;
}



GWEN_WAITCALLBACK *AB_WaitCallback_Instantiate(GWEN_WAITCALLBACK *ctx){
  AB_WAITCALLBACK *wcb;

  assert(ctx);
  wcb=GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK, AB_WAITCALLBACK, ctx);
  assert(wcb);

  return AB_WaitCallback_new(wcb->banking, GWEN_WaitCallback_GetId(ctx));
}



void AB_WaitCallback_Log(GWEN_WAITCALLBACK *ctx,
                         unsigned int level,
			 GWEN_LOGGER_LEVEL loglevel,
                         const char *s){
  AB_WAITCALLBACK *wcb;
  AB_BANKING_LOGLEVEL nl;

  assert(ctx);
  wcb=GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK, AB_WAITCALLBACK, ctx);
  assert(wcb);

  DBG_INFO(AQBANKING_LOGDOMAIN, "Logging this: %d/%d %s",
	   level, loglevel, s);

  switch(loglevel) {
  case GWEN_LoggerLevelError:     nl=AB_Banking_LogLevelError; break;
  case GWEN_LoggerLevelWarning:   nl=AB_Banking_LogLevelWarn; break;
  case GWEN_LoggerLevelNotice:    nl=AB_Banking_LogLevelNotice; break;
  default:
    DBG_NOTICE(AQBANKING_LOGDOMAIN, "Loglevel %d -> info", loglevel);
    nl=AB_Banking_LogLevelInfo;
    break;
  }

  AB_Banking_ProgressLog(wcb->banking, 0, nl, s);
}








