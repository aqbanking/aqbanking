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

#ifndef GB_PROGRESS_L_H
#define GB_PROGRESS_L_H


#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>



GtkWidget *GB_Progress_new(AB_BANKING *ab, GWEN_TYPE_UINT32 id);

int GB_Progress_Start(GtkWidget *w,
                      const char *title,
                      const char *text,
                      GWEN_TYPE_UINT32 total);
int GB_Progress_Advance(GtkWidget *w, GWEN_TYPE_UINT32 progress);
int GB_Progress_Log(GtkWidget *w,
                    AB_BANKING_LOGLEVEL level,
                    const char *text);
int GB_Progress_End(GtkWidget *w);


GWEN_TYPE_UINT32 GB_Progress_GetId(GtkWidget *w);










#endif



