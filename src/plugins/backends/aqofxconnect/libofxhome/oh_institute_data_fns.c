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



int OH_InstituteData_ReadXml(OH_INSTITUTE_DATA *oh, GWEN_XMLNODE *node) {
  const char *s;
  int i;

  s=GWEN_XMLNode_GetProperty(node, "id", NULL);
  if (s && *s) {
    if (sscanf(s, "%d", &i)!=1) {
      DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "Bad value for property \"id\": [%s]", s);
      return GWEN_ERROR_BAD_DATA;
    }
    OH_InstituteData_SetId(oh, i);
  }

  s=GWEN_XMLNode_GetCharValue(node, "name", NULL);
  OH_InstituteData_SetName(oh, s);

  s=GWEN_XMLNode_GetCharValue(node, "fid", NULL);
  OH_InstituteData_SetFid(oh, s);

  s=GWEN_XMLNode_GetCharValue(node, "org", NULL);
  OH_InstituteData_SetOrg(oh, s);

  s=GWEN_XMLNode_GetCharValue(node, "brokerId", NULL);
  OH_InstituteData_SetBrokerId(oh, s);

  s=GWEN_XMLNode_GetCharValue(node, "url", NULL);
  OH_InstituteData_SetUrl(oh, s);

  i=GWEN_XMLNode_GetIntValue(node, "ofxfail", 0);
  if (i!=0)
    OH_InstituteData_AddFlags(oh, OH_INSTITUTE_DATA_FLAGS_OFXFAIL);
  else
    OH_InstituteData_SubFlags(oh, OH_INSTITUTE_DATA_FLAGS_OFXFAIL);

  i=GWEN_XMLNode_GetIntValue(node, "sslfail", 0);
  if (i!=0)
    OH_InstituteData_AddFlags(oh, OH_INSTITUTE_DATA_FLAGS_SSLFAIL);
  else
    OH_InstituteData_SubFlags(oh, OH_INSTITUTE_DATA_FLAGS_SSLFAIL);

  s=GWEN_XMLNode_GetCharValue(node, "lastofxvalidation", NULL);
  if (s && *s) {
    GWEN_TIME *ti;

    ti=GWEN_Time_fromString(s, "YYYY-MM-DD hh:mm:ss");
    if (ti) {
      OH_InstituteData_SetLastOfxValidationTime(oh, ti);
      GWEN_Time_free(ti);
    }
  }

  s=GWEN_XMLNode_GetCharValue(node, "lastsslvalidation", NULL);
  if (s && *s) {
    GWEN_TIME *ti;

    ti=GWEN_Time_fromString(s, "YYYY-MM-DD hh:mm:ss");
    if (ti) {
      OH_InstituteData_SetLastSslValidationTime(oh, ti);
      GWEN_Time_free(ti);
    }
  }

  return 0;
}



OH_INSTITUTE_DATA *OH_InstituteData_fromXml(GWEN_XMLNODE *node) {
  OH_INSTITUTE_DATA *oh;
  int rv;

  oh=OH_InstituteData_new();
  rv=OH_InstituteData_ReadXml(oh, node);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    OH_InstituteData_free(oh);
    return NULL;
  }

  return oh;
}








