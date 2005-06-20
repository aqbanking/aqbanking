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


#ifndef CBANKING_CBANKING_P_H
#define CBANKING_CBANKING_P_H

#include <gwenhywfar/buffer.h>

#include "cbanking_l.h"
#include "progress_l.h"


#define CBANKING_CHAR_ABORT  27
#define CBANKING_CHAR_DELETE 127
#define CBANKING_CHAR_ENTER  10



typedef struct CBANKING CBANKING;
struct CBANKING {
  CBANKING_PROGRESS_LIST *progressList;
  GWEN_TYPE_UINT32 nextBoxId;
  GWEN_TYPE_UINT32 nextProgressId;
  char *charSet;
  GWEN_DB_NODE *dbPins;
  int nonInteractive;
};
void CBanking_FreeData(void *bp, void *p);


char CBanking__readCharFromStdin(int waitFor);

int CBanking__input(GWEN_TYPE_UINT32 flags,
                    char *buffer,
                    int minLen,
                    int maxLen);
CBANKING_PROGRESS *CBanking__findProgress(AB_BANKING *ab,
                                          GWEN_TYPE_UINT32 id);

int CBanking__ConvertFromUtf8(AB_BANKING *ab,
                              const char *text,
                              int len,
                              GWEN_BUFFER *tbuf);


int CBanking_MessageBox(AB_BANKING *ab,
                        GWEN_TYPE_UINT32 flags,
                        const char *title,
                        const char *text,
                        const char *b1,
                        const char *b2,
                        const char *b3);


int CBanking_InputBox(AB_BANKING *ab,
                      GWEN_TYPE_UINT32 flags,
                      const char *title,
                      const char *text,
                      char *buffer,
                      int minLen,
                      int maxLen);

GWEN_TYPE_UINT32 CBanking_ShowBox(AB_BANKING *ab,
                                  GWEN_TYPE_UINT32 flags,
                                  const char *title,
                                  const char *text);


void CBanking_HideBox(AB_BANKING *ab, GWEN_TYPE_UINT32 id);


GWEN_TYPE_UINT32 CBanking_ProgressStart(AB_BANKING *ab,
                                        const char *title,
                                        const char *text,
                                        GWEN_TYPE_UINT32 total);



int CBanking_ProgressAdvance(AB_BANKING *ab,
                             GWEN_TYPE_UINT32 id,
                             GWEN_TYPE_UINT32 progress);


int CBanking_ProgressLog(AB_BANKING *ab,
                         GWEN_TYPE_UINT32 id,
                         AB_BANKING_LOGLEVEL level,
                         const char *text);


int CBanking_ProgressEnd(AB_BANKING *ab, GWEN_TYPE_UINT32 id);


int CBanking_GetPin(AB_BANKING *ab,
                    GWEN_TYPE_UINT32 flags,
                    const char *token,
                    const char *title,
                    const char *text,
                    char *buffer,
                    int minLen,
                    int maxLen);



#endif

