/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Apr 05 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_ACCSTATUS_P_H
#define AQBANKING_ACCSTATUS_P_H

#include <aqbanking/accstatus.h>


struct AB_ACCOUNT_STATUS {
  GWEN_TIME *time;
  AB_VALUE *bankLine;
  AB_VALUE *disposable;
  AB_VALUE *disposed;
  AB_BALANCE *bookedBalance;
  AB_BALANCE *notedBalance;
};



#endif /* AQBANKING_ACCSTATUS_P_H */


