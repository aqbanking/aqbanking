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

#include <aqgeldkarte/account.h>


struct AG_ACCOUNT {
  char *cardId;
};

void AG_Account_FreeData(void *bp, void *p);



#endif
