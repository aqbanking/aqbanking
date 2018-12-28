/***************************************************************************
    begin       : Sat Mar 08 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_MSG_EU_P_H
#define AQEBICS_MSG_EU_P_H


#include "eu.h"


struct EB_EU {
  GWEN_LIST_ELEMENT(EB_EU)
  char *version;
  int modLen;
  char *jobType;
  uint8_t *signaturePtr;
  uint32_t signatureLen;
  char *userId;
  char *originalFileName;
  GWEN_TIME *creationTime;
  GWEN_TIME *signatureTime;
};


static void copyTrimmedString(const uint8_t *p, uint32_t l, char **pDst);



#endif

