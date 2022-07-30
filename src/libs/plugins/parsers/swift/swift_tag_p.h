/***************************************************************************
 begin       : Fri Apr 02 2004
 copyright   : (C) 2004-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCIBANK_SWIFT_TAG_P_H
#define AQHBCIBANK_SWIFT_TAG_P_H

#include "swift_tag.h"


struct AHB_SWIFT_TAG {
  GWEN_LIST_ELEMENT(AHB_SWIFT_TAG);
  char *id;
  char *content;
};


struct AHB_SWIFT_SUBTAG {
  GWEN_LIST_ELEMENT(AHB_SWIFT_SUBTAG);
  int id;
  char *content;
};



#endif /* AQHBCIBANK_SWIDT_TAG_H */

