/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Fri Apr 02 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "swift_p.h"
#include "swift940_l.h"
#include <aqbanking/banking.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


GWEN_LIST_FUNCTIONS(AHB_SWIFT_TAG, AHB_SWIFT_Tag);



int AHB_SWIFT_Condense(char *buffer) {
  char *src;
  char *dst;
  int lastWasBlank;

  src=buffer;
  while(*src && isspace(*src)) src++;
  dst=buffer;
  lastWasBlank=0;
  while(*src) {
    if (isspace(*src) && (*src!=10)) {
      if (!lastWasBlank) {
        *dst=' ';
        dst++;
        lastWasBlank=1;
      }
    }
    else {
      lastWasBlank=0;
      if (*src!=10) {
	*dst=*src;
	dst++;
      }
    }
    src++;
  } /* while */
  *dst=0;
  return 0;
}




AHB_SWIFT_TAG *AHB_SWIFT_Tag_new(const char *id,
                                 const char *content){
  AHB_SWIFT_TAG *tg;

  assert(id);
  assert(content);
  GWEN_NEW_OBJECT(AHB_SWIFT_TAG, tg);
  GWEN_LIST_INIT(AHB_SWIFT_TAG, tg);
  tg->id=strdup(id);
  tg->content=strdup(content);

  return tg;
}



void AHB_SWIFT_Tag_free(AHB_SWIFT_TAG *tg){
  if (tg) {
    GWEN_LIST_FINI(AHB_SWIFT_TAG, tg);
    free(tg->id);
    free(tg->content);
    GWEN_FREE_OBJECT(tg);
  }
}



const char *AHB_SWIFT_Tag_GetId(const AHB_SWIFT_TAG *tg){
  assert(tg);
  return tg->id;
}



const char *AHB_SWIFT_Tag_GetData(const AHB_SWIFT_TAG *tg){
  assert(tg);
  return tg->content;
}





int AHB_SWIFT_ReadLine(GWEN_BUFFEREDIO *bio,
                       char *buffer,
                       unsigned int s){
  int lastWasAt;

  assert(bio);
  assert(buffer);
  assert(s);

  *buffer=0;
  lastWasAt=0;

  for(;;) {
    int c;

    if (GWEN_BufferedIO_CheckEOF(bio))
      break;
    c=GWEN_BufferedIO_ReadChar(bio);
    if (c<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading from stream");
      *buffer=0;
      return -1;
    }
    if (c==10)
      break;
    else if (c=='@') {
      if (lastWasAt)
        break;
      else
        lastWasAt=1;
    }
    else {
      lastWasAt=0;
      if (c!=13) {
        if (s<2) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Buffer full (line too long)");
          *buffer=0;
          return -1;
        }
        *buffer=c;
        buffer++;
        s--;
      }
    }
  } /* for */
  *buffer=0;
  return 0;
}




int AHB_SWIFT_ReadDocument(GWEN_BUFFEREDIO *bio,
                           AHB_SWIFT_TAG_LIST *tl,
                           unsigned int maxTags) {
  GWEN_ERRORCODE err;
  GWEN_BUFFER *lbuf;
  char buffer[AHB_SWIFT_MAXLINELEN];
  char *p;
  char *p2;
  AHB_SWIFT_TAG *tag;
  int tagCount;

  lbuf=GWEN_Buffer_new(0, AHB_SWIFT_MAXLINELEN, 0, 1);
  tagCount=0;

  /* read first line, should be empty */
  for (;;) {
    if (GWEN_BufferedIO_CheckEOF(bio)) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Empty stream");
      GWEN_Buffer_free(lbuf);
      return 1;
    }
    err=AHB_SWIFT_ReadLine(bio, buffer, sizeof(buffer)-1);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading from stream");
      GWEN_Buffer_free(lbuf);
      return -1;
    }
    if (buffer[0])
      /* line is not empty, let's dance */
      break;
  } /* for */

  if (buffer[0]=='-') {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Empty report");
    GWEN_Buffer_free(lbuf);
    return 1;
  }

  for (;;) {
    /* get a tag */
    GWEN_Buffer_Reset(lbuf);

    if (buffer[0]) {
      if (buffer[0]=='-' && buffer[1]==0) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "End of SWIFT document reached");
        GWEN_Buffer_free(lbuf);
        return 0;
      }
      GWEN_Buffer_AppendString(lbuf, buffer);
    }

    /* get a complete tag, don't be fooled by CR/LF inside a tag.
     * well, normally a CR/LF sequence ends a tag. However, in :86: tags
     * we may have fields which might include CR/LF sequences...
     */
    for (;;) {
      buffer[0]=0;
      if (GWEN_BufferedIO_CheckEOF(bio)) {
        /* eof met */
        if (GWEN_Buffer_GetUsedBytes(lbuf)==0) {
          /* eof met and buffer empty, finished */
          DBG_INFO(AQBANKING_LOGDOMAIN,
                   "SWIFT document not terminated by \'-\'");
	  GWEN_Buffer_free(lbuf);
	  return 0;
	}
	else {
	  buffer[0]='-';
          buffer[1]=0;
	  break;
	}
      }
      else {
	/* read next line */
	err=AHB_SWIFT_ReadLine(bio, buffer, sizeof(buffer)-1);
	if (!GWEN_Error_IsOk(err)) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN,
		    "Error reading from stream");
	  GWEN_Buffer_free(lbuf);
	  return -1;
        }
      }

      /* check whether the line starts with a ":" or "-" */
      if (buffer[0]==':' || (buffer[0]=='-' && buffer[1]==0)) {
        /* it does, so the buffer contains the next line, go handle the
         * previous line */
        break;
      }

      /* it doesn't, so there is a CR/LF inside the tag */
      GWEN_Buffer_AppendByte(lbuf, 10);
      GWEN_Buffer_AppendString(lbuf, buffer);
    } /* for */

    /* tag complete, parse it */
    p=GWEN_Buffer_GetStart(lbuf);

    if (*p!=':') {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Error in SWIFT data: no tag name (data follows)");
      GWEN_Text_DumpString(GWEN_Buffer_GetStart(lbuf),
                           GWEN_Buffer_GetUsedBytes(lbuf), stderr, 2);
      GWEN_Buffer_free(lbuf);
      return -1;
    }
    p++;
    p2=p;
    while(*p2 && *p2!=':') p2++;
    if (*p2!=':') {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Error in SWIFT data: incomplete tag name (data follows)");
      GWEN_Text_DumpString(GWEN_Buffer_GetStart(lbuf),
                           GWEN_Buffer_GetUsedBytes(lbuf), stderr, 2);
      GWEN_Buffer_free(lbuf);
      return -1;
    }
    *p2=0;
    p2++;

    /* create tag */
    DBG_DEBUG(AQBANKING_LOGDOMAIN,
              "Creating tag \"%s\" (%s)", p, p2);
    tag=AHB_SWIFT_Tag_new(p, p2);
    AHB_SWIFT_Tag_List_Add(tag, tl);
    tagCount++;
    if (maxTags && tagCount>=maxTags) {
      DBG_INFO(AQBANKING_LOGDOMAIN,
               "Read maximum number of tags (%d)", tagCount);
      GWEN_Buffer_free(lbuf);
      return 0;
    }

  } /* for */

  /* we should never reach this point... */
  return 0;
}



int AHB_SWIFT_Import(GWEN_DBIO *dbio,
		     GWEN_BUFFEREDIO *bio,
		     GWEN_TYPE_UINT32 flags,
                     GWEN_DB_NODE *data,
                     GWEN_DB_NODE *cfg){
  AHB_SWIFT_TAG_LIST *tl;
  int rv;
  const char *p;

  p=GWEN_DB_GetCharValue(cfg, "type", 0, "mt940");
  if (strcasecmp(p, "mt940")!=0 &&
      strcasecmp(p, "mt942")!=0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Type \"%s\" not supported by plugin \"%s\"",
              p,
              GWEN_DBIO_GetName(dbio));
    return -1;
  }
  tl=AHB_SWIFT_Tag_List_new();

  /* fill tag list */
  GWEN_WaitCallback_Log(2, "SWIFT: Reading complete stream");
  for(;;) {
    rv=AHB_SWIFT_ReadDocument(bio, tl, 0);
    if (rv==-1) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Error in report, aborting");
      AHB_SWIFT_Tag_List_free(tl);
      return -1;
    }

    if (rv==1) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "End of document reached");
      break;
    }
  } /* for */

  /* now all tags have been read, transform them */
  GWEN_WaitCallback_Log(2, "SWIFT: Parsing data");
  if (AHB_SWIFT940_Import(bio, tl, flags, data, cfg)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Error importing SWIFT MT940");
    AHB_SWIFT_Tag_List_free(tl);
    return -1;
  }
  AHB_SWIFT_Tag_List_free(tl);
  DBG_INFO(AQBANKING_LOGDOMAIN, "SWIFT MT940 successfully imported");
  return 0;
}



int AHB_SWIFT_Export(GWEN_DBIO *dbio,
		     GWEN_BUFFEREDIO *bio,
		     GWEN_TYPE_UINT32 flags,
		     GWEN_DB_NODE *data,
                     GWEN_DB_NODE *cfg){
  DBG_ERROR(AQBANKING_LOGDOMAIN, "AHB_SWIFT_Export: Not yet implemented");
  return -1;
}



GWEN_DBIO_CHECKFILE_RESULT AHB_SWIFT_CheckFile(GWEN_DBIO *dbio,
                                               const char *fname) {
  int fd;
  GWEN_BUFFEREDIO *bio;
  AHB_SWIFT_TAG_LIST *tl;
  int rv;
  int cnt;

  assert(dbio);
  assert(fname);

  fd=open(fname, O_RDONLY);
  if (fd==-1) {
    /* error */
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "open(%s): %s", fname, strerror(errno));
    return GWEN_DBIO_CheckFileResultNotOk;
  }

  tl=AHB_SWIFT_Tag_List_new();
  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 256);
  rv=AHB_SWIFT_ReadDocument(bio, tl, 1);
  cnt=AHB_SWIFT_Tag_List_GetCount(tl);
  AHB_SWIFT_Tag_List_free(tl);
  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  if (rv==0) {
    if (cnt==0) {
      /* unknown but rather unlikely that this file is supported */
      DBG_INFO(AQBANKING_LOGDOMAIN,
               "Unknown whether file \"%s\" is supported by this plugin",
               fname);
      return GWEN_DBIO_CheckFileResultUnknown;
    }
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "File \"%s\" is supported by this plugin",
             fname);
    return GWEN_DBIO_CheckFileResultOk;
  }
  /* definately not supported if an error occurred */
  DBG_INFO(AQBANKING_LOGDOMAIN,
           "File \"%s\" is not supported by this plugin",
           fname);
  return GWEN_DBIO_CheckFileResultNotOk;
}



GWEN_DBIO *swift_factory() {
  GWEN_DBIO *dbio;

  dbio=GWEN_DBIO_new("swift", "Imports and exports SWIFT data");
  GWEN_DBIO_SetImportFn(dbio, AHB_SWIFT_Import);
  GWEN_DBIO_SetExportFn(dbio, AHB_SWIFT_Export);
  GWEN_DBIO_SetCheckFileFn(dbio, AHB_SWIFT_CheckFile);
  return dbio;
}







