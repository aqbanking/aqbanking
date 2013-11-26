

#include "keys.h"
#include "msg.h"
#include "xml.h"
#include "zip.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/cryptkeyrsa.h>

#include <stdio.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/crypto.h>



int test1(int argc, char **argv) {
  const char *fname;
  GWEN_BUFFER *rbuf;
  EB_MSG *m;
  /*int rv;*/

  if (argc<3) {
    fprintf(stderr, "File name missing.\n");
    return 1;
  }
  fname=argv[2];


  /* Init libxml and libxslt libraries */
  xmlInitParser();
  LIBXML_TEST_VERSION


  /* Init xmlsec library */
  if(xmlSecInit() < 0) {
      fprintf(stderr, "Error: xmlsec initialization failed.\n");
      return(-1);
  }

  /* Check loaded library version */
  if(xmlSecCheckVersion() != 1) {
      fprintf(stderr, "Error: loaded xmlsec library version is not compatible.\n");
      return(-1);
  }

  /* Load default crypto engine if we are supporting dynamic
   * loading for xmlsec-crypto libraries. Use the crypto library
   * name ("openssl", "nss", etc.) to load corresponding 
   * xmlsec-crypto library.
   */
#ifdef XMLSEC_CRYPTO_DYNAMIC_LOADING
  if(xmlSecCryptoDLLoadLibrary(BAD_CAST XMLSEC_CRYPTO) < 0) {
      fprintf(stderr, "Error: unable to load default xmlsec-crypto library. Make sure\n"
                      "that you have it installed and check shared libraries path\n"
                      "(LD_LIBRARY_PATH) envornment variable.\n");
      return(-1);	
  }
#endif /* XMLSEC_CRYPTO_DYNAMIC_LOADING */

  /* Init crypto library */
  if(xmlSecCryptoAppInit(NULL) < 0) {
      fprintf(stderr, "Error: crypto initialization failed.\n");
      return(-1);
  }

  /* Init xmlsec-crypto library */
  if(xmlSecCryptoInit() < 0) {
      fprintf(stderr, "Error: xmlsec-crypto initialization failed.\n");
      return(-1);
  }

  m=EB_Msg_fromFile(fname);
  if (!m) {
    fprintf(stderr, "Bad file.\n");
    return 3;
  }
  rbuf=GWEN_Buffer_new(0, 256, 0, 1);
  EB_Msg_BuildHashSha1(m, rbuf);

  GWEN_Buffer_Dump(rbuf, 2);

  return 0;
}



int test2(int argc, char **argv) {
  const char *fname;
  GWEN_BUFFER *rbuf;
  EB_MSG *m;
  int rv;

  if (argc<3) {
    fprintf(stderr, "File name missing.\n");
    return 1;
  }
  fname=argv[2];


  /* Init libxml and libxslt libraries */
  xmlInitParser();
  LIBXML_TEST_VERSION


  /* Init xmlsec library */
  if(xmlSecInit() < 0) {
      fprintf(stderr, "Error: xmlsec initialization failed.\n");
      return(-1);
  }

  /* Check loaded library version */
  if(xmlSecCheckVersion() != 1) {
      fprintf(stderr, "Error: loaded xmlsec library version is not compatible.\n");
      return(-1);
  }

  /* Load default crypto engine if we are supporting dynamic
   * loading for xmlsec-crypto libraries. Use the crypto library
   * name ("openssl", "nss", etc.) to load corresponding 
   * xmlsec-crypto library.
   */
#ifdef XMLSEC_CRYPTO_DYNAMIC_LOADING
  if(xmlSecCryptoDLLoadLibrary(BAD_CAST XMLSEC_CRYPTO) < 0) {
      fprintf(stderr, "Error: unable to load default xmlsec-crypto library. Make sure\n"
                      "that you have it installed and check shared libraries path\n"
                      "(LD_LIBRARY_PATH) envornment variable.\n");
      return(-1);	
  }
#endif /* XMLSEC_CRYPTO_DYNAMIC_LOADING */

  /* Init crypto library */
  if(xmlSecCryptoAppInit(NULL) < 0) {
      fprintf(stderr, "Error: crypto initialization failed.\n");
      return(-1);
  }

  /* Init xmlsec-crypto library */
  if(xmlSecCryptoInit() < 0) {
      fprintf(stderr, "Error: xmlsec-crypto initialization failed.\n");
      return(-1);
  }

  m=EB_Msg_fromFile(fname);
  if (!m) {
    fprintf(stderr, "Bad file.\n");
    return 3;
  }
  rbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=EB_Msg_ReadHash(m, rbuf);
  if (rv) {
    fprintf(stderr, "No hash...\n");
  }
  GWEN_Buffer_Dump(rbuf, 2);

  return 0;
}



void dumpNode(xmlNodePtr node, int level) {
  xmlNode *cur_node = NULL;

  for (cur_node = node; cur_node; cur_node = cur_node->next) {
    int i;

    printf("%02d:", level);
    for (i=0; i<level*2; i++)
      printf(" ");
    printf("node type: %d, name: %s\n",
	   cur_node->type, cur_node->name);
#if 0
    if (cur_node->type == XML_ELEMENT_NODE) {
      int i;

      for (i=0; i<level*2; i++)
	printf(" ");
      printf("node type: %d, name: %s\n",
	     cur_node->type, cur_node->name);
    }
#endif
    dumpNode(cur_node->children, level+1);
  }
}



int test3(int argc, char **argv) {
  const char *fname;
  EB_MSG *m;
  xmlNodePtr node;

  if (argc<3) {
    fprintf(stderr, "File name missing.\n");
    return 1;
  }
  fname=argv[2];


  /* Init libxml and libxslt libraries */
  xmlInitParser();
  LIBXML_TEST_VERSION


  /* Init xmlsec library */
  if(xmlSecInit() < 0) {
      fprintf(stderr, "Error: xmlsec initialization failed.\n");
      return(-1);
  }

  /* Check loaded library version */
  if(xmlSecCheckVersion() != 1) {
      fprintf(stderr, "Error: loaded xmlsec library version is not compatible.\n");
      return(-1);
  }

  /* Load default crypto engine if we are supporting dynamic
   * loading for xmlsec-crypto libraries. Use the crypto library
   * name ("openssl", "nss", etc.) to load corresponding 
   * xmlsec-crypto library.
   */
#ifdef XMLSEC_CRYPTO_DYNAMIC_LOADING
  if(xmlSecCryptoDLLoadLibrary(BAD_CAST XMLSEC_CRYPTO) < 0) {
      fprintf(stderr, "Error: unable to load default xmlsec-crypto library. Make sure\n"
                      "that you have it installed and check shared libraries path\n"
                      "(LD_LIBRARY_PATH) envornment variable.\n");
      return(-1);	
  }
#endif /* XMLSEC_CRYPTO_DYNAMIC_LOADING */

  /* Init crypto library */
  if(xmlSecCryptoAppInit(NULL) < 0) {
      fprintf(stderr, "Error: crypto initialization failed.\n");
      return(-1);
  }

  /* Init xmlsec-crypto library */
  if(xmlSecCryptoInit() < 0) {
      fprintf(stderr, "Error: xmlsec-crypto initialization failed.\n");
      return(-1);
  }

  m=EB_Msg_fromFile(fname);
  if (!m) {
    fprintf(stderr, "Bad file.\n");
    return 3;
  }
  node=EB_Msg_GetRootNode(m);
  assert(node);
  dumpNode(node, 0);

  return 0;
}


int test5(int argc, char **argv) {
  /*const char *fname;*/
  xmlDocPtr doc;
  xmlNsPtr ns;
  xmlNodePtr root_node = NULL;
  xmlNodePtr nodeX = NULL;
  /*xmlNodePtr nodeXX = NULL;*/

  xmlInitParser();
  LIBXML_TEST_VERSION

  doc=xmlNewDoc(BAD_CAST "1.0");
  root_node=xmlNewNode(NULL, BAD_CAST "ebics");
  xmlDocSetRootElement(doc, root_node);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.ebics.org/H001",
	      NULL);
  assert(ns);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.w3.org/2000/09/xmldsig#",
	      BAD_CAST "ds");
  assert(ns);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.w3.org/2001/XMLSchema-instance",
	      BAD_CAST "xsi");
  xmlNewNsProp(root_node,
	       ns,
	       BAD_CAST "schemaLocation",
	       BAD_CAST "http://www.ebics.org/H001 "
	       "http://www.ebics.org/H001/ebics_request.xsd");
  nodeX=xmlNewChild(root_node, NULL, BAD_CAST "header", NULL);
  ns=xmlSearchNs(doc, nodeX, BAD_CAST "ds");
  if (!ns) {
    fprintf(stderr, "Namespace not found.\n");
    return 2;
  }
  xmlNewChild(nodeX, ns, BAD_CAST "withns", NULL);
  xmlDocDump(stderr, doc);

  xmlFreeDoc(doc);
  return 0;
}



int test6(int argc, char **argv) {
  xmlDocPtr doc;
  xmlNodePtr root_node = NULL;

  xmlInitParser();
  LIBXML_TEST_VERSION

  doc=xmlNewDoc(BAD_CAST "1.0");
  root_node=xmlNewNode(NULL, BAD_CAST "ebics");
  xmlDocSetRootElement(doc, root_node);
  EB_Xml_Ebicsify(root_node, "H002");

  EB_Xml_SetCharValue(root_node,
		      "header/ds:AuthStuff/ds:Signature",
                      "Test");

  xmlDocDump(stderr, doc);

  xmlFreeDoc(doc);
  return 0;
}



int test7(int argc, char **argv) {
  xmlDocPtr doc=NULL;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  xmlNodePtr nodeX = NULL;

  GWEN_Logger_Open(AQEBICS_LOGDOMAIN, "aqebics", 0,
                   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);
  GWEN_Logger_SetLevel(AQEBICS_LOGDOMAIN, GWEN_LoggerLevel_Info);

  doc=xmlNewDoc(BAD_CAST "1.0");
  root_node=xmlNewNode(NULL, BAD_CAST "ebics");
  xmlDocSetRootElement(doc, root_node);

  node=xmlNewChild(root_node, NULL, BAD_CAST "test1", NULL);
  nodeX=xmlNewChild(node, NULL, BAD_CAST "test2", BAD_CAST "valueOf2");
  nodeX=xmlNewChild(node, NULL, BAD_CAST "test3", NULL);
  nodeX=xmlNewChild(node, NULL, BAD_CAST "test4", NULL);
  nodeX=xmlNewNode(NULL, BAD_CAST "before2");
  EB_Xml_InsertChild(node, nodeX);
  dumpNode(root_node, 0);

  return 0;
}



int check1(int argc, char **argv) {
  GWEN_CRYPT_KEY *key1p;
  GWEN_CRYPT_KEY *key1s;
  GWEN_CRYPT_KEY *key2;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  const char *p1, *p2;
  char userIdBuf[16];
  int i;
  int rv;

  fprintf(stderr, "Generating key...\n");
  rv=GWEN_Crypt_KeyRsa_GeneratePair(96, 1, &key1p, &key1s);
  if (rv<0) {
    fprintf(stderr, "Could not create key (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Generating key done\n");

  buf1=GWEN_Buffer_new(0, 512, 0, 1);
  rv=EB_Key_toBin(key1p, "martin", "A004", 768, buf1);
  if (rv) {
    fprintf(stderr, "EB_Key_toBin: %d\n", rv);
    return 3;
  }

  userIdBuf[0]=0;
  rv=EB_Key_fromBin(&key2, "A004",
                    userIdBuf, sizeof(userIdBuf),
		    GWEN_Buffer_GetStart(buf1),
		    GWEN_Buffer_GetUsedBytes(buf1));
  if (rv) {
    fprintf(stderr, "EB_Key_fromBin: %d\n", rv);
    return 3;
  }

  if (strcmp(userIdBuf, "martin")!=0) {
    fprintf(stderr,
	    "Non-matching user id (expected \"martin\", got \"%s\")\n",
	    userIdBuf);
    return 3;
  }

  buf2=GWEN_Buffer_new(0, 512, 0, 1);
  rv=EB_Key_toBin(key2, "martin", "A004", 768, buf2);
  if (rv) {
    fprintf(stderr, "EB_Key_toBin: %d\n", rv);
    return 3;
  }

  if (GWEN_Buffer_GetUsedBytes(buf1)!=GWEN_Buffer_GetUsedBytes(buf2)) {
    fprintf(stderr, "Buffer1:\n");
    GWEN_Buffer_Dump(buf1, 2);

    fprintf(stderr, "Buffer2:\n");
    GWEN_Buffer_Dump(buf2, 2);

    fprintf(stderr, "Key data differs in length\n");
    return 3;
  }

  p1=GWEN_Buffer_GetStart(buf1);
  p2=GWEN_Buffer_GetStart(buf2);
  rv=0;
  for (i=0; i<GWEN_Buffer_GetUsedBytes(buf1); i++) {
    if (p1[i]!=p2[i]) {
      fprintf(stderr, "Buffer1:\n");
      GWEN_Buffer_Dump(buf1, 2);

      fprintf(stderr, "Buffer2:\n");
      GWEN_Buffer_Dump(buf2, 2);

      fprintf(stderr, "Differ at %d (%04x)\n", i, i);
      rv=-1;
    }
  }

  if (rv) {
    fprintf(stderr, "Key data differs in content\n");
    return 3;
  }

  fprintf(stderr, "Check1: PASSED.\n");

  return 0;
}






int check2(int argc, char **argv) {
  xmlDocPtr doc=NULL;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  /*xmlNodePtr nodeX = NULL;*/
  const char *s;
  int rv;

  GWEN_Logger_Open(AQEBICS_LOGDOMAIN, "aqebics", 0,
                   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);
  GWEN_Logger_SetLevel(AQEBICS_LOGDOMAIN, GWEN_LoggerLevel_Info);

  doc=xmlNewDoc(BAD_CAST "1.0");
  root_node=xmlNewNode(NULL, BAD_CAST "ebics");
  xmlDocSetRootElement(doc, root_node);

  node=xmlNewChild(root_node, NULL, BAD_CAST "test1", NULL);
  xmlNewChild(node, NULL, BAD_CAST "test2", BAD_CAST "valueOf2");
  xmlNewChild(node, NULL, BAD_CAST "test3", NULL);
  xmlNewChild(node, NULL, BAD_CAST "test4", NULL);
  node=xmlNewChild(root_node, NULL, BAD_CAST "test5", NULL);

  s=EB_Xml_GetCharValue(root_node, "test1/test2", 0);
  if (!s) {
    fprintf(stderr, "FAILED: Value of \"test2\" not found\n");
    return 3;
  }
  if (strcmp(s, "valueOf2")!=0) {
    fprintf(stderr, "FAILED: Bad value of \"test2\": [%s]\n", s);
    return 3;
  }

  rv=EB_Xml_SetCharValue(root_node, "test1/test3", "valueOf3");
  if (rv) {
    fprintf(stderr, "FAILED: Could not set value of \"test3\"\n");
    return 3;
  }
  s=EB_Xml_GetCharValue(root_node, "test1/test3", 0);
  if (!s) {
    fprintf(stderr, "FAILED: Value of \"test3\" not found\n");
    return 3;
  }
  if (strcmp(s, "valueOf3")!=0) {
    fprintf(stderr, "FAILED: Bad value of \"test3\": [%s]\n", s);
    return 3;
  }

  /* try again value of test2 */
  s=EB_Xml_GetCharValue(root_node, "test1/test2", 0);
  if (!s) {
    fprintf(stderr, "FAILED: Value of \"test2\" not found (2)\n");
    return 3;
  }
  if (strcmp(s, "valueOf2")!=0) {
    fprintf(stderr, "FAILED: Bad value of \"test2\": [%s] (2)\n", s);
    return 3;
  }

  rv=EB_Xml_SetCharValue(root_node, "test4", "valueOf4");
  if (rv) {
    fprintf(stderr, "FAILED: Could not set value of \"test4\"\n");
    return 3;
  }
  s=EB_Xml_GetCharValue(root_node, "test4", 0);
  if (!s) {
    fprintf(stderr, "FAILED: Value of \"test4\" not found\n");
    return 3;
  }
  if (strcmp(s, "valueOf4")!=0) {
    fprintf(stderr, "FAILED: Bad value of \"test4\": [%s]\n", s);
    return 3;
  }

  fprintf(stderr, "Check2: PASSED.\n");
  return 0;
}



int check3(int argc, char **argv) {
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  int rv;
  const char *testString="Hello, this is a zip test string";
  const char *p1, *p2;
  int i;

  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=EB_Zip_Deflate(testString, strlen(testString), buf1);
  if (rv) {
    fprintf(stderr, "FAILED: Could not deflate.\n");
    return 3;
  }

  buf2=GWEN_Buffer_new(0, 256, 0, 1);
  rv=EB_Zip_Inflate(GWEN_Buffer_GetStart(buf1),
                    GWEN_Buffer_GetUsedBytes(buf1), buf2);
  if (rv) {
    fprintf(stderr, "FAILED: Could not inflate.\n");
    return 3;
  }

  if (strlen(testString)!=GWEN_Buffer_GetUsedBytes(buf2)) {
    fprintf(stderr, "Buffer2:\n");
    GWEN_Buffer_Dump(buf2, 2);

    fprintf(stderr, "Data differs in length\n");
    return 3;
  }

  p1=testString;
  p2=GWEN_Buffer_GetStart(buf2);
  rv=0;
  for (i=0; i<GWEN_Buffer_GetUsedBytes(buf2); i++) {
    if (p1[i]!=p2[i]) {
      fprintf(stderr, "Buffer2:\n");
      GWEN_Buffer_Dump(buf2, 2);

      fprintf(stderr, "Differ at %d (%04x)\n", i, i);
      rv=-1;
    }
  }

  if (rv) {
    fprintf(stderr, "Data differs in content\n");
    return 3;
  }

  fprintf(stderr, "Check3: PASSED.\n");
  return 0;
}



int check4(int argc, char **argv) {
  xmlDocPtr doc=NULL;
  xmlDocPtr doc2=NULL;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  int rv;
  xmlChar *xmlbuff;
  int buffersize;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  GWEN_BUFFER *bufTmp;
  const char *p1, *p2;
  int i;

  GWEN_Logger_Open(AQEBICS_LOGDOMAIN, "aqebics", 0,
                   GWEN_LoggerType_Console,
                   GWEN_LoggerFacility_User);
  GWEN_Logger_SetLevel(AQEBICS_LOGDOMAIN, GWEN_LoggerLevel_Info);

  doc=xmlNewDoc(BAD_CAST "1.0");
  root_node=xmlNewNode(NULL, BAD_CAST "ebics");
  xmlDocSetRootElement(doc, root_node);

  node=xmlNewChild(root_node, NULL, BAD_CAST "test1", NULL);
  xmlNewChild(node, NULL, BAD_CAST "test2", BAD_CAST "valueOf2");
  xmlNewChild(node, NULL, BAD_CAST "test3", NULL);
  xmlNewChild(node, NULL, BAD_CAST "test4", NULL);
  node=xmlNewChild(root_node, NULL, BAD_CAST "test5", NULL);

  buf1=GWEN_Buffer_new(0, 1024, 0, 1);
  xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);
  GWEN_Buffer_AppendBytes(buf1, (const char*)xmlbuff, buffersize);
  xmlFree(xmlbuff);

  bufTmp=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=EB_Xml_Compress64Doc(doc, bufTmp);
  if (rv) {
    fprintf(stderr, "Could not compress document.\n");
    return 3;
  }
  rv=EB_Xml_Uncompress64Doc(GWEN_Buffer_GetStart(bufTmp),
                            GWEN_Buffer_GetUsedBytes(bufTmp),
                            &doc2);
  if (rv) {
    fprintf(stderr, "Could not decompress document.\n");
    return 3;
  }

  buf2=GWEN_Buffer_new(0, 1024, 0, 1);
  xmlDocDumpFormatMemory(doc2, &xmlbuff, &buffersize, 1);
  GWEN_Buffer_AppendBytes(buf2, (const char*)xmlbuff, buffersize);
  xmlFree(xmlbuff);

  if (GWEN_Buffer_GetUsedBytes(buf1)!=GWEN_Buffer_GetUsedBytes(buf2)) {
    fprintf(stderr, "Buffer1:\n");
    GWEN_Buffer_Dump(buf1, 2);

    fprintf(stderr, "Buffer2:\n");
    GWEN_Buffer_Dump(buf2, 2);

    fprintf(stderr, "Data differs in length\n");
    return 3;
  }

  p1=GWEN_Buffer_GetStart(buf1);
  p2=GWEN_Buffer_GetStart(buf2);
  rv=0;
  for (i=0; i<GWEN_Buffer_GetUsedBytes(buf1); i++) {
    if (p1[i]!=p2[i]) {
      fprintf(stderr, "Buffer1:\n");
      GWEN_Buffer_Dump(buf1, 2);

      fprintf(stderr, "Buffer2:\n");
      GWEN_Buffer_Dump(buf2, 2);

      fprintf(stderr, "Differ at %d (%04x)\n", i, i);
      rv=-1;
    }
  }

  if (rv) {
    fprintf(stderr, "Data differs in content\n");
    return 3;
  }


  fprintf(stderr, "Check4: PASSED.\n");
  return 0;
}



int main(int argc, char **argv) {
  const char *cmd;
  int rv;

  if (argc<2) {
    fprintf(stderr, "Command missing.\n");
    return 1;
  }

  cmd=argv[1];
  if (strcasecmp(cmd, "test1")==0)
    rv=test1(argc, argv);
  else if (strcasecmp(cmd, "test2")==0)
    rv=test2(argc, argv);
  else if (strcasecmp(cmd, "test3")==0)
    rv=test3(argc, argv);
  else if (strcasecmp(cmd, "test5")==0)
    rv=test5(argc, argv);
  else if (strcasecmp(cmd, "test6")==0)
    rv=test6(argc, argv);
  else if (strcasecmp(cmd, "test7")==0)
    rv=test7(argc, argv);
  else if (strcasecmp(cmd, "check1")==0)
    rv=check1(argc, argv);
  else if (strcasecmp(cmd, "check2")==0)
    rv=check2(argc, argv);
  else if (strcasecmp(cmd, "check3")==0)
    rv=check3(argc, argv);
  else if (strcasecmp(cmd, "check4")==0)
    rv=check4(argc, argv);
  else {
    fprintf(stderr, "Unknown test \"%s\"\n", cmd);
    return 1;
  }

  return rv;
}




