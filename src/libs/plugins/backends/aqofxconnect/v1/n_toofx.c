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


#include "n_toofx.h"
#include "aqofxconnect/common/n_utils.h"

#include "aqofxconnect/user.h"

#include <gwenhywfar/gui.h>


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int _writeToStream(const GWEN_XMLNODE *n, GWEN_BUFFER *buf, const char *encoding);
static int _writeElementToStream(const GWEN_XMLNODE *n, GWEN_BUFFER *buf, const char *encoding);
static int _writeDataToStream(const GWEN_XMLNODE *n, GWEN_BUFFER *buf, const char *encoding);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


int AO_V1_XmlToOfx(GWEN_XMLNODE *xmlNode, GWEN_BUFFER *buf, const char *encoding)
{
  int rv;

  rv=_writeElementToStream(xmlNode, buf, encoding);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}




int _writeToStream(const GWEN_XMLNODE *n, GWEN_BUFFER *buf, const char *encoding)
{
  int rv;

  assert(n);

  switch (GWEN_XMLNode_GetType(n)) {
  case GWEN_XMLNodeTypeTag:
    rv=_writeElementToStream(n, buf, encoding);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    break;
  case GWEN_XMLNodeTypeData:
    rv=_writeDataToStream(n, buf, encoding);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    break;
  default:
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Ignoring type %d", GWEN_XMLNode_GetType(n));
  }

  return 0;
}



int _writeElementToStream(const GWEN_XMLNODE *n, GWEN_BUFFER *buf, const char *encoding)
{
  int rv;
  int hasSubTags;
  GWEN_XMLNODE *c;
  const char *sData;

  sData=GWEN_XMLNode_GetData(n);

  /* write element opening ("<NAME") */
  GWEN_Buffer_AppendByte(buf, '<');
  if (sData)
    GWEN_Buffer_AppendString(buf, sData);
  else
    GWEN_Buffer_AppendString(buf, "UNKNOWN");
  GWEN_Buffer_AppendByte(buf, '>');

  hasSubTags=(GWEN_XMLNode_GetFirstTag(n)!=NULL);
  if (hasSubTags)
    GWEN_Buffer_AppendString(buf, "\r\n");

  /* write children */
  c=GWEN_XMLNode_GetChild(n);
  while (c) {
    rv=_writeToStream(c, buf, encoding);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    c=GWEN_XMLNode_Next(c);
  }

  if (hasSubTags) {
    /* write closing tag ("</NAME>") */
    GWEN_Buffer_AppendString(buf, "</");
    if (sData)
      GWEN_Buffer_AppendString(buf, sData);
    else
      GWEN_Buffer_AppendString(buf, "UNKNOWN");
    GWEN_Buffer_AppendByte(buf, '>');
  }
  GWEN_Buffer_AppendString(buf, "\r\n");

  return 0;
}



int _writeDataToStream(const GWEN_XMLNODE *n, GWEN_BUFFER *buf, const char *encoding)
{
  const char *sData;

  sData=GWEN_XMLNode_GetData(n);
  if (sData) {
    int rv=0;

    rv=GWEN_Text_ConvertCharset("UTF-8", encoding, sData, strlen(sData), buf);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}








