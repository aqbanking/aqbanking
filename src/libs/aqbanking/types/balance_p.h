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


#ifndef AQBANKING_BALANCE_P_H
#define AQBANKING_BALANCE_P_H

#include <aqbanking/balance.h>


struct AB_BALANCE {
  AB_VALUE *value;
  GWEN_TIME *time;
};



#endif /* AQBANKING_BALANCE_P_H */


