/***************************************************************************
 begin       : Fri Jun 29 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#include <gwenhywfar/db.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/base64.h>

#include <gwenhywfar/logger.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/dbio.h>

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>



int importData(AB_BANKING *ab,
               AB_IMEXPORTER_CONTEXT *ctx,
               const char *importerName,
               const char *profileName,
               GWEN_BUFFER *dataBuf) {
  int rv;

  /*GWEN_Buffer_Dump(dataBuf, 2);*/
  rv=AB_Banking_ImportFromBufferLoadProfile(ab, importerName, ctx,
                                            profileName, NULL,
                                            (const uint8_t*) GWEN_Buffer_GetStart(dataBuf),
                                            GWEN_Buffer_GetUsedBytes(dataBuf));
  if (rv<0) {
    fprintf(stderr, "Error importing data: %d\n", rv);
    return rv;
  }

  return 0;
}


int testFile(AB_BANKING *ab, const char *fname) {
  int rv;
  GWEN_XMLNODE *doc;
  GWEN_XMLNODE *n;
  const char *data;
  const char *importerName=NULL;
  const char *profileName=NULL;
  GWEN_BUFFER *dataBuf=NULL;
  GWEN_DB_NODE *dbExpected=NULL;
  AB_IMEXPORTER_CONTEXT *ctxImported;
  AB_IMEXPORTER_CONTEXT *ctxReference;

  doc=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  rv=GWEN_XML_ReadFile(doc, fname, GWEN_XML_FLAGS_DEFAULT | GWEN_XML_FLAGS_KEEP_CNTRL);
  if (rv<0) {
    fprintf(stderr, "Error reading XML file: %d\n", rv);
    return rv;
  }

  /*GWEN_XMLNode_Dump(doc, 2);*/

  n=GWEN_XMLNode_FindFirstTag(doc, "importtest", NULL, NULL);
  if (n==NULL) {
    fprintf(stderr, "importtest element not found.\n");
    return 2;
  }

  importerName=GWEN_XMLNode_GetProperty(n, "importer", NULL);
  profileName=GWEN_XMLNode_GetProperty(n, "profile", NULL);

  /* read testdata */
  if (!(importerName && *importerName && profileName && *profileName)) {
    fprintf(stderr, "importtest missing importer name and/or profile name.\n");
    return 2;
  }


  data=GWEN_XMLNode_GetCharValue(n, "testdata", NULL);
  if (!(data && *data)) {
    fprintf(stderr, "importtest missing testdata.\n");
    return 2;
  }

  dataBuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=GWEN_Base64_Decode((const unsigned char*)data, strlen(data), dataBuf);
  if (rv<0) {
    fprintf(stderr, "testdata is not base64 encoded (%d).\n", rv);
    return 2;
  }
  GWEN_Buffer_Rewind(dataBuf);

  /* now dataBuffer, importerName and profileName are set */

  /* read expected results */
  data=GWEN_XMLNode_GetCharValue(n, "expectedresult", NULL);
  if (!(data && *data)) {
    fprintf(stderr, "importtest missing testdata.\n");
    return 2;
  }

  dbExpected=GWEN_DB_Group_new("expectedResult");
  rv=GWEN_DB_ReadFromString(dbExpected, data, strlen(data), GWEN_DB_FLAGS_DEFAULT | GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv<0) {
    fprintf(stderr, "Error reading DB of expected results: %d\n", rv);
    return rv;
  }

  ctxReference=AB_ImExporterContext_fromDb(dbExpected);
  if (ctxReference==NULL) {
    fprintf(stderr, "Error readinng expected context.\n");
    return 2;
  }

  ctxImported=AB_ImExporterContext_new();
  rv=importData(ab, ctxImported, importerName, profileName, dataBuf);
  if (rv<0) {
    fprintf(stderr, "Error importing testdata: %d\n", rv);
    return rv;
  }

  if (1) {
    GWEN_DB_NODE *dbReference;
    GWEN_DB_NODE *dbImported;
    GWEN_BUFFER *buf1;
    GWEN_BUFFER *buf2;
    const char *s1;
    const char *s2;
  
    dbReference=GWEN_DB_Group_new("reference");
    dbImported=GWEN_DB_Group_new("imported");
    AB_ImExporterContext_toDb(ctxReference, dbReference);
    AB_ImExporterContext_toDb(ctxImported, dbImported);

    buf1=GWEN_Buffer_new(0, 256, 0, 1);
    buf2=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_DB_WriteToBuffer(dbReference, buf1, GWEN_DB_FLAGS_DEFAULT);
    if (rv<0) {
      fprintf(stderr, "Error writing reference DB to buffer: %d\n", rv);
      return rv;
    }
    rv=GWEN_DB_WriteToBuffer(dbImported, buf2, GWEN_DB_FLAGS_DEFAULT);
    if (rv<0) {
      fprintf(stderr, "Error writing imported DB to buffer: %d\n", rv);
      return rv;
    }

    s1=GWEN_Buffer_GetStart(buf1);
    s2=GWEN_Buffer_GetStart(buf2);
    if (strcmp(s1, s2)!=0) {
      fprintf(stderr, "Contexts don't match:\n1:%s\n2:%s\n", s1, s2);
      GWEN_Buffer_free(buf2);
      GWEN_Buffer_free(buf1);
      return 2;
    }
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);

  }

  /* now dataBuffer, importerName, profileName and dbExpected are set */

  return 0;
}




int main(int argc, char **argv) {
  AB_BANKING *ab;
  int rv;
  int rvTest;
  const char *inFile=NULL;

  if (argc<2) {
    fprintf(stderr, "No file name given\n");
    return 1;
  }
  inFile=argv[1];

  /*fprintf(stderr, "Creating AB_Banking...\n");*/
  ab=AB_Banking_new("imptest", "./aqbanking.conf", 0);

  /*fprintf(stderr, "Initializing AB_Banking...\n");*/
  rv=AB_Banking_Init(ab);
  if (rv) {
    fprintf(stderr, "Could not init AqBanking (%d)\n", rv);
    return 2;
  }

  rvTest=testFile(ab, inFile);

  /*fprintf(stderr, "Deinitializing AB_Banking...\n");*/
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "Could not deinit AqBanking (%d)\n", rv);
    return 2;
  }

  /*fprintf(stderr, "Freeing AB_Banking...\n");*/
  AB_Banking_free(ab);

  if (rvTest!=0)
    fprintf(stderr, "Failed.\n");
  else
    fprintf(stderr, "Passed.\n");
  return 0;
}








