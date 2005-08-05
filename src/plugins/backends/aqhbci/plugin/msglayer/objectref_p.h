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

#ifndef AH_OBJECTREF_P_H
#define AH_OBJECTREF_P_H


#include <aqhbci/objectref.h>



struct AH_OBJECTREF {
  GWEN_LIST_ELEMENT(AH_OBJECTREF);
  char *type;
  int country;
  char *bankId;
  char *accountId;
  char *userId;
  char *customerId;
};



#endif /* AH_OBJECTREF_P_H */



