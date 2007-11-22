/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_COUNTRY_P_H
#define AQBANKING_COUNTRY_P_H

#include "country_l.h"


struct AB_COUNTRY {
  const char *name;
  const char *code;
  int numericCode;
  const char *currencyName;
  const char *currencyCode;
};



#endif /* AQBANKING_COUNTRY_P_H */
