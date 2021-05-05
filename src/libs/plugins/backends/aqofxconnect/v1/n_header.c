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


#include "n_header.h"
#include "aqofxconnect/common/n_utils.h"

#include "aqofxconnect/user.h"

#include <gwenhywfar/gui.h>




/*
 *

 */

int AO_V1_AddOfxHeaders(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *buf, const char *encoding)
{
  GWEN_TIME *ti;
  const char *s;

  ti=GWEN_CurrentTime();
  assert(ti);

  GWEN_Buffer_AppendString(buf, "OFXHEADER:100\r\n");
  GWEN_Buffer_AppendString(buf, "DATA:OFXSGML\r\n");

  GWEN_Buffer_AppendString(buf, "VERSION:");
  s=AO_User_GetHeaderVer(u);
  if (!s || !*s)
    s="102";
  GWEN_Buffer_AppendString(buf, s);
  GWEN_Buffer_AppendString(buf, "\r\n");

  s=AO_User_GetSecurityType(u);
  if (!s || !*s)
    s="NONE";
  GWEN_Buffer_AppendString(buf, "SECURITY:");
  GWEN_Buffer_AppendString(buf, s);
  GWEN_Buffer_AppendString(buf, "\r\n");

  GWEN_Buffer_AppendString(buf, "ENCODING:");
  GWEN_Buffer_AppendString(buf, encoding?encoding:"USASCII");
  GWEN_Buffer_AppendString(buf, "\r\n");
  GWEN_Buffer_AppendString(buf,
                           "CHARSET:1252\r\n"
                           "COMPRESSION:NONE\r\n"
                           "OLDFILEUID:NONE\r\n"
                           "NEWFILEUID:NONE\r\n");

  /* header finished */
  GWEN_Buffer_AppendString(buf, "\r\n");

  /* cleanup */
  GWEN_Time_free(ti);

  /* done */
  return 0;
}



