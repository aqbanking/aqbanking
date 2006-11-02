/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: banking_p.h 764 2006-01-13 14:00:00Z cstim $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQ_BANKING_CPP_P_H
#define AQ_BANKING_CPP_P_H


#include <aqbanking/banking.h>
#include "banking.h"

#include <gwenhywfar/inherit.h>


class Banking_Linker {
  friend class Banking;

  static int MessageBox(AB_BANKING *ab,
                        GWEN_TYPE_UINT32 flags,
                        const char *title,
                        const char *text,
                        const char *b1,
                        const char *b2,
                        const char *b3);
  static int InputBox(AB_BANKING *ab,
                      GWEN_TYPE_UINT32 flags,
                      const char *title,
                      const char *text,
                      char *buffer,
                      int minLen,
                      int maxLen);
  static GWEN_TYPE_UINT32 ShowBox(AB_BANKING *ab,
                                  GWEN_TYPE_UINT32 flags,
                                  const char *title,
                                  const char *text);
  static void HideBox(AB_BANKING *ab,GWEN_TYPE_UINT32 id);
  static GWEN_TYPE_UINT32 ProgressStart(AB_BANKING *ab,
                                        const char *title,
                                        const char *text,
                                        GWEN_TYPE_UINT32 total);
  static int ProgressAdvance(AB_BANKING *ab,
                             GWEN_TYPE_UINT32 id,
                             GWEN_TYPE_UINT32 progress);
  static int ProgressLog(AB_BANKING *ab,
                         GWEN_TYPE_UINT32 id,
                         AB_BANKING_LOGLEVEL level,
                         const char *text);
  static int ProgressEnd(AB_BANKING *ab, GWEN_TYPE_UINT32 id);
  static int Print(AB_BANKING *ab,
                   const char *docTitle,
                   const char *docType,
                   const char *descr,
                   const char *text);
  static void freeData(void *bp, void *p);
};




#endif /* AQ_BANKING_CPP_P_H */


