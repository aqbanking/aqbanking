/***************************************************************************
 begin       : Mon May 30 2010
 copyright   : (C) 2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals.h"
#include <aqhbci/user.h>
#include <aqhbci/msgengine.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/syncio_file.h>
#include <gwenhywfar/msgengine.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>




static int _readLogFile(const char *fname, GWEN_DB_NODE *db) {
  GWEN_SYNCIO *sio;
  GWEN_FAST_BUFFER *fb;
  int rv;
  GWEN_BUFFER *tbuf = NULL;

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_ERROR(0, "Error opening file [%s] (%d)", fname, rv);
    return rv;
  }

  /* create fast buffer around io layer */
  fb=GWEN_FastBuffer_new(1024, sio);

  for (;;) {
    GWEN_DB_NODE *dbMsg;
    GWEN_DB_NODE *dbHeader;
    unsigned int size;

    /* read header */
    dbMsg=GWEN_DB_Group_new("Message");
    dbHeader=GWEN_DB_GetGroup(dbMsg, GWEN_DB_FLAGS_DEFAULT, "header");

    rv=GWEN_DB_ReadFromFastBuffer(dbHeader, fb,
				  GWEN_DB_FLAGS_HTTP |
				  GWEN_DB_FLAGS_UNTIL_EMPTY_LINE);
    if (rv<0) {
      if (rv==GWEN_ERROR_EOF)
	break;
      else {
	GWEN_DB_Group_free(dbMsg);
	GWEN_FastBuffer_free(fb);
	GWEN_SyncIo_Disconnect(sio);
	GWEN_SyncIo_free(sio);
	DBG_ERROR(0, "Error reading header from file [%s] (%d)", fname, rv);
	GWEN_DB_Dump(db, 2);
	return rv;
      }
    }

    /* read body */
    size=GWEN_DB_GetIntValue(dbHeader, "size", 0, 0);
    tbuf=GWEN_Buffer_new(0, 2048, 0, 1);
    while(size) {
      unsigned int lsize;
      uint8_t buffer[1024];

      lsize=size;
      if (lsize>sizeof(buffer))
	lsize=sizeof(buffer);

      GWEN_FASTBUFFER_READFORCED(fb, rv, buffer, lsize);
      if (rv<0) {
	GWEN_DB_Group_free(dbMsg);
	GWEN_FastBuffer_free(fb);
	GWEN_SyncIo_Disconnect(sio);
	GWEN_SyncIo_free(sio);
	DBG_ERROR(0, "Error reading body from file [%s] (%d)", fname, rv);
	return rv;
      }
      GWEN_Buffer_AppendBytes(tbuf, (const char*)buffer, lsize);
      size-=lsize;
    } // while

    GWEN_DB_SetBinValue(dbMsg, GWEN_DB_FLAGS_OVERWRITE_VARS, "body",
			GWEN_Buffer_GetStart(tbuf),
			GWEN_Buffer_GetUsedBytes(tbuf));
    GWEN_Buffer_Reset(tbuf);

    GWEN_DB_AddGroup(db, dbMsg);
  }
  GWEN_Buffer_free(tbuf);

  GWEN_FastBuffer_free(fb);
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  return 0;
}



static int dumpMsg(GWEN_SYNCIO *sio,
		   GWEN_DB_NODE *hd,
		   const uint8_t *p,
		   uint32_t len) {
  int rv;

  rv=GWEN_DB_WriteToIo(hd, sio,
		       GWEN_DB_FLAGS_WRITE_SUBGROUPS |
		       GWEN_DB_FLAGS_DETAILED_GROUPS |
		       GWEN_DB_FLAGS_USE_COLON|
		       GWEN_DB_FLAGS_OMIT_TYPES);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  /* append empty line to separate header from data */
  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t*) "\n", 1);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  /* write data */
  if (p && len) {
    rv=GWEN_SyncIo_WriteForced(sio, p, len);
    if (rv<0) {
      DBG_INFO(0, "here (%d)", rv);
      return rv;
    }
  }

  /* append CR for better readability */
  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t*) "\n", 1);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  return 0;
}





int logFile(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv) {
  int rv;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbMessages;
  const char *s;
  GWEN_MSGENGINE *e;
  GWEN_SYNCIO *sioOut=NULL;
  GWEN_SYNCIO *sioDb=NULL;
  const char *inFile;
  const char *outFile;
  const char *dbOutFile;
  int i;
  GWEN_DB_NODE *dbT;
  int trustLevel;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "xmlfile",                    /* name */
    0,                            /* minnum */
    99,                           /* maxnum */
    "x",                          /* short option */
    "xmlfile",                    /* long option */
    "Specify XML files to load",  /* short description */
    "Specify XML files to load"   /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "infile",                     /* name */
    1,                            /* minnum */
    1,                            /* maxnum */
    "i",                          /* short option */
    "infile",                     /* long option */
    "Specify input file",         /* short description */
    "Specify input file"          /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "outfile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "o",                          /* short option */
    "outfile",                    /* long option */
    "Specify output file",        /* short description */
    "Specify output file"         /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "dboutfile",                  /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "d",                          /* short option */
    "dbfile",                     /* long option */
    "Specify DB output file",     /* short description */
    "Specify DB output file"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Int,            /* type */
    "trustLevel",                 /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "L",                          /* short option */
    "trustlevel",                 /* long option */
    "Specify the trust level",    /* short description */
    "Specify the trust level"     /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,            /* type */
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
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  trustLevel=GWEN_DB_GetIntValue(db, "trustLevel", 0, 0);
  outFile=GWEN_DB_GetCharValue(db, "outFile", 0, NULL);
  dbOutFile=GWEN_DB_GetCharValue(db, "dbOutFile", 0, NULL);
  inFile=GWEN_DB_GetCharValue(db, "inFile", 0, NULL);
  assert(inFile);

  /* do it */
  dbMessages=GWEN_DB_Group_new("Messages");
  rv=_readLogFile(inFile, dbMessages);
  if (rv<0) {
    DBG_ERROR(0, "Error reading message (%d)", rv);
    return 2;
  }

  /* create message engine, read XML definitions */
  e=AH_MsgEngine_new();
  for (i=0; i<99; i++) {
    s=GWEN_DB_GetCharValue(dbArgs, "xmlfile", i, NULL);
    if (s && *s) {
      GWEN_XMLNODE *defs;

      defs=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "defs");
      if (GWEN_XML_ReadFile(defs, s, GWEN_XML_FLAGS_DEFAULT)){
	fprintf(stderr, "Error parsing.\n");
	GWEN_MsgEngine_free(e);
	return 2;
      }
      GWEN_MsgEngine_AddDefinitions(e, defs);
      GWEN_XMLNode_free(defs);
    }
    else {
      if (i==0) {
	GWEN_XMLNODE *defs;

	defs=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "defs");
	if (GWEN_XML_ReadFile(defs, XMLDATA_DIR "/hbci.xml", GWEN_XML_FLAGS_DEFAULT)){
	  fprintf(stderr, "Error parsing.\n");
	  GWEN_MsgEngine_free(e);
	  return 2;
	}
	GWEN_MsgEngine_AddDefinitions(e, defs);
	GWEN_XMLNode_free(defs);
      }
      break;
    }
  }

  if (outFile) {
    sioOut=GWEN_SyncIo_File_new(outFile, GWEN_SyncIo_File_CreationMode_CreateAlways);
    GWEN_SyncIo_AddFlags(sioOut,
			 GWEN_SYNCIO_FILE_FLAGS_READ |
			 GWEN_SYNCIO_FILE_FLAGS_WRITE |
			 GWEN_SYNCIO_FILE_FLAGS_UREAD |
			 GWEN_SYNCIO_FILE_FLAGS_UWRITE |
			 GWEN_SYNCIO_FILE_FLAGS_APPEND);
    rv=GWEN_SyncIo_Connect(sioOut);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_free(sioOut);
      return 2;
    }
  }

  if (dbOutFile) {
    sioDb=GWEN_SyncIo_File_new(dbOutFile, GWEN_SyncIo_File_CreationMode_CreateAlways);
    GWEN_SyncIo_AddFlags(sioDb,
			 GWEN_SYNCIO_FILE_FLAGS_READ |
			 GWEN_SYNCIO_FILE_FLAGS_WRITE |
			 GWEN_SYNCIO_FILE_FLAGS_UREAD |
			 GWEN_SYNCIO_FILE_FLAGS_UWRITE |
			 GWEN_SYNCIO_FILE_FLAGS_APPEND);
    rv=GWEN_SyncIo_Connect(sioDb);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_free(sioDb);
      return 2;
    }
  }

  dbT=GWEN_DB_GetFirstGroup(dbMessages);
  while(dbT) {
    const uint8_t *p;
    uint32_t len;
    GWEN_DB_NODE *dbHeader;

    dbHeader=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "header");
    assert(dbHeader);

    s=GWEN_DB_GetCharValue(dbHeader, "mode", 0, "PINTAN");
    GWEN_MsgEngine_SetMode(e, s);

    i=GWEN_DB_GetIntValue(dbHeader, "hbciVersion", 0, 220);
    GWEN_MsgEngine_SetProtocolVersion(e, i);

    p=GWEN_DB_GetBinValue(dbT, "body", 0, NULL, 0, &len);
    if (p && len) {
      GWEN_BUFFER *tbuf;
      GWEN_DB_NODE *gr;
      GWEN_MSGENGINE_TRUSTEDDATA *trustedData;
      GWEN_MSGENGINE_TRUSTEDDATA *ntd;
      GWEN_DB_NODE *repl;

      gr=GWEN_DB_Group_new("message");
      tbuf=GWEN_Buffer_new((char*) p, len, len, 0);
      rv=GWEN_MsgEngine_ReadMessage(e, "SEG", tbuf, gr,
				    GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO);
      if (rv) {
	fprintf(stderr, "ERROR.\n");
	GWEN_Buffer_Dump(tbuf, 2);
	return 2;
      }

      /* work on trust data */
      trustedData=GWEN_MsgEngine_TakeTrustInfo(e);
      if (trustedData) {
	if (GWEN_MsgEngine_TrustedData_CreateReplacements(trustedData)) {
	  fprintf(stderr, "Could not anonymize log (createReplacements)\n");
	  GWEN_MsgEngine_TrustedData_free(trustedData);
	  GWEN_MsgEngine_free(e);
	  return 2;
	}
      }

      /* anonymize file */
      ntd=trustedData;
      repl=GWEN_DB_GetGroup(dbHeader, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "replacements");
      assert(repl);
      while(ntd) {
	if (GWEN_MsgEngine_TrustedData_GetTrustLevel(ntd)>trustLevel) {
	  int pos;
	  unsigned int size;
	  char rbuffer[3];
	  const char *rpstr;

	  rpstr=GWEN_MsgEngine_TrustedData_GetReplacement(ntd);
	  assert(rpstr);
	  assert(*rpstr);
	  size=strlen(rpstr);
	  if (size==1) {
	    rbuffer[0]=rpstr[0];
	    rbuffer[1]=0;
	  }
	  else {
	    rbuffer[0]=rpstr[0];
	    rbuffer[1]=rpstr[1];
	    rbuffer[2]=0;
	  }
	  GWEN_DB_SetCharValue(repl,
			       GWEN_DB_FLAGS_DEFAULT |
			       GWEN_PATH_FLAGS_CREATE_VAR,
			       rbuffer,
			       GWEN_MsgEngine_TrustedData_GetDescription(ntd));
	  size=GWEN_MsgEngine_TrustedData_GetSize(ntd);
	  pos=GWEN_MsgEngine_TrustedData_GetFirstPos(ntd);
	  while(pos>=0) {
	    DBG_INFO(0, "Replacing %d bytes at %d", size, pos);
	    GWEN_Buffer_SetPos(tbuf, pos);
	    GWEN_Buffer_ReplaceBytes(tbuf,
				     size,
				     GWEN_MsgEngine_TrustedData_GetReplacement(ntd),
				     size);
	    pos=GWEN_MsgEngine_TrustedData_GetNextPos(ntd);
	  } // while pos
	}
	ntd=GWEN_MsgEngine_TrustedData_GetNext(ntd);
      } // while ntd

      GWEN_DB_SetIntValue(dbHeader, GWEN_DB_FLAGS_OVERWRITE_VARS, "size", GWEN_Buffer_GetUsedBytes(tbuf));
      if (outFile) {
	rv=dumpMsg(sioOut,
		   dbHeader,
		   (const uint8_t*)GWEN_Buffer_GetStart(tbuf),
		   GWEN_Buffer_GetUsedBytes(tbuf));
	if (rv<0) {
	  fprintf(stderr, "Could not anonymize log (dumpMsg)\n");
	  GWEN_MsgEngine_TrustedData_free(trustedData);
	  GWEN_MsgEngine_free(e);
	  return 2;
	}
      }

      if (dbOutFile) {
	GWEN_BUFFER *xbuf;
	GWEN_DB_NODE *dbOut;

	xbuf=GWEN_Buffer_new(0, 256, 0, 1);
	GWEN_Buffer_AppendString(xbuf, "# ========== Message ( ");
	s=GWEN_DB_GetCharValue(dbHeader, "sender", 0, "UNK");
	if (s && *s) {
	  GWEN_Buffer_AppendString(xbuf, "sender=");
	  GWEN_Buffer_AppendString(xbuf, s);
	  GWEN_Buffer_AppendString(xbuf, " ");
	}
	s=GWEN_DB_GetCharValue(dbHeader, "crypt", 0, "UNK");
	if (s && *s) {
	  GWEN_Buffer_AppendString(xbuf, "crypt=");
	  GWEN_Buffer_AppendString(xbuf, s);
	  GWEN_Buffer_AppendString(xbuf, " ");
	}
	GWEN_Buffer_AppendString(xbuf, ") ==========\n");

	dbOut=GWEN_DB_Group_new("Message");
	GWEN_Buffer_Rewind(tbuf);
	rv=GWEN_MsgEngine_ReadMessage(e, "SEG", tbuf, dbOut, 0);
	if (rv) {
	  fprintf(stderr, "ERROR.\n");
	  GWEN_Buffer_Dump(tbuf, 2);
	  return 2;
	}
  
	rv=GWEN_SyncIo_WriteForced(sioDb,
				   (const uint8_t*) GWEN_Buffer_GetStart(xbuf),
				   GWEN_Buffer_GetUsedBytes(xbuf));
	GWEN_Buffer_free(xbuf);
	if (rv<0) {
	  DBG_INFO(0, "here (%d)", rv);
	  return rv;
	}
	rv=GWEN_DB_WriteToIo(dbOut, sioDb, GWEN_DB_FLAGS_DEFAULT);
	if (rv<0) {
	  DBG_INFO(0, "here (%d)", rv);
	  return 2;
	}
  
	/* append empty line to separate header from data */
	rv=GWEN_SyncIo_WriteForced(sioDb, (const uint8_t*) "\n", 1);
	if (rv<0) {
	  DBG_INFO(0, "here (%d)", rv);
	  return rv;
	}
      }

      GWEN_Buffer_free(tbuf);
    }

    dbT=GWEN_DB_GetNextGroup(dbT);
  }

  /* close output layer */
  if (outFile) {
    rv=GWEN_SyncIo_Disconnect(sioOut);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_free(sioOut);
      return 2;
    }
    GWEN_SyncIo_free(sioOut);
  }

  if (dbOutFile) {
    rv=GWEN_SyncIo_Disconnect(sioDb);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_free(sioDb);
      return 2;
    }
    GWEN_SyncIo_free(sioDb);
  }

  return 0;
}


