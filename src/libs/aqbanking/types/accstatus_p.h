/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Apr 05 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_ACCSTATUS_P_H
#define AQBANKING_ACCSTATUS_P_H

#include "accstatus_l.h"


struct AB_ACCOUNT_STATUS {
  GWEN_LIST_ELEMENT(AB_ACCOUNT_STATUS)
  GWEN_TIME *time;
  AB_VALUE *bankLine;
  AB_VALUE *disposable;
  AB_VALUE *disposed;
  AB_BALANCE *bookedBalance;
  AB_BALANCE *notedBalance;
};


static AB_ACCOUNT_STATUS*
AB_AccountStatus_List2__freeAll_cb(AB_ACCOUNT_STATUS *as,
                                   void *user_data);



#endif /* AQBANKING_ACCSTATUS_P_H */


