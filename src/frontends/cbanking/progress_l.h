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


#ifndef CBANKING_PROGRESS_L_H
#define CBANKING_PREGRESS_L_H


#include <aqbanking/banking.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/misc.h>


typedef struct CBANKING_PROGRESS CBANKING_PROGRESS;
GWEN_LIST_FUNCTION_DEFS(CBANKING_PROGRESS, CBankingProgress)


CBANKING_PROGRESS *CBankingProgress_new(AB_BANKING *ab,
                                        GWEN_TYPE_UINT32 id,
                                        const char *title,
                                        const char *text,
                                        GWEN_TYPE_UINT32 total);

void CBankingProgress_free(CBANKING_PROGRESS *pr);

int CBankingProgress_Advance(CBANKING_PROGRESS *pr,
                             GWEN_TYPE_UINT32 progress);

int CBankingProgress_Log(CBANKING_PROGRESS *pr,
                         AB_BANKING_LOGLEVEL level,
                         const char *text);

int CBankingProgress_End(CBANKING_PROGRESS *pr);


GWEN_TYPE_UINT32 CBankingProgress_GetId(const CBANKING_PROGRESS *pr);
const char *CBankingProgress_GetTitle(const CBANKING_PROGRESS *pr);
const char *CBankingProgress_GetText(const CBANKING_PROGRESS *pr);
GWEN_TYPE_UINT32 CBankingProgress_GetTotal(const CBANKING_PROGRESS *pr);
GWEN_TYPE_UINT32 CBankingProgress_GetCurrent(const CBANKING_PROGRESS *pr);

const char *CBankingProgress_GetLog(const CBANKING_PROGRESS *pr);


#endif

