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


#ifndef AQBANKING_VALUE_P_H
#define AQBANKING_VALUE_P_H

#include <aqbanking/value.h>

/** Internal structure of AB_VALUE -- do not access this directly! */
struct AB_VALUE {
  double value;
  char *currency;
  int isValid;
};



#endif /* AQBANKING_VALUE_P_H */


