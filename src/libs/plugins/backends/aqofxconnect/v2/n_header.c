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



/*
 *

 */

GWEN_XMLNODE *AO_V2_MkOfxHeader(AB_USER *u)
{
  GWEN_XMLNODE *xmlOFX;
  const char *s;

  xmlOFX=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "OFX");

  GWEN_XMLNode_SetProperty(xmlOFX, "OFXHEADER", "200");

  s=AO_User_GetHeaderVer(u);
  GWEN_XMLNode_SetProperty(xmlOFX, "VERSION", s?s:"100");

  s=AO_User_GetSecurityType(u);
  GWEN_XMLNode_SetProperty(xmlOFX, "SECURITY", s?s:"NONE");

  if (1) {
    GWEN_TIME *ti;
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    ti=GWEN_CurrentTime();
    assert(ti);
    if (AO_User_GetFlags(u) & AO_USER_FLAGS_SEND_SHORT_DATE)
      GWEN_Time_toString(ti, "YYYYMMDDhhmmss", tbuf);
    else
      GWEN_Time_toString(ti, "YYYYMMDDhhmmss.000", tbuf);
    GWEN_Buffer_AppendString(buf, "\r\n");

    GWEN_XMLNode_SetCharValue(xmlSignonRq, "NEWFILEUID", GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
    GWEN_Time_free(ti);
  }


  return xmlOfx;
}



