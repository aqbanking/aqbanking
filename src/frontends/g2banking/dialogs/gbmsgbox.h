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

#ifndef GB_MSGBOX_H
#define GB_MSGBOX_H

#include <gtk/gtk.h>

#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>



GtkWidget *GB_MsgBox_new(AB_BANKING *ab,
                         GWEN_TYPE_UINT32 flags,
                         const char *title,
                         const char *text,
                         const char *b1,
                         const char *b2,
                         const char *b3,
                         GtkWidget *parent);



#endif /* GB_MSGBOX_H */



