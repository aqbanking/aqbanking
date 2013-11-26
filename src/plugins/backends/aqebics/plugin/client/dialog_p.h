/***************************************************************************
 $RCSfile: medium_p.h,v $
                             -------------------
    cvs         : $Id: medium_p.h,v 1.3 2006/01/23 05:16:27 aquamaniac Exp $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef EBC_CLIENT_DIALOG_P_H
#define EBC_CLIENT_DIALOG_P_H

#include "dialog_l.h"



typedef struct EBC_DIALOG EBC_DIALOG;
struct EBC_DIALOG {
  int dummy;
};

static GWENHYWFAR_CB void EBC_Dialog_FreeData(void *bp, void *p);





#endif

