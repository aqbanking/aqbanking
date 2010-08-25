



#include <gwenhywfar/logger.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/dbio.h>

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/msgengine.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


int test1(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;

  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", "./aqbanking.conf", 0);

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Deinitializing AB_Banking...\n");
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Freeing AB_Banking...\n");
  AB_Banking_free(ab);

  fprintf(stderr, "Finished\n");
  return 0;
}


int test3(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;

  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", "./aqbanking.conf", 0);

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }



  fprintf(stderr, "Deinitializing AB_Banking...\n");
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Freeing AB_Banking...\n");
  AB_Banking_free(ab);

  fprintf(stderr, "Finished\n");
  return 0;
}



int test5(int argc, char **argv) {
  int rv;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbParams;

  db=GWEN_DB_Group_new("test");
  dbParams=GWEN_DB_Group_new("params");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "params/type", "mt940");
  rv=GWEN_DB_ReadFileAs(db, "test.swift", "swift", dbParams,
			GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv) {
    DBG_ERROR(0, "Error reading file");
    return 2;
  }
  GWEN_DB_Dump(db, 2);

  return 0;
}



int test6(int argc, char **argv) {
  int rv;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbParams;
  GWEN_DB_NODE *dbT;
  GWEN_DB_NODE *dbOut;
  FILE *f;
  int first=1;

  db=GWEN_DB_Group_new("test");
  dbParams=GWEN_DB_Group_new("params");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "quote", "1");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "title", "0");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "delimiter", "SPACE");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "group", "country");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/1", "v[0]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/2", "v[1]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/3", "v[2]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/4", "v[3]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/5", "v[4]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/6", "v[5]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/7", "v[6]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/8", "v[7]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/9", "v[8]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/10", "v[9]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/11", "v[10]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/12", "v[11]");

  rv=GWEN_DB_ReadFileAs(db, "test.txt", "csv", dbParams,
			GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv) {
    DBG_ERROR(0, "Error reading file");
    return 2;
  }
  GWEN_DB_Dump(db, 2);

  dbOut=GWEN_DB_Group_new("out");
  GWEN_DB_ClearGroup(dbParams, 0);
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "quote", "1");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "title", "0");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "delimiter", ";");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "group", "country");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/1", "v[0]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/2", "v[1]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/3", "v[2]");
  dbT=GWEN_DB_FindFirstGroup(db, "country");
  while(dbT) {
    int cnt;
    int i;
    GWEN_BUFFER *buf;
    GWEN_DB_NODE *dbX;

    dbX=GWEN_DB_GetGroup(dbOut, GWEN_PATH_FLAGS_CREATE_GROUP, "country");
    assert(dbX);
    buf=GWEN_Buffer_new(0, 256, 0, 1);
    for (i=0; ; i++) {
      if (GWEN_DB_GetCharValue(dbT, "v", i, 0)==0)
        break;
    }

    cnt=i;
    for (i=0; i<(cnt-3); i++) {
      if (GWEN_Buffer_GetUsedBytes(buf))
        GWEN_Buffer_AppendByte(buf, ' ');
      GWEN_Buffer_AppendString(buf, GWEN_DB_GetCharValue(dbT, "v", i, 0));
    }
    GWEN_DB_SetCharValue(dbX, GWEN_DB_FLAGS_DEFAULT, "v",
                         GWEN_Buffer_GetStart(buf));
    /* second column */
    GWEN_DB_SetCharValue(dbX, GWEN_DB_FLAGS_DEFAULT, "v",
                         GWEN_DB_GetCharValue(dbT, "v", i++, 0));
    /* fourth column */
    GWEN_DB_SetCharValue(dbX, GWEN_DB_FLAGS_DEFAULT, "v",
                         GWEN_DB_GetCharValue(dbT, "v", ++i, 0));
    dbT=GWEN_DB_FindNextGroup(dbT, "country");
  }

  rv=GWEN_DB_WriteFileAs(dbOut, "countries.csv", "csv", dbParams,
			 GWEN_DB_FLAGS_DEFAULT);
  if (rv) {
    DBG_ERROR(0, "Error writing file");
    return 2;
  }

  f=fopen("countries.c", "w+");
  assert(f);

  dbT=GWEN_DB_FindFirstGroup(db, "country");
  first=1;
  fprintf(f, "ab_country_list= {\n");
  while(dbT) {
    int cnt;
    int i;
    int j;
    GWEN_BUFFER *buf;
    GWEN_DB_NODE *dbX;

    if (first) {
      first=0;
    }
    else {
      fprintf(f, ",\n");
    }
    dbX=GWEN_DB_GetGroup(dbOut, GWEN_PATH_FLAGS_CREATE_GROUP, "country");
    assert(dbX);
    buf=GWEN_Buffer_new(0, 256, 0, 1);
    for (i=0; ; i++) {
      if (GWEN_DB_GetCharValue(dbT, "v", i, 0)==0)
        break;
    }

    cnt=i;
    for (i=0; i<(cnt-3); i++) {
      if (GWEN_Buffer_GetUsedBytes(buf))
        GWEN_Buffer_AppendByte(buf, ' ');
      GWEN_Buffer_AppendString(buf, GWEN_DB_GetCharValue(dbT, "v", i, 0));
    }
    fprintf(f, "{ I18N_NOOP(\"%s\"), ",
            GWEN_Buffer_GetStart(buf));
    fprintf(f, "\"%s\", ", GWEN_DB_GetCharValue(dbT, "v", i++, 0));

    if (sscanf(GWEN_DB_GetCharValue(dbT, "v", ++i, 0), "%d", &j)!=1) {
      fprintf(stderr, "ERROR in country %s\n",
              GWEN_Buffer_GetStart(buf));
      return 2;
    }
    fprintf(f, "%d }", j);
    dbT=GWEN_DB_FindNextGroup(dbT, "country");
  } /* while */

  fprintf(f, "\n}\n");

  if (fclose(f)) {
    fprintf(stderr, "Could not close.\n");
    return 3;
  }

  return 0;
}



int readCSVCountries(const char *fname, GWEN_DB_NODE *db) {
  int rv;
  GWEN_DB_NODE *dbParams;

  dbParams=GWEN_DB_Group_new("params");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "quote", "1");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "title", "0");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "delimiter", "SPACE");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "group", "country");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/1", "v[0]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/2", "v[1]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/3", "v[2]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/4", "v[3]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/5", "v[4]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/6", "v[5]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/7", "v[6]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/8", "v[7]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/9", "v[8]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/10", "v[9]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/11", "v[10]");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "columns/12", "v[11]");

  rv=GWEN_DB_ReadFileAs(db, fname, "csv", dbParams,
			GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv) {
    DBG_ERROR(0, "Error reading file");
    return 2;
  }

  return 0;
}




int readXMLCountries(const char *fname, GWEN_DB_NODE *dbCountries) {
  GWEN_XMLNODE *nRoot;
  GWEN_XMLNODE *nRow;

  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Info);
  nRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  if (GWEN_XML_ReadFile(nRoot, fname,
			GWEN_XML_FLAGS_DEFAULT |
			GWEN_XML_FLAGS_HANDLE_HEADERS)) {
    DBG_ERROR(0, "Could not read XML file.\n");
    return 2;
  }

  nRow=GWEN_XMLNode_FindFirstTag(nRoot, "tr", 0, 0);
  while(nRow) {
    GWEN_XMLNODE *nCol;
    GWEN_DB_NODE *dbCountry=0;

    dbCountry=GWEN_DB_Group_new("country");
    nCol=GWEN_XMLNode_FindFirstTag(nRow, "td", 0, 0);
    if (nCol) {
      GWEN_XMLNODE *nData;

      nData=GWEN_XMLNode_GetFirstData(nCol);
      if (nData) {
	GWEN_BUFFER *dbuf;
        const char *s;
	char *p;

        dbuf=GWEN_Buffer_new(0, 256, 0, 1);
	s=GWEN_XMLNode_GetData(nData);
	assert(s);
	if (GWEN_Text_UnescapeXmlToBuffer(s, dbuf)) {
	  fprintf(stderr, "Error unescaping country \"%s\"", s);
	  return 2;
	}
	p=GWEN_Buffer_GetStart(dbuf);
	if (strlen(p)>1) {
	  p=strchr(p, '(');
	  if (p) {
	    *p=0;
	    GWEN_Text_CondenseBuffer(dbuf);
	  }
	  if (strcasecmp(GWEN_Buffer_GetStart(dbuf), "&nbsp;")!=0)
	    GWEN_DB_SetCharValue(dbCountry, GWEN_DB_FLAGS_DEFAULT,
				 "countryName", GWEN_Buffer_GetStart(dbuf));
	}
        GWEN_Buffer_free(dbuf);
      }
    }

    if (nCol) nCol=GWEN_XMLNode_FindNextTag(nCol, "td", 0, 0);
    if (nCol) nCol=GWEN_XMLNode_FindNextTag(nCol, "td", 0, 0);
    if (nCol) {
      GWEN_XMLNODE *nData;

      nData=GWEN_XMLNode_GetFirstData(nCol);
      if (nData) {
	GWEN_BUFFER *dbuf;
        const char *s;
	char *p;

        dbuf=GWEN_Buffer_new(0, 256, 0, 1);
	s=GWEN_XMLNode_GetData(nData);
	assert(s);
	if (GWEN_Text_UnescapeXmlToBuffer(s, dbuf)) {
	  fprintf(stderr, "Error unescaping country code \"%s\"", s);
	  return 2;
	}
	p=GWEN_Buffer_GetStart(dbuf);
	if (strlen(p)>1) {
	  if (strcasecmp(GWEN_Buffer_GetStart(dbuf), "&nbsp;")!=0) {
	    p[2]=0;
	    GWEN_DB_SetCharValue(dbCountry, GWEN_DB_FLAGS_DEFAULT,
				 "countryCode", GWEN_Buffer_GetStart(dbuf));
	  }
	}
	GWEN_Buffer_free(dbuf);
      }
    }

    if (nCol) nCol=GWEN_XMLNode_FindNextTag(nCol, "td", 0, 0);
    if (nCol) nCol=GWEN_XMLNode_FindNextTag(nCol, "td", 0, 0);
    if (nCol) {
      GWEN_XMLNODE *nData;

      nData=GWEN_XMLNode_GetFirstData(nCol);
      if (nData) {
	GWEN_BUFFER *dbuf;
        const char *s;
	char *p;

        dbuf=GWEN_Buffer_new(0, 256, 0, 1);
	s=GWEN_XMLNode_GetData(nData);
	assert(s);
	if (GWEN_Text_UnescapeXmlToBuffer(s, dbuf)) {
	  fprintf(stderr, "Error unescaping currency name \"%s\"", s);
	  return 2;
	}
	p=GWEN_Buffer_GetStart(dbuf);
	if (strlen(p)>1) {
	  p=strchr(p, '(');
	  if (p) {
	    *p=0;
	    GWEN_Text_CondenseBuffer(dbuf);
	  }
	  if (strcasecmp(GWEN_Buffer_GetStart(dbuf), "&nbsp;")!=0)
	    GWEN_DB_SetCharValue(dbCountry, GWEN_DB_FLAGS_DEFAULT,
				 "currencyName", GWEN_Buffer_GetStart(dbuf));
	}
        GWEN_Buffer_free(dbuf);
      }
    }

    if (nCol) nCol=GWEN_XMLNode_FindNextTag(nCol, "td", 0, 0);
    if (nCol) nCol=GWEN_XMLNode_FindNextTag(nCol, "td", 0, 0);
    if (nCol) {
      GWEN_XMLNODE *nData;

      nData=GWEN_XMLNode_GetFirstData(nCol);
      if (nData) {
	GWEN_BUFFER *dbuf;
        const char *s;
	char *p;

        dbuf=GWEN_Buffer_new(0, 256, 0, 1);
	s=GWEN_XMLNode_GetData(nData);
	assert(s);
	if (GWEN_Text_UnescapeXmlToBuffer(s, dbuf)) {
	  fprintf(stderr, "Error unescaping currency code \"%s\"", s);
	  return 2;
	}
	p=GWEN_Buffer_GetStart(dbuf);
	if (strlen(p)>2) {
	  if (strcasecmp(GWEN_Buffer_GetStart(dbuf), "&nbsp;")!=0) {
	    p[3]=0;
	    GWEN_DB_SetCharValue(dbCountry, GWEN_DB_FLAGS_DEFAULT,
				 "currencyCode", GWEN_Buffer_GetStart(dbuf));
	  }
	}
        GWEN_Buffer_free(dbuf);
      }
    }

    if (GWEN_DB_VariableExists(dbCountry, "countryName") &&
	GWEN_DB_VariableExists(dbCountry, "countryCode") &&
	GWEN_DB_VariableExists(dbCountry, "currencyName") &&
	GWEN_DB_VariableExists(dbCountry, "currencyCode")) {
      GWEN_DB_AddGroup(dbCountries, dbCountry);
    }
    else {
      GWEN_DB_Group_free(dbCountry);
    }

    nRow=GWEN_XMLNode_FindNextTag(nRow, "tr", 0, 0);
  }

  return 0;
}



int test7(int argc, char **argv) {
  const char *fname;
  GWEN_DB_NODE *dbCountries;
  int rv;

  if (argc<3) {
    fprintf(stderr, "Filename for input needed\n");
    return 1;
  }
  fname=argv[2];

  dbCountries=GWEN_DB_Group_new("countries");
  rv=readXMLCountries(fname, dbCountries);
  if (rv)
    return rv;

  GWEN_DB_Dump(dbCountries, 2);
  return 0;
}



int packCsvCountries(GWEN_DB_NODE *dbCSV) {
  GWEN_DB_NODE *dbT;

  dbT=GWEN_DB_FindFirstGroup(dbCSV, "country");
  while(dbT) {
    int cnt;
    int i;
    int j;
    GWEN_BUFFER *buf;

    buf=GWEN_Buffer_new(0, 256, 0, 1);
    for (i=0; ; i++) {
      if (GWEN_DB_GetCharValue(dbT, "v", i, 0)==0)
        break;
    }

    cnt=i;
    for (i=0; i<(cnt-3); i++) {
      if (GWEN_Buffer_GetUsedBytes(buf))
        GWEN_Buffer_AppendByte(buf, ' ');
      GWEN_Buffer_AppendString(buf, GWEN_DB_GetCharValue(dbT, "v", i, 0));
    }
    GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "countryName",
			 GWEN_Buffer_GetStart(buf));
    GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "countryCode",
			 GWEN_DB_GetCharValue(dbT, "v", i++, 0));
    if (sscanf(GWEN_DB_GetCharValue(dbT, "v", ++i, 0), "%d", &j)!=1) {
      fprintf(stderr, "ERROR in country %s\n",
	      GWEN_Buffer_GetStart(buf));
      return 2;
    }
    else {
      GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "countryNum", j);
    }
    GWEN_DB_DeleteVar(dbT, "v");

    dbT=GWEN_DB_FindNextGroup(dbT, "country");
  } /* while */

  return 0;
}




int test8(int argc, char **argv) {
  const char *fnameXML, *fnameCSV;
  GWEN_DB_NODE *dbXML;
  GWEN_DB_NODE *dbCSV;
  GWEN_DB_NODE *dbC;
  GWEN_DB_NODE *dbT;
  FILE *f;
  int first;
  int rv;

  if (argc<4) {
    fprintf(stderr, "Filenames for input needed (XML, CSV)\n");
    return 1;
  }
  fnameXML=argv[2];
  fnameCSV=argv[3];

  dbXML=GWEN_DB_Group_new("countries");
  dbCSV=GWEN_DB_Group_new("countries");

  rv=readCSVCountries(fnameCSV, dbCSV);
  if (rv) {
    fprintf(stderr, "Error reading CSV countries\n");
    return rv;
  }
  rv=packCsvCountries(dbCSV);
  if (rv) {
    fprintf(stderr, "Error packing CSV countries\n");
    return rv;
  }
  //GWEN_DB_Dump(dbCSV, stderr, 4);

  rv=readXMLCountries(fnameXML, dbXML);
  if (rv)
    return rv;

  dbC=GWEN_DB_GetFirstGroup(dbXML);
  while(dbC) {
    const char *code;

    code=GWEN_DB_GetCharValue(dbC, "countryCode", 0, 0);
    if (code) {
      dbT=GWEN_DB_GetFirstGroup(dbCSV);
      while(dbT) {
	const char *s;

	s=GWEN_DB_GetCharValue(dbT, "countryCode", 0, 0);
	if (s) {
	  if (strcasecmp(s, code)==0)
	    break;
	}
	dbT=GWEN_DB_GetNextGroup(dbT);
      }

      if (!dbT) {
        DBG_ERROR(0, "Country \"%s\" not found", code);
      }
      else {
	int nc;

	nc=GWEN_DB_GetIntValue(dbT, "countryNum", 0, 0);
	if (nc) {
	  DBG_ERROR(0, "Setting country code %s=%d", code, nc);
	  GWEN_DB_SetIntValue(dbC, GWEN_DB_FLAGS_OVERWRITE_VARS,
			      "countryNum", nc);
	}
	else {
	  DBG_ERROR(0, "Country \"%s\" has no number", code);
	  GWEN_DB_Dump(dbT, 2);
	}
      }
    }
    else {
      DBG_ERROR(0, "No country code");
    }

    dbC=GWEN_DB_GetNextGroup(dbC);
  }

  f=fopen("countries2.c", "w+");
  assert(f);

  dbT=GWEN_DB_FindFirstGroup(dbXML, "country");
  first=1;
  fprintf(f, "ab_country_list= {\n");
  while(dbT) {
    const char *s;
    int i;

    if (first) {
      first=0;
    }
    else {
      fprintf(f, ",\n");
    }

    i=GWEN_DB_GetIntValue(dbT, "countryNum", 0, 0);
    if (i!=280) {
      if (i==276) {
	s=GWEN_DB_GetCharValue(dbT, "countryName", 0, 0);
	assert(s);
	fprintf(f, "{ I18N_NOOP(\"%s\"), ",
		s);
	s=GWEN_DB_GetCharValue(dbT, "countryCode", 0, 0);
	assert(s);
	fprintf(f, "\"%s\",", s);
	i=GWEN_DB_GetIntValue(dbT, "countryNum", 0, 0);
	fprintf(f, " %d,", 280);
	s=GWEN_DB_GetCharValue(dbT, "currencyName", 0, 0);
	assert(s);
	fprintf(f, " I18N_NOOP(\"%s\"), ", s);
	s=GWEN_DB_GetCharValue(dbT, "currencyCode", 0, 0);
	assert(s);
	fprintf(f, " \"%s\" }", s);
	fprintf(f, ",\n");
      }

      s=GWEN_DB_GetCharValue(dbT, "countryName", 0, 0);
      assert(s);
      fprintf(f, "{ I18N_NOOP(\"%s\"), ",
	      s);
      s=GWEN_DB_GetCharValue(dbT, "countryCode", 0, 0);
      assert(s);
      fprintf(f, "\"%s\",", s);
      fprintf(f, " %d,", i);
      s=GWEN_DB_GetCharValue(dbT, "currencyName", 0, 0);
      assert(s);
      fprintf(f, " I18N_NOOP(\"%s\"), ", s);
      s=GWEN_DB_GetCharValue(dbT, "currencyCode", 0, 0);
      assert(s);
      fprintf(f, " \"%s\" }", s);
    }

    dbT=GWEN_DB_FindNextGroup(dbT, "country");
  } /* while */

  fprintf(f, "\n}\n");

  if (fclose(f)) {
    fprintf(stderr, "Could not close.\n");
    return 3;
  }


  GWEN_DB_Dump(dbXML, 2);
  return 0;
}



int test9(int argc, char **argv) {
  AB_BANKING *ab;
  AB_BANKINFO_LIST2 *bl;
  AB_BANKINFO_LIST2_ITERATOR *bit;
  AB_BANKINFO *tbi;
  int rv;

  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", 0, 0);

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }

  bl=AB_BankInfo_List2_new();
  tbi=AB_BankInfo_new();
  AB_BankInfo_SetLocation(tbi, "Wilhelmshaven");
  rv=AB_Banking_GetBankInfoByTemplate(ab, "de", tbi, bl);
  if (rv) {
    fprintf(stderr, "Error looking for bank info: %d\n", rv);
    return 2;
  }

  bit=AB_BankInfo_List2_First(bl);
  if (bit) {
    AB_BANKINFO *bi;
    int count=0;

    fprintf(stdout, "Found the following banks:\n");
    bi=AB_BankInfo_List2Iterator_Data(bit);
    assert(bi);
    while(bi) {
      count++;
      fprintf(stdout, "%5d %s %s %s\n",
              count,
              AB_BankInfo_GetBankId(bi),
              AB_BankInfo_GetBankName(bi),
              AB_BankInfo_GetLocation(bi));
      bi=AB_BankInfo_List2Iterator_Next(bit);
    }
    AB_BankInfo_List2Iterator_free(bit);
  }
  AB_BankInfo_List2_free(bl);

  fprintf(stderr, "Deinitializing AB_Banking...\n");
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Freeing AB_Banking...\n");
  AB_Banking_free(ab);

  fprintf(stderr, "Finished\n");
  return 0;
}


int test10(int argc, char **argv) {
  AB_BANKING *ab;
  AB_BANKINFO_LIST2 *bl;
  AB_BANKINFO_LIST2_ITERATOR *bit;
  AB_BANKINFO *tbi;
  int rv;

  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", 0, 0);

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }

  bl=AB_BankInfo_List2_new();
  tbi=AB_BankInfo_new();
  AB_BankInfo_SetBankId(tbi, "28250110");
  rv=AB_Banking_GetBankInfoByTemplate(ab, "de", tbi, bl);
  if (rv) {
    fprintf(stderr, "Error looking for bank info: %d\n", rv);
    return 2;
  }

  bit=AB_BankInfo_List2_First(bl);
  if (bit) {
    AB_BANKINFO *bi;
    int count=0;

    fprintf(stdout, "Found the following banks:\n");
    bi=AB_BankInfo_List2Iterator_Data(bit);
    assert(bi);
    while(bi) {
      count++;
      fprintf(stdout, "%5d %s %s %s\n",
              count,
              AB_BankInfo_GetBankId(bi),
              AB_BankInfo_GetBankName(bi),
              AB_BankInfo_GetLocation(bi));
      bi=AB_BankInfo_List2Iterator_Next(bit);
    }
    AB_BankInfo_List2Iterator_free(bit);
  }
  AB_BankInfo_List2_free(bl);

  fprintf(stderr, "Deinitializing AB_Banking...\n");
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Freeing AB_Banking...\n");
  AB_Banking_free(ab);

  fprintf(stderr, "Finished\n");
  return 0;
}



int test11(int argc, char **argv) {
  AB_BANKING *ab;
  AB_BANKINFO_LIST2 *bl;
  AB_BANKINFO_LIST2_ITERATOR *bit;
  AB_BANKINFO *tbi;
  int rv;

  GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Notice);
  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", 0, 0);

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }

  bl=AB_BankInfo_List2_new();
  tbi=AB_BankInfo_new();
  rv=AB_Banking_GetBankInfoByTemplate(ab, "de", tbi, bl);
  if (rv) {
    fprintf(stderr, "Error looking for bank info: %d\n", rv);
    return 2;
  }

  bit=AB_BankInfo_List2_First(bl);
  if (bit) {
    AB_BANKINFO *bi;
    int count=0;

    fprintf(stdout, "Found the following banks:\n");
    bi=AB_BankInfo_List2Iterator_Data(bit);
    assert(bi);
    while(bi) {
      count++;
      fprintf(stdout, "%5d %s %s %s\n",
              count,
              AB_BankInfo_GetBankId(bi),
              AB_BankInfo_GetBankName(bi),
              AB_BankInfo_GetLocation(bi));
      bi=AB_BankInfo_List2Iterator_Next(bit);
    }
    AB_BankInfo_List2Iterator_free(bit);
  }
  AB_BankInfo_List2_free(bl);
  AB_BankInfo_free(tbi);

  fprintf(stderr, "Deinitializing AB_Banking...\n");
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Freeing AB_Banking...\n");
  AB_Banking_free(ab);

  fprintf(stderr, "Finished\n");
  return 0;
}



int test12(int argc, char **argv) {
  AB_BANKING *ab;
  AB_BANKINFO_LIST2 *bl;
  AB_BANKINFO_LIST2_ITERATOR *bit;
  AB_BANKINFO *tbi;
  int rv;
  const char *country;

  GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Error);
  if (argc<3) {
    fprintf(stderr, "Country code needed.\n");
    return 1;
  }
  country=argv[2];

  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", 0, 0);

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }

  bl=AB_BankInfo_List2_new();
  tbi=AB_BankInfo_new();
  rv=AB_Banking_GetBankInfoByTemplate(ab, country, tbi, bl);
  if (rv) {
    fprintf(stderr, "Error looking for bank info: %d\n", rv);
    return 2;
  }

  bit=AB_BankInfo_List2_First(bl);
  if (bit) {
    AB_BANKINFO *bi;
    int count=0;

    fprintf(stdout, "Found the following banks:\n");
    bi=AB_BankInfo_List2Iterator_Data(bit);
    assert(bi);
    while(bi) {
      count++;
      fprintf(stdout, "%5d %s %s %s\n",
              count,
              AB_BankInfo_GetBankId(bi),
              AB_BankInfo_GetBankName(bi),
              AB_BankInfo_GetLocation(bi));
      bi=AB_BankInfo_List2Iterator_Next(bit);
    }
    AB_BankInfo_List2Iterator_free(bit);
  }
  AB_BankInfo_List2_free(bl);
  AB_BankInfo_free(tbi);

  fprintf(stderr, "Deinitializing AB_Banking...\n");
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Freeing AB_Banking...\n");
  AB_Banking_free(ab);

  fprintf(stderr, "Finished\n");
  return 0;
}



int test13(int argc, char **argv) {
  int rv;
  const char *iban;

  GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Error);
  if (argc<3) {
    fprintf(stderr, "IBAN needed.\n");
    return 1;
  }
  iban=argv[2];

  /*
  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", 0);

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }
  */

  rv=AB_Banking_CheckIban(iban);
  fprintf(stderr, "Result for IBAN \"%s\": %d\n",
          iban, rv);

  /*
  fprintf(stderr, "Deinitializing AB_Banking...\n");
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Freeing AB_Banking...\n");
  AB_Banking_free(ab);
  */

  fprintf(stderr, "Finished\n");
  return 0;
}



int test15(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;
  GWEN_DBIO *dbio;
  const char *fname;
  GWEN_DB_NODE *dbParams;
  GWEN_DB_NODE *dbData;

  GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Error);

  if (argc<3) {
    fprintf(stderr, "Usage: %s FILENAME\n", argv[0]);
    return 1;
  }
  fname=argv[2];

  fprintf(stderr, "Creating AB_Banking...\n");
  ab=AB_Banking_new("abtest", 0, 0);

  fprintf(stderr, "Initializing AB_Banking...\n");
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }

  dbio=GWEN_DBIO_GetPlugin("swift");
  if (dbio==NULL) {
    fprintf(stderr, "SWIFT plugin not found.\n");
    return 2;
  }

  dbParams=GWEN_DB_Group_new("params");
  dbData=GWEN_DB_Group_new("data");

  rv=GWEN_DBIO_ImportFromFile(dbio, fname,
			      dbData,
			      dbParams,
			      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv) {
    fprintf(stderr, "Error on import (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Deinitializing AB_Banking...\n");
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Freeing AB_Banking...\n");
  AB_Banking_free(ab);

  GWEN_DB_Dump(dbData, 2);

  fprintf(stderr, "Finished\n");
  return 0;
}



int testDate(int argc, char **argv) {
  GWEN_TIME *ti;
  int y,m,d;

  if (argc<4) {
    fprintf(stderr, "Usage: %s msg XMLFILE DATAFILE\n", argv[0]);
    return 1;
  }
  ti=AB_ImExporter_DateFromString(argv[2], argv[3], 1);
  assert(ti);
  GWEN_Time_GetBrokenDownDate(ti, &d, &m, &y);
  DBG_ERROR(0, "Date: %02d %02d %04d", d, m, y);
  return 0;
}



int main(int argc, char **argv) {
  const char *cmd;
  int rv;

  if (argc<2) {
    fprintf(stderr, "Usage: %s COMMAND\n", argv[0]);
    return 1;
  }

  cmd=argv[1];

  GWEN_Logger_SetLevel(AQBANKING_LOGDOMAIN, GWEN_LoggerLevel_Info);

  if (strcasecmp(cmd, "test1")==0)
    rv=test1(argc, argv);
  else if (strcasecmp(cmd, "test3")==0)
    rv=test3(argc, argv);
  else if (strcasecmp(cmd, "test5")==0)
    rv=test5(argc, argv);
  else if (strcasecmp(cmd, "test6")==0)
    rv=test6(argc, argv);
  else if (strcasecmp(cmd, "test7")==0)
    rv=test7(argc, argv);
  else if (strcasecmp(cmd, "test8")==0)
    rv=test8(argc, argv);
  else if (strcasecmp(cmd, "test9")==0)
    rv=test9(argc, argv);
  else if (strcasecmp(cmd, "test10")==0)
    rv=test10(argc, argv);
  else if (strcasecmp(cmd, "test11")==0)
    rv=test11(argc, argv);
  else if (strcasecmp(cmd, "test12")==0)
    rv=test12(argc, argv);
  else if (strcasecmp(cmd, "test13")==0)
    rv=test13(argc, argv);
  else if (strcasecmp(cmd, "test15")==0)
    rv=test15(argc, argv);
  else if (strcasecmp(cmd, "date")==0)
    rv=testDate(argc, argv);
  else {
    fprintf(stderr, "Unknown command \"%s\"", cmd);
    rv=1;
  }

  return rv;
}

