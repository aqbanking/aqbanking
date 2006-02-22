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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "aqhbci_l.h"

#include <string.h>



AH_CRYPT_MODE AH_CryptMode_fromString(const char *s) {
  if (strcasecmp(s, "none")==0)
    return AH_CryptMode_None;
  else if (strcasecmp(s, "ddv")==0)
    return AH_CryptMode_Ddv;
  else if (strcasecmp(s, "pintan")==0)
    return AH_CryptMode_Pintan;
  else if (strcasecmp(s, "rdh")==0)
    return AH_CryptMode_Rdh;
  else
    return AH_CryptMode_Unknown;
}



const char *AH_CryptMode_toString(AH_CRYPT_MODE v) {
  switch(v) {
  case AH_CryptMode_None:   return "none";
  case AH_CryptMode_Ddv:    return "ddv";
  case AH_CryptMode_Pintan: return "pintan";
  case AH_CryptMode_Rdh:    return "rdh";
  default:                  return "unknown";
  }
}

