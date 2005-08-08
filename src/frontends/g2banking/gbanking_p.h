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



#ifndef GBANKING_P_H
#define GBANKING_P_H

#include "gbanking.h"
#include <gtk/gtk.h>
#include <glib.h>

#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>


typedef struct GBANKING GBANKING;
struct GBANKING {
  GObject parent;

  GWEN_TYPE_UINT32 lastWidgetId;
  GtkWidget *parentWidget;
  GSList *progressWidgets;
  GSList *simpleBoxes;

  GWEN_TYPE_UINT32 _lastAccountUpdate;
  GWEN_TYPE_UINT32 _lastQueueUpdate;

  GBANKING_IMPORTCONTEXT_FN importContextFn;
};
void GBanking_FreeData(void *bp, void *p);

GtkWidget *GBanking__findSimpleBox(AB_BANKING *ab, GWEN_TYPE_UINT32 id);
GtkWidget *GBanking__findProgressWidget(AB_BANKING *ab, GWEN_TYPE_UINT32 id);


int GBanking_MessageBox(AB_BANKING *ab,
                          GWEN_TYPE_UINT32 flags,
                          const char *title,
                          const char *text,
                          const char *b1,
                          const char *b2,
                          const char *b3);

int GBanking_InputBox(AB_BANKING *ab,
                        GWEN_TYPE_UINT32 flags,
                        const char *title,
                        const char *text,
                        char *buffer,
                        int minLen,
                        int maxLen);

GWEN_TYPE_UINT32 GBanking_ShowBox(AB_BANKING *ab,
                                    GWEN_TYPE_UINT32 flags,
                                    const char *title,
                                    const char *text);


void GBanking_HideBox(AB_BANKING *ab, GWEN_TYPE_UINT32 id);


GWEN_TYPE_UINT32 GBanking_ProgressStart(AB_BANKING *ab,
                                          const char *title,
                                          const char *text,
                                          GWEN_TYPE_UINT32 total);


int GBanking_ProgressAdvance(AB_BANKING *ab,
                               GWEN_TYPE_UINT32 id,
                               GWEN_TYPE_UINT32 progress);



int GBanking_ProgressLog(AB_BANKING *ab,
                           GWEN_TYPE_UINT32 id,
                           AB_BANKING_LOGLEVEL level,
                           const char *text);


int GBanking_ProgressEnd(AB_BANKING *ab, GWEN_TYPE_UINT32 id);


int GBanking__extractText(const char *text, GWEN_BUFFER *tbuf);



#endif /* GBANKING_P_H */









