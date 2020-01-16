/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "n_utils.h"
#include "aqofxconnect/user.h"




void AO_V2_Util_SetDateValue(GWEN_XMLNODE *xmlNode, const GWEN_DATE *da, uint32_t userFlags, const char *varName)
{
  if (da) {
    GWEN_BUFFER *tbuf;
  
    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    if (userFlags & AO_USER_FLAGS_SEND_SHORT_DATE)
      GWEN_Date_toStringWithTemplate(da, "YYYYMMDD000000", tbuf);
    else
      GWEN_Date_toStringWithTemplate(da, "YYYYMMDD000000.000", tbuf);
    GWEN_XMLNode_SetCharValue(xmlNode, varName, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
}



void AO_V2_Util_SetTimeValue(GWEN_XMLNODE *xmlNode, const GWEN_TIME *ti, uint32_t userFlags, const char *varName)
{
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new(0, 32, 0, 1);
  if (userFlags & AO_USER_FLAGS_SEND_SHORT_DATE)
    GWEN_Time_toString(ti, "YYYYMMDDhhmmss", tbuf);
  else
    GWEN_Time_toString(ti, "YYYYMMDDhhmmss.000", tbuf);

  GWEN_XMLNode_SetCharValue(xmlNode, varName, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



void AO_V2_Util_SetCurrentTimeValue(GWEN_XMLNODE *xmlNode, uint32_t userFlags, const char *varName)
{
  GWEN_TIME *ti;

  ti=GWEN_CurrentTime();
  assert(ti);
  AO_V2_Util_SetTimeValue(xmlNode, ti, userFlags, varName);
  GWEN_Time_free(ti);
}

