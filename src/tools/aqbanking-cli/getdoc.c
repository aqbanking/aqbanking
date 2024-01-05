/***************************************************************************
 begin       : Fri Mar 20 2021
 copyright   : (C) 2021 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"
#include <gwenhywfar/text.h>

#include <aqbanking/types/balance.h>
#include <aqbanking/types/document.h>


#define GETDOC_FLAGS_MULTI  0x0001


static AB_DOCUMENT_LIST2 *_getMatchingDocuments(AB_IMEXPORTER_CONTEXT *ctx, const char *wantedDocId);
static int _ensureDocData(AB_DOCUMENT *doc);
static int _exportDocuments(const AB_DOCUMENT_LIST2 *docList, const char *dest, uint32_t flags);
static int _writeDoc(const AB_DOCUMENT *doc, const char *dest, uint32_t flags);
static GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv);




int getDoc(AB_BANKING *ab, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const char *ctxFile;
  uint32_t flags=0;
  AB_IMEXPORTER_CONTEXT *ctx=NULL;
  AB_DOCUMENT_LIST2 *docList2;
  const char *docId;
  const char *dest;

  /* parse command line arguments */
  db=_readCommandLine(dbArgs, argc, argv);
  if (db==NULL) {
    /* error in command line */
    return 1;
  }

  /* read command line arguments */
  docId=GWEN_DB_GetCharValue(db, "docId", 0, 0);
  dest=GWEN_DB_GetCharValue(db, "destination", 0, 0);
  flags|=(GWEN_DB_GetIntValue(db, "multi", 0, 0)>0)?GETDOC_FLAGS_MULTI:0;

  /* init AqBanking */
  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  /* load ctx file */
  ctxFile=GWEN_DB_GetCharValue(db, "ctxfile", 0, 0);
  rv=readContext(ctxFile, &ctx, 1);
  if (rv<0) {
    DBG_ERROR(0, "Error reading context (%d)", rv);
    AB_ImExporterContext_free(ctx);
    return 3;
  }

  docList2=_getMatchingDocuments(ctx, docId);
  if (docList2==NULL) {
    DBG_ERROR(NULL, "No documents to export");
    AB_Document_List2_free(docList2);
    AB_ImExporterContext_free(ctx);
    return 4;
  }
  if (AB_Document_List2_GetSize(docList2)>1 && !(flags & GETDOC_FLAGS_MULTI)) {
    DBG_ERROR(NULL, "Ambigous arguments: Multiple matching documents but no -m flag given");
    AB_Document_List2_free(docList2);
    return 4;
  }

  rv=_exportDocuments(docList2, dest, flags);
  if (rv<0) {
    DBG_INFO(NULL, "Error exporting documents");
    AB_Document_List2_free(docList2);
    AB_ImExporterContext_free(ctx);
    return 4;
  }


  AB_Document_List2_free(docList2);
  AB_ImExporterContext_free(ctx);

  /* deinit */
  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}



AB_DOCUMENT_LIST2 *_getMatchingDocuments(AB_IMEXPORTER_CONTEXT *ctx, const char *wantedDocId)
{
  AB_DOCUMENT_LIST2 *docList2;
  AB_IMEXPORTER_ACCOUNTINFO *iea;

  docList2=AB_Document_List2_new();

  /* copy context, but only keep wanted accounts and transactions */
  iea=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while (iea) {
    AB_DOCUMENT_LIST *docList;

    docList=AB_ImExporterAccountInfo_GetEStatementList(iea);
    if (docList) {
      AB_DOCUMENT *doc;

      doc=AB_Document_List_First(docList);
      while (doc) {
        if (wantedDocId) {
          const char *docId;

          docId=AB_Document_GetId(doc);
          if (docId && *docId) {
            if (GWEN_Text_ComparePattern(docId, wantedDocId, 0)!=-1)
              AB_Document_List2_PushBack(docList2, doc);
          }
        }
        else
          AB_Document_List2_PushBack(docList2, doc);
        doc=AB_Document_List_Next(doc);
      }
    }

    iea=AB_ImExporterAccountInfo_List_Next(iea);
  } /* while */

  if (AB_Document_List2_GetSize(docList2)==0) {
    AB_Document_List2_free(docList2);
    return NULL;
  }

  return docList2;
}



int _exportDocuments(const AB_DOCUMENT_LIST2 *docList, const char *dest, uint32_t flags)
{
  AB_DOCUMENT_LIST2_ITERATOR *it;

  it=AB_Document_List2_First(docList);
  if (it) {
    AB_DOCUMENT *doc;

    doc=AB_Document_List2Iterator_Data(it);
    while (doc) {
      const char *docId;
      int rv;

      docId=AB_Document_GetId(doc);

      rv=_ensureDocData(doc);
      if (rv<0) {
        DBG_INFO(NULL, "Error ensuring data for document \"%s\"", docId?docId:"<no id>");
        return rv;
      }

      rv=_writeDoc(doc, dest, flags);
      if (rv<0) {
        DBG_INFO(NULL, "here (%d)", rv);
        return rv;
      }
      doc=AB_Document_List2Iterator_Next(it);
    } /* while */

    AB_Document_List2Iterator_free(it);
  }

  return 0;
}



int _writeDoc(const AB_DOCUMENT *doc, const char *dest, uint32_t flags)
{
  const char *docId;
  int rv;

  docId=AB_Document_GetId(doc);

  if (flags & GETDOC_FLAGS_MULTI) {
    GWEN_BUFFER *pathBuffer;

    /* dest is a folder */
    if (!(docId && *docId)) {
      DBG_ERROR(NULL, "No id in document, SNH!");
      return GWEN_ERROR_BAD_DATA;
    }

    pathBuffer=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(pathBuffer, dest);
    GWEN_Buffer_AppendString(pathBuffer, GWEN_DIR_SEPARATOR_S);
    GWEN_Buffer_AppendString(pathBuffer, docId);
    GWEN_Buffer_AppendString(pathBuffer, ".pdf");

    rv=GWEN_SyncIo_Helper_WriteFile(GWEN_Buffer_GetStart(pathBuffer),
                                    AB_Document_GetDataPtr(doc),
                                    AB_Document_GetDataLen(doc));
    if (rv<0) {
      DBG_INFO(NULL, "Error writing data for document \"%s\" to \"%s\"",
               docId?docId:"<no id>",
               GWEN_Buffer_GetStart(pathBuffer));
      GWEN_Buffer_free(pathBuffer);
      return rv;
    }
    return 0;
  }
  else {
    rv=GWEN_SyncIo_Helper_WriteFile(dest,
                                    AB_Document_GetDataPtr(doc),
                                    AB_Document_GetDataLen(doc));
    if (rv<0) {
      DBG_INFO(NULL, "Error writing data for document \"%s\" to \"%s\"",
               docId?docId:"<no id>",
               dest);
      return rv;
    }
    return 0;
  }
}



int _ensureDocData(AB_DOCUMENT *doc)
{
  const uint8_t *ptrData;
  uint32_t lenData;

  ptrData=AB_Document_GetDataPtr(doc);
  lenData=AB_Document_GetDataLen(doc);
  if (!(ptrData && lenData)) {
    const char *filePath;

    filePath=AB_Document_GetFilePath(doc);
    if (filePath && *filePath) {
      GWEN_BUFFER *dbuf;
      int rv;

      dbuf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=GWEN_SyncIo_Helper_ReadFile(filePath, dbuf);
      if (rv<0) {
        DBG_ERROR(NULL, "Could not read source file \"%s\" (%d)", filePath, rv);
        GWEN_Buffer_free(dbuf);
        return rv;
      }
      AB_Document_SetData(doc,
                          (const uint8_t *) GWEN_Buffer_GetStart(dbuf),
                          GWEN_Buffer_GetUsedBytes(dbuf));
      GWEN_Buffer_free(dbuf);
      return 0;
    }
  }

  return 0;
}



/* parse command line */
GWEN_DB_NODE *_readCommandLine(GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DB_NODE *db;
  int rv;
  const GWEN_ARGS args[]= {
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "docId",                       /* name */
      0,                             /* minnum */
      1,                             /* maxnum */
      "d",                           /* short option */
      "docid",                       /* long option */
      "Specify the document id",     /* short description */
      "Specify the document id"      /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT,  /* flags */
      GWEN_ArgsType_Char,            /* type */
      "destination",                 /* name */
      1,                             /* minnum */
      1,                             /* maxnum */
      "o",                           /* short option */
      "destination",                 /* long option */
      "Specify destination",         /* short description */
      "Specify destination (folder if -m flag given, specific filename otherwise)"      /* long description */
    },
    {
      0, /* flags */
      GWEN_ArgsType_Int,            /* type */
      "multi",                      /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "m",                          /* short option */
      "multi",                      /* long option */
      "Export multiple documents",  /* short description */
      "Export multiple documents"   /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
      GWEN_ArgsType_Char,            /* type */
      "ctxFile",                    /* name */
      0,                            /* minnum */
      1,                            /* maxnum */
      "c",                          /* short option */
      "ctxfile",                    /* long option */
      "Specify the file to store the context in",   /* short description */
      "Specify the file to store the context in"      /* long description */
    },
    {
      GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
      GWEN_ArgsType_Int,             /* type */
      "help",                       /* name */
      0,                            /* minnum */
      0,                            /* maxnum */
      "h",                          /* short option */
      "help",                       /* long option */
      "Show this help screen",      /* short description */
      "Show this help screen"       /* long description */
    }
  };


  db=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "local");
  rv=GWEN_Args_Check(argc, argv, 1,
                     0 /*GWEN_ARGS_MODE_ALLOW_FREEPARAM*/,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return NULL;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return NULL;
    }
    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, "\n");
    GWEN_Buffer_AppendString(ubuf, "There are two ways to use this tool:\n");
    GWEN_Buffer_AppendString(ubuf, "1) Export a specific document\n");
    GWEN_Buffer_AppendString(ubuf, "   aqbanking-cli getdoc -d DOCID -o OUTFILE.PDF\n");
    GWEN_Buffer_AppendString(ubuf, "2) Export multiple documents:\n");
    GWEN_Buffer_AppendString(ubuf, "   aqbanking-cli getdoc -m -o OUTFOLDER\n");
    GWEN_Buffer_AppendString(ubuf, "In this case for every document in the context file a new\n");
    GWEN_Buffer_AppendString(ubuf, "is created in the OUTFOLDER folder, each file has the name of the document\n");
    GWEN_Buffer_AppendString(ubuf, "with '.pdf' appended.\n");
    fprintf(stdout, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return NULL;
  }

  return db;
}





