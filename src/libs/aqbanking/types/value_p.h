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


#ifndef AB_VALUE_P_H
#define AB_VALUE_P_H

#include "value.h"

#include <gmp.h>


/** Internal structure of AB_VALUE -- do not access this directly! */
struct AB_VALUE {
  GWEN_LIST_ELEMENT(AB_VALUE)

  mpq_t value;
  char *currency;
};


static void AB_Value__toString(const AB_VALUE *v, GWEN_BUFFER *buf);


#endif /* AB_VALUE_P_H */








