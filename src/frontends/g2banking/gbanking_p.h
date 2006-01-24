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

#include "gbanking_l.h"
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

  char *charSet;
};
static void GBanking_FreeData(void *bp, void *p);

static GtkWidget*
  GBanking__findSimpleBox(AB_BANKING *ab, GWEN_TYPE_UINT32 id);
static GtkWidget*
  GBanking__findProgressWidget(AB_BANKING *ab, GWEN_TYPE_UINT32 id);

static int GBanking_MessageBox(AB_BANKING *ab,
                               GWEN_TYPE_UINT32 flags,
                               const char *title,
                               const char *text,
                               const char *b1,
                               const char *b2,
                               const char *b3);

static int GBanking_InputBox(AB_BANKING *ab,
                             GWEN_TYPE_UINT32 flags,
                             const char *title,
                             const char *text,
                             char *buffer,
                             int minLen,
                             int maxLen);

static GWEN_TYPE_UINT32 GBanking_ShowBox(AB_BANKING *ab,
                                         GWEN_TYPE_UINT32 flags,
                                         const char *title,
                                         const char *text);


static void GBanking_HideBox(AB_BANKING *ab, GWEN_TYPE_UINT32 id);


static GWEN_TYPE_UINT32 GBanking_ProgressStart(AB_BANKING *ab,
                                               const char *title,
                                               const char *text,
                                               GWEN_TYPE_UINT32 total);


static int GBanking_ProgressAdvance(AB_BANKING *ab,
                                    GWEN_TYPE_UINT32 id,
                                    GWEN_TYPE_UINT32 progress);



static int GBanking_ProgressLog(AB_BANKING *ab,
                                GWEN_TYPE_UINT32 id,
                                AB_BANKING_LOGLEVEL level,
                                const char *text);


static int GBanking_ProgressEnd(AB_BANKING *ab, GWEN_TYPE_UINT32 id);


static int GBanking__convertFromUtf8(AB_BANKING *ab,
                                     const char *text,
                                     int len,
                                     GWEN_BUFFER *tbuf);



#endif /* GBANKING_P_H */









