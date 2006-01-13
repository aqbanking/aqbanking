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

#ifndef AG_ACCOUNT_P_H
#define AG_ACCOUNT_P_H

#include "account_l.h"


typedef struct AG_ACCOUNT AG_ACCOUNT;
struct AG_ACCOUNT {
  int dummy;
};

void AG_Account_FreeData(void *bp, void *p);



#endif
