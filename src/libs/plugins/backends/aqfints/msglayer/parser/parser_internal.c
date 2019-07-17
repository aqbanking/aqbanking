/***************************************************************************
 begin       : Fri Jun 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "parser_normalize.h"

#include <gwenhywfar/text.h>




int AQFINTS_Parser_IsCharType(const char *sType)
{
  if (sType && *sType) {
    if (strcasecmp(sType, "AN")==0 ||
        strcasecmp(sType, "float")==0 ||
        strcasecmp(sType, "alpha")==0 ||
        strcasecmp(sType, "ascii")==0)
      return 1;
  }
  return 0;
}



int AQFINTS_Parser_IsIntType(const char *sType)
{
  if (sType && *sType) {
    if (strcasecmp(sType, "num")==0)
      return 1;
  }
  return 0;
}



int AQFINTS_Parser_IsBinType(const char *sType)
{
  if (sType && *sType) {
    if (strcasecmp(sType, "bin")==0)
      return 1;
  }
  return 0;
}





