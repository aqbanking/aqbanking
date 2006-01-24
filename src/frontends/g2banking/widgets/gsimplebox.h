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

#ifndef GBANKING_SIMPLEBOX_H
#define GBANKING_SIMPLEBOX_H

#include <gtk/gtk.h>

#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>


GtkWidget *GBanking_SimpleBox_new(AB_BANKING *ab,
                                  GWEN_TYPE_UINT32 id,
                                  GWEN_TYPE_UINT32 flags,
                                  const char *title,
                                  const char *text);
GWEN_TYPE_UINT32 GBanking_SimpleBox_GetId(GtkWidget *w);






#endif



