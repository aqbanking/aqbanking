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

#ifndef GBANKING_INPUTBOX_H
#define GBANKING_INPUTBOX_H

#include <gtk/gtk.h>

#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>



gboolean GBanking_GetInput(AB_BANKING *ab,
                           GWEN_TYPE_UINT32 flags,
                           const char *title,
                           const char *text,
                           char *buffer,
                           int minLen,
                           int maxLen,
                           GtkWidget *parent);


#endif /* GBANKING_INPUTBOX_H */



