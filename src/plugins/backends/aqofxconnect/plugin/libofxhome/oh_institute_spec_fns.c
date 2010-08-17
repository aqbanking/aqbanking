/***************************************************************************
    begin       : Tue Aug 17 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif




int OH_InstituteSpec_ReadXml(OH_INSTITUTE_SPEC *os, GWEN_XMLNODE *node) {
  const char *s;
  int i;

  s=GWEN_XMLNode_GetProperty(node, "id", NULL);
  if (s && *s) {
    if (sscanf(s, "%d", &i)!=1) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Bad value for property \"id\": [%s]", s);
      return GWEN_ERROR_BAD_DATA;
    }
    OH_InstituteSpec_SetId(os, i);
  }
  else {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Missing property \"id\"");
    return GWEN_ERROR_BAD_DATA;
  }

  s=GWEN_XMLNode_GetProperty(node, "name", NULL);
  if (s && *s)
    OH_InstituteSpec_SetName(os, s);
  else {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Missing property \"name\"");
    return GWEN_ERROR_BAD_DATA;
  }

  return 0;
}



OH_INSTITUTE_SPEC *OH_InstituteSpec_fromXml(GWEN_XMLNODE *node) {
  OH_INSTITUTE_SPEC *os;
  int rv;

  os=OH_InstituteSpec_new();
  rv=OH_InstituteSpec_ReadXml(os, node);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    OH_InstituteSpec_free(os);
    return NULL;
  }

  return os;
}






