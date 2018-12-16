/***************************************************************************
    begin       : Sun Dec 16 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <ctype.h>




AB_IMEXPORTER_XML_CONTEXT *AB_ImExporterXML_Context_new(GWEN_XMLNODE *documentRoot, GWEN_DB_NODE *dbRoot) {
  AB_IMEXPORTER_XML_CONTEXT *ctx;

  GWEN_NEW_OBJECT(AB_IMEXPORTER_XML_CONTEXT, ctx);
  assert(ctx);

  ctx->docRoot=documentRoot;
  ctx->xmlNodeStack=GWEN_XMLNode_List2_new();
  ctx->dbRoot=dbRoot;
  ctx->tempDbRoot=GWEN_DB_Group_new("dbTempRoot");

  ctx->currentDbGroup=ctx->dbRoot;
  ctx->currentTempDbGroup=ctx->tempDbRoot;
  ctx->currentDocNode=documentRoot;

  return ctx;
}



void AB_ImExporterXML_Context_free(AB_IMEXPORTER_XML_CONTEXT *ctx) {
  if (ctx) {
    GWEN_XMLNode_List2_free(ctx->xmlNodeStack);
    ctx->xmlNodeStack=NULL;

    GWEN_DB_Group_free(ctx->tempDbRoot);
    GWEN_FREE_OBJECT(ctx);
  }
}



void AB_ImExporterXML_Context_EnterDocNode(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  assert(ctx);
  assert(xmlNode);

  GWEN_XMLNode_List2_PushBack(ctx->xmlNodeStack, ctx->currentDocNode);
  ctx->currentDocNode=xmlNode;
}



void AB_ImExporterXML_Context_LeaveDocNode(AB_IMEXPORTER_XML_CONTEXT *ctx) {
  GWEN_XMLNODE *xmlNode;

  assert(ctx);

  xmlNode=GWEN_XMLNode_List2_GetBack(ctx->xmlNodeStack);
  if (xmlNode==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Nothing on stack");
    assert(xmlNode);
  }
  ctx->currentDocNode=xmlNode;
  GWEN_XMLNode_List2_PopBack(ctx->xmlNodeStack);
}





int AB_ImExporterXML_Import_ReplaceVars(const char *s, GWEN_DB_NODE *db, GWEN_BUFFER *dbuf) {
  const char *p;

  p=s;
  while(*p) {
    if (*p=='$') {
      p++;
      if (*p=='$')
	GWEN_Buffer_AppendByte(dbuf, '$');
      else if (*p=='(') {
	const char *pStart;

	p++;
	pStart=p;
	while(*p && *p!=')')
	  p++;
	if (*p!=')') {
	  DBG_ERROR(GWEN_LOGDOMAIN, "Unterminated variable name in code");
	  return GWEN_ERROR_BAD_DATA;
	}
	else {
          int len;
	  char *name;
	  const char *valueString;
	  int valueInt;
	  char numbuf[32];
	  int rv;

	  len=p-pStart;
	  if (len<1) {
	    DBG_ERROR(GWEN_LOGDOMAIN, "Empty variable name in code");
	    return GWEN_ERROR_BAD_DATA;
	  }
	  name=(char*) malloc(len+1);
	  assert(name);
	  memmove(name, pStart, len);
          name[len]=0;

	  switch(GWEN_DB_GetVariableType(db, name)) {
	  case GWEN_DB_NodeType_ValueInt:
	    valueInt=GWEN_DB_GetIntValue(db, name, 0, 0);
	    rv=GWEN_Text_NumToString(valueInt, numbuf, sizeof(numbuf)-1, 0);
	    if (rv>=0)
	      GWEN_Buffer_AppendString(dbuf, numbuf);
	    break;
	  case GWEN_DB_NodeType_ValueChar:
	    valueString=GWEN_DB_GetCharValue(db, name, 0, NULL);
	    if (valueString)
	      GWEN_Buffer_AppendString(dbuf, valueString);
#if 0 /* just replace with empty value */
	    else {
	      GWEN_Buffer_AppendString(dbuf, " [__VALUE OF ");
	      GWEN_Buffer_AppendString(dbuf, name);
	      GWEN_Buffer_AppendString(dbuf, " WAS NOT SET__] ");
	    }
#endif
	    break;

	  default:
	    break;
	  }
	  free(name);
	}
      }
      else {
	DBG_ERROR(GWEN_LOGDOMAIN, "Bad variable string in code");
        return GWEN_ERROR_BAD_DATA;
      }
      p++;
    }
    else {
      if (*p=='#') {
	/* let # lines begin on a new line */
	GWEN_Buffer_AppendByte(dbuf, '\n');
	GWEN_Buffer_AppendByte(dbuf, *p);

	/* skip introducing cross and copy all stuff until the next cross
	 * upon which wa inject a newline (to make the preprocessor happy)
	 */
	p++;
	while(*p && *p!='#') {
	  GWEN_Buffer_AppendByte(dbuf, *p);
	  p++;
	}
	if (*p=='#') {
	  GWEN_Buffer_AppendByte(dbuf, '\n');
	  p++;
	}
      }
      else if (*p=='\\') {
	/* check for recognized control escapes */
	if (tolower(p[1])=='n') {
	  GWEN_Buffer_AppendByte(dbuf, '\n');
	  p+=2; /* skip introducing backslash and control character */
	}
	else if (tolower(p[1])=='t') {
	  GWEN_Buffer_AppendByte(dbuf, '\t');
	  p+=2; /* skip introducing backslash and control character */
	}
	else if (tolower(p[1])=='\\') {
	  GWEN_Buffer_AppendByte(dbuf, '\\');
	  p+=2; /* skip introducing backslash and control character */
	}
	else {
	  /* no known escape character, just add literally */
	  GWEN_Buffer_AppendByte(dbuf, *p);
	  p++;
	}
      }
      else {
	GWEN_Buffer_AppendByte(dbuf, *p);
	p++;
      }
    }
  }

  return 0;
}



const char *AB_ImExporterXML_Import_GetCharValueByPath(GWEN_XMLNODE *xmlNode, const char *path, const char *defValue) {
  const char *s;

  s=strchr(path, '@');
  if (s) {
    int idx;
    char *cpyOfPath;
    char *property;
    GWEN_XMLNODE *n;


    idx=s-path;
    cpyOfPath=strdup(path);
    assert(cpyOfPath);
    cpyOfPath[idx]=0;
    property=cpyOfPath+idx+1;

    if (*cpyOfPath) {
      n=GWEN_XMLNode_GetNodeByXPath(xmlNode, cpyOfPath, GWEN_PATH_FLAGS_PATHMUSTEXIST);
      DBG_INFO(AQBANKING_LOGDOMAIN, "Looking for path \"%s\" (%s)", cpyOfPath, n?"found":"not found");
    }
    else
      n=xmlNode;

    if (n) {
      const char *result;

      result=GWEN_XMLNode_GetProperty(n, property, defValue);
      free(cpyOfPath);
      return result;
    }
    free(cpyOfPath);
    return defValue;
  }
  else
    return GWEN_XMLNode_GetCharValueByPath(xmlNode, path, defValue);
}





int AB_ImExporterXML_Import_Handle_Enter(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  const char *path;
  GWEN_XMLNODE *n;
  int rv;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (path==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing path in \"EnterPath\"");
    return GWEN_ERROR_INVALID;
  }

  n=GWEN_XMLNode_GetNodeByXPath(ctx->currentDocNode, path, GWEN_PATH_FLAGS_PATHMUSTEXIST);
  if (n==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Path \"%s\" does not exist", path);
    return GWEN_ERROR_INVALID;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN, "Entering node \"%s\"", path);

  /* enter given document node */
  AB_ImExporterXML_Context_EnterDocNode(ctx, n);

  rv=AB_ImExporterXML_Import_HandleChildren(ctx, xmlNode);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN, "Leaving node \"%s\"", path);

  /* leave given document node, re-select previously active one, thus restoring status from the beginning */
  AB_ImExporterXML_Context_LeaveDocNode(ctx);
  return 0;
}



int AB_ImExporterXML_Import_Handle_ForEvery(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  const char *path;
  GWEN_XMLNODE *n;
  int rv;

  path=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (path==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing name in \"ForEvery\"");
    return GWEN_ERROR_INVALID;
  }

  n=GWEN_XMLNode_FindFirstTag(ctx->currentDocNode, path, NULL, NULL);
  while (n){

    DBG_INFO(AQBANKING_LOGDOMAIN, "Entering node \"%s\"", path);

    /* enter given document node */
    AB_ImExporterXML_Context_EnterDocNode(ctx, n);

    /* handle all children of this parser XML node with the current document node */
    rv=AB_ImExporterXML_Import_HandleChildren(ctx, xmlNode);

    DBG_INFO(AQBANKING_LOGDOMAIN, "Leaving node \"%s\"", path);

    /* leave given document node, re-select previously active one, thus restoring status from the beginning */
    AB_ImExporterXML_Context_LeaveDocNode(ctx);

    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    n=GWEN_XMLNode_FindNextTag(n, path, NULL, NULL);
  }

  return 0;
}



int AB_ImExporterXML_Import_Handle_CreateAndEnterDbGroup(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  const char *name;
  GWEN_DB_NODE *dbLast;
  int rv;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty name in \"CreateAnEnterDbGroup\"");
    return GWEN_ERROR_INVALID;
  }

  /* push group */
  dbLast=ctx->currentDbGroup;

  /* create group */
  ctx->currentDbGroup=GWEN_DB_GetGroup(dbLast, GWEN_PATH_FLAGS_CREATE_GROUP, name);

  /* handle children (nothing special here) */
  rv=AB_ImExporterXML_Import_HandleChildren(ctx, xmlNode);

  /* pop group */
  ctx->currentDbGroup=dbLast;

  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_ImExporterXML_Import_Handle_CreateAndEnterTempDbGroup(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  const char *name;
  GWEN_DB_NODE *dbLast;
  int rv;

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty name in \"CreateAnEnterTempDbGroup\"");
    return GWEN_ERROR_INVALID;
  }

  /* push group */
  dbLast=ctx->currentTempDbGroup;

  /* create group */
  ctx->currentTempDbGroup=GWEN_DB_GetGroup(dbLast, GWEN_PATH_FLAGS_CREATE_GROUP, name);
  assert(ctx->currentTempDbGroup);

  /* handle children (nothing special here) */
  rv=AB_ImExporterXML_Import_HandleChildren(ctx, xmlNode);

  /* delete temp group */
  GWEN_DB_UnlinkGroup(ctx->currentTempDbGroup);
  GWEN_DB_Group_free(ctx->currentTempDbGroup);

  /* pop group */
  ctx->currentTempDbGroup=dbLast;

  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_ImExporterXML_Import_Handle_SetCharValue(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  const char *name;
  const char *value;
  int doTrim=0;

  doTrim=GWEN_XMLNode_GetIntProperty(xmlNode, "trim", 0);

  DBG_INFO(AQBANKING_LOGDOMAIN, "SetCharValue entered.");

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty name in \"SetCharValue\"");
    return GWEN_ERROR_INVALID;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN, "Name of dbVar: \"%s\"", name);

  value=GWEN_XMLNode_GetProperty(xmlNode, "value", NULL);
  if (value) {
    GWEN_BUFFER *dbuf;
    int rv;

    DBG_INFO(AQBANKING_LOGDOMAIN, "Value for dbVar \"%s\" is \"%s\"", name, value);

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=AB_ImExporterXML_Import_ReplaceVars(value, ctx->currentTempDbGroup, dbuf);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(dbuf);
      return rv;
    }
    GWEN_Buffer_free(dbuf);
  }
  else {
    const char *path;

    path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
    if (!(path && *path)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty path in \"SetCharValue\"");
      return GWEN_ERROR_INVALID;
    }

    value=AB_ImExporterXML_Import_GetCharValueByPath(ctx->currentDocNode, path, NULL);
    if (value && *value) {
      if (doTrim) {
        GWEN_BUFFER *dbuf;

        dbuf=GWEN_Buffer_new(0, 256, 0, 1);
        GWEN_Buffer_AppendString(dbuf, value);
        GWEN_Text_CondenseBuffer(dbuf);
        DBG_INFO(AQBANKING_LOGDOMAIN, "Setting dbVar %s = %s", name, GWEN_Buffer_GetStart(dbuf));
        GWEN_DB_SetCharValue(ctx->currentDbGroup, GWEN_DB_FLAGS_DEFAULT, name, GWEN_Buffer_GetStart(dbuf));
        GWEN_Buffer_free(dbuf);
      }
      else {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Setting dbVar %s = %s", name, value);
        GWEN_DB_SetCharValue(ctx->currentDbGroup, GWEN_DB_FLAGS_DEFAULT, name, value);
      }
    }
    else {
      GWEN_BUFFER *tbuf;

      tbuf=GWEN_Buffer_new(0, 256, 0, 1);

      GWEN_XMLNode_GetXPath(NULL, ctx->currentDocNode, tbuf);

      DBG_INFO(AQBANKING_LOGDOMAIN, "No value in path \"%s\" (%s)", path, GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);

      GWEN_XMLNode_Dump(ctx->currentDocNode, 2);
    }
  }

  return 0;
}



int AB_ImExporterXML_Import_Handle_SetTempCharValue(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  const char *name;
  const char *value;
  int doTrim=0;

  doTrim=GWEN_XMLNode_GetIntProperty(xmlNode, "trim", 0);

  DBG_INFO(AQBANKING_LOGDOMAIN, "SetCharValue entered.");

  name=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(name && *name)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty name in \"SetCharValue\"");
    return GWEN_ERROR_INVALID;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN, "Name of dbVar: \"%s\"", name);

  value=GWEN_XMLNode_GetProperty(xmlNode, "value", NULL);
  if (value) {
    GWEN_BUFFER *dbuf;
    int rv;

    DBG_INFO(AQBANKING_LOGDOMAIN, "Value for dbVar \"%s\" is \"%s\"", name, value);

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=AB_ImExporterXML_Import_ReplaceVars(value, ctx->currentTempDbGroup, dbuf);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(dbuf);
      return rv;
    }
    GWEN_Buffer_free(dbuf);
  }
  else {
    const char *path;

    path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
    if (!(path && *path)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty path in \"SetCharValue\"");
      return GWEN_ERROR_INVALID;
    }

    value=AB_ImExporterXML_Import_GetCharValueByPath(ctx->currentDocNode, path, NULL);
    if (value && *value) {
      if (doTrim) {
        GWEN_BUFFER *dbuf;

        dbuf=GWEN_Buffer_new(0, 256, 0, 1);
        GWEN_Buffer_AppendString(dbuf, value);
        GWEN_Text_CondenseBuffer(dbuf);
        DBG_INFO(AQBANKING_LOGDOMAIN, "Setting dbVar %s = %s", name, GWEN_Buffer_GetStart(dbuf));
        GWEN_DB_SetCharValue(ctx->currentTempDbGroup, GWEN_DB_FLAGS_DEFAULT, name, GWEN_Buffer_GetStart(dbuf));
        GWEN_Buffer_free(dbuf);
      }
      else {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Setting dbVar %s = %s", name, value);
        GWEN_DB_SetCharValue(ctx->currentTempDbGroup, GWEN_DB_FLAGS_DEFAULT, name, value);
      }
    }
  }

  return 0;
}



int AB_ImExporterXML_Import_Handle_IfCharDataMatches(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  const char *pattern;
  const char *path;
  const char *value;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty path in \"IfCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  pattern=GWEN_XMLNode_GetProperty(xmlNode, "pattern", NULL);
  if (!(pattern && *pattern)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty pattern in \"IfCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  value=AB_ImExporterXML_Import_GetCharValueByPath(ctx->currentDocNode, path, NULL);
  if (value) {
    if (-1!=GWEN_Text_ComparePattern(value, pattern, 0)) {
      int rv;

      /* pattern matches, handle children  */
      rv=AB_ImExporterXML_Import_HandleChildren(ctx, xmlNode);
      if (rv<0) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
  }

  return 0;
}



int AB_ImExporterXML_Import_Handle_IfNotCharDataMatches(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  const char *pattern;
  const char *path;
  const char *value;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty path in \"IfNotCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  pattern=GWEN_XMLNode_GetProperty(xmlNode, "pattern", NULL);
  if (!(pattern && *pattern)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty pattern in \"IfNotCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  value=AB_ImExporterXML_Import_GetCharValueByPath(ctx->currentDocNode, path, NULL);
  if (value) {
    if (-1==GWEN_Text_ComparePattern(value, pattern, 0)) {
      int rv;

      /* pattern doesnt match, handle children  */
      rv=AB_ImExporterXML_Import_HandleChildren(ctx, xmlNode);
      if (rv<0) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
	return rv;
      }
    }
  }

  return 0;
}



int AB_ImExporterXML_Import_Handle_IfHasCharData(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  const char *path;
  const char *value;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty path in \"IfCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  value=AB_ImExporterXML_Import_GetCharValueByPath(ctx->currentDocNode, path, NULL);
  if (value && *value) {
    int rv;

    /* there is a value, handle children  */
    rv=AB_ImExporterXML_Import_HandleChildren(ctx, xmlNode);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int AB_ImExporterXML_Import_Handle_IfNotHasCharData(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  const char *path;
  const char *value;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty path in \"IfNotCharDataMatches\"");
    return GWEN_ERROR_INVALID;
  }

  value=AB_ImExporterXML_Import_GetCharValueByPath(ctx->currentDocNode, path, NULL);
  if (!(value && *value)) {
    int rv;

    /* there is a value, handle children  */
    rv=AB_ImExporterXML_Import_HandleChildren(ctx, xmlNode);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int AB_ImExporterXML_Import_Handle_IfPathExists(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  const char *path;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty path in \"IfPathExists\"");
    return GWEN_ERROR_INVALID;
  }

  if (GWEN_XMLNode_GetNodeByXPath(ctx->currentDocNode, path, GWEN_PATH_FLAGS_PATHMUSTEXIST)) {
    int rv;

    /* path exists, handle children  */
    rv=AB_ImExporterXML_Import_HandleChildren(ctx, xmlNode);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Path \"%s\" does not exist", path);
  }

  return 0;
}



int AB_ImExporterXML_Import_Handle_IfNotPathExists(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  const char *path;

  path=GWEN_XMLNode_GetProperty(xmlNode, "path", NULL);
  if (!(path && *path)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing or empty path in \"IfNotPathExists\"");
    return GWEN_ERROR_INVALID;
  }

  if (NULL==GWEN_XMLNode_GetNodeByXPath(ctx->currentDocNode, path, GWEN_PATH_FLAGS_PATHMUSTEXIST)) {
    int rv;

    /* path does not exist, handle children  */
    rv=AB_ImExporterXML_Import_HandleChildren(ctx, xmlNode);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Path \"%s\" exists", path);
  }

  return 0;
}




int AB_ImExporterXML_Import_HandleChildren(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode) {
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_GetFirstTag(xmlNode);
  while(n) {
    const char *name;

    name=GWEN_XMLNode_GetData(n);
    if (name && *name) {
      int rv;

      DBG_INFO(AQBANKING_LOGDOMAIN, "Handling element \"%s\"", name);
      if (strcasecmp(name, "Enter")==0)
	rv=AB_ImExporterXML_Import_Handle_Enter(ctx, n);
      else if (strcasecmp(name, "ForEvery")==0)
	rv=AB_ImExporterXML_Import_Handle_ForEvery(ctx, n);
      else if (strcasecmp(name, "CreateAndEnterDbGroup")==0)
	rv=AB_ImExporterXML_Import_Handle_CreateAndEnterDbGroup(ctx, n);
      else if (strcasecmp(name, "CreateAndEnterTempDbGroup")==0)
	rv=AB_ImExporterXML_Import_Handle_CreateAndEnterTempDbGroup(ctx, n);
      else if (strcasecmp(name, "SetCharValue")==0)
	rv=AB_ImExporterXML_Import_Handle_SetCharValue(ctx, n);
      else if (strcasecmp(name, "SetTempCharValue")==0)
	rv=AB_ImExporterXML_Import_Handle_SetTempCharValue(ctx, n);
      else if (strcasecmp(name, "IfCharDataMatches")==0)
	rv=AB_ImExporterXML_Import_Handle_IfCharDataMatches(ctx, n);
      else if (strcasecmp(name, "IfNotCharDataMatches")==0)
	rv=AB_ImExporterXML_Import_Handle_IfNotCharDataMatches(ctx, n);
      else if (strcasecmp(name, "IfHasCharData")==0)
	rv=AB_ImExporterXML_Import_Handle_IfHasCharData(ctx, n);
      else if (strcasecmp(name, "IfNotHasCharData")==0)
	rv=AB_ImExporterXML_Import_Handle_IfNotHasCharData(ctx, n);
      else if (strcasecmp(name, "IfPathExists")==0)
	rv=AB_ImExporterXML_Import_Handle_IfPathExists(ctx, n);
      else if (strcasecmp(name, "IfNotPathExists")==0)
	rv=AB_ImExporterXML_Import_Handle_IfNotPathExists(ctx, n);
      else {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown element \"%s\", aborting", name);
	return GWEN_ERROR_INVALID;
      }
      if (rv<0) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in element \"%s\", aborting", name);
	return rv;
      }
    }

    n=GWEN_XMLNode_GetNextTag(n);
  }

  return 0;
}




int AB_ImExporterXML_ImportToDb(GWEN_XMLNODE *xmlNodeDocument,
				GWEN_XMLNODE *xmlNodeSchema,
				GWEN_DB_NODE *dbDestination) {
  AB_IMEXPORTER_XML_CONTEXT *ctx;
  int rv;

  ctx=AB_ImExporterXML_Context_new(xmlNodeDocument, dbDestination);
  rv=AB_ImExporterXML_Import_HandleChildren(ctx, xmlNodeSchema);
  AB_ImExporterXML_Context_free(ctx);

  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



