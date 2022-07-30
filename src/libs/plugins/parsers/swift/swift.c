/***************************************************************************
 begin       : Fri Apr 02 2004
 copyright   : (C) 2004-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "swift_p.h"
#include "swift940_l.h"
#include "swift535_l.h"
#include "aqbanking/i18n_l.h"
#include <aqbanking/banking.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/fastbuffer.h>
#include <gwenhywfar/dbio_be.h>
#include <gwenhywfar/syncio_file.h>
#include <gwenhywfar/syncio_buffered.h>

#include <ctype.h>
#include <fcntl.h>
#include <errno.h>


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _pluginExport(GWEN_DBIO *dbio, GWEN_SYNCIO *sio, GWEN_DB_NODE *data, GWEN_DB_NODE *cfg, uint32_t flags);
static int _pluginImport(GWEN_DBIO *dbio, GWEN_SYNCIO *sio, GWEN_DB_NODE *data, GWEN_DB_NODE *cfg, uint32_t flags);
static GWEN_DBIO_CHECKFILE_RESULT _pluginCheckFile(GWEN_DBIO *dbio, const char *fname);
static GWEN_DBIO *_pluginFactory(GWEN_PLUGIN *pl);

static void _iso8859_1ToUtf8(const char *p, int size, GWEN_BUFFER *buf);
static GWEN_DATE *_dateFromYMD(int dateYear, int dateMonth, int dateDay);

static int _readDocument(GWEN_FAST_BUFFER *fb, AHB_SWIFT_TAG_LIST *tl, unsigned int maxTags);
static int _readTextBlock(GWEN_FAST_BUFFER *fb, AHB_SWIFT_TAG_LIST *tl, unsigned int maxTags);
/**
 * This function reads a line from a buffered IO stream. It stops when either
 * the line or the stream ends. The end of line is signalled by an LF
 * character or a series of two "@" characters (for historical reasons).
 */
static int _readDocLine(GWEN_FAST_BUFFER *fb, char *buffer, unsigned int s);


GWENHYWFAR_EXPORT GWEN_PLUGIN *dbio_swift_factory(GWEN_PLUGIN_MANAGER *pm, const char *modName, const char *fileName);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

int AHB_SWIFT_Condense(char *buffer, int keepMultipleBlanks)
{
  char *src;
  char *dst;

  if (keepMultipleBlanks) {
    src=buffer;
    dst=buffer;

    /* only remove line feed */
    while (*src) {
      if (*src!=10) {
        *dst=*src;
        dst++;
      }
      src++;
    } /* while */
  }
  else {
    int lastWasBlank;

    src=buffer;
    while (*src && isspace(*src))
      src++;
    dst=buffer;
    lastWasBlank=0;
    while (*src) {
      if (isspace(*src) && (*src!=10)) {
        if (!lastWasBlank) {
          *(dst++)=' ';
          lastWasBlank=1;
        }
      }
      else {
        lastWasBlank=0;
        if (*src!=10) {
          *(dst++)=*src;
        }
      }
      src++;
    } /* while */
  }
  *dst=0;
  return 0;
}



/* This reads a line within a SWIFT data block (block 4)
   @param *fb     pointer to a GWEN_FAST_BUFFER input buffer
   @param *buffer pointer to a char* output buffer
   @param s       size of the output buffer
   */
int _readDocLine(GWEN_FAST_BUFFER *fb, char *buffer, unsigned int s)
{
  int lastWasAt;
  char *obuffer;

  assert(fb);
  assert(buffer);
  assert(s);

  obuffer=buffer;

  *buffer=0;
  lastWasAt=0;

  for (;;) {
    int c;

    GWEN_FASTBUFFER_PEEKBYTE(fb, c);
    if (c<0) {
      if (c==GWEN_ERROR_EOF) {
        if (*obuffer==0) {
          return GWEN_ERROR_EOF;
        }
        break;
      }
      else  {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading from stream");
        *buffer=0;
        return c;
      }
    }
    if (c=='}') {
      /* stop on curly bracket without reading it */
      break;
    }
    GWEN_FASTBUFFER_READBYTE(fb, c);

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

  /*GWEN_Text_DumpString(obuffer, buffer-obuffer+1, stderr, 2);*/

  return 0;
}



/* This will read the contents of a SWIFT data block ({4: ... })
   inside of a SWIFT document
 */
int _readTextBlock(GWEN_FAST_BUFFER *fb, AHB_SWIFT_TAG_LIST *tl, unsigned int maxTags)
{
  GWEN_BUFFER *lbuf;
  char buffer[AHB_SWIFT_MAXLINELEN];
  char *p;
  char *p2;
  AHB_SWIFT_TAG *tag;
  int tagCount;
  int rv;

  lbuf=GWEN_Buffer_new(0, AHB_SWIFT_MAXLINELEN, 0, 1);
  tagCount=0;

  /* read first line, should be empty */
  for (;;) {
    rv=_readDocLine(fb, buffer, sizeof(buffer)-1);
    if (rv<0) {
      if (rv==GWEN_ERROR_EOF) {
        GWEN_Buffer_free(lbuf);
        return 1;
      }
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading from stream (%d)", rv);
      GWEN_Buffer_free(lbuf);
      return rv;
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
      GWEN_FASTBUFFER_PEEKBYTE(fb, rv);
      if (rv<0) {
        if (rv==GWEN_ERROR_EOF) {
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
          DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
          GWEN_Buffer_free(lbuf);
          return rv;
        }
      }
      else {
        /* read next line */
        rv=_readDocLine(fb, buffer, sizeof(buffer)-1);
        if (rv<0) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Error reading from stream (%d)", rv);
          GWEN_Buffer_free(lbuf);
          return rv;
        }
      }

      /* check whether the line starts with a new tag (:123x:) */
      if (buffer[0]==':') {
        const char *s;

        if (strncasecmp(buffer, ":NS:", 4)==0) {
          break;
        }
        else {
          s=buffer+1;
          while (*s && isdigit(*s))
            s++;
          if (isalpha(*s))
            s++;
          if (*s==':') {
            DBG_DEBUG(AQBANKING_LOGDOMAIN, "End of tag reached");
            break;
          }
        }
      }

      /* check whether the line starts with a ":" or "-" */
      /* CAVEAT: Some banks use "-" at end of line in ":86:" tags, we must not take that for the end of document */
      if (
        (GWEN_Buffer_GetUsedBytes(lbuf)<4 || strncasecmp(GWEN_Buffer_GetStart(lbuf), ":86:", 4) !=0) &&
        buffer[0]=='-' && buffer[1]==0
      ) {
        /* it does, so the buffer contains the next line, go handle the
         * previous line */
        DBG_DEBUG(AQBANKING_LOGDOMAIN, "End of doc reached");
        break;
      }

      /* it doesn't, so there is a CR/LF inside the tag */
      if (GWEN_Buffer_GetUsedBytes(lbuf)>AHB_SWIFT_MAXLINELEN*4) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Too many bytes in line, maybe not SWIFT");
        GWEN_Buffer_free(lbuf);
        return -1;
      }

      GWEN_Buffer_AppendByte(lbuf, 10);
      GWEN_Buffer_AppendString(lbuf, buffer);
    } /* for */

    /* tag complete, parse it */
    p=GWEN_Buffer_GetStart(lbuf);

    if (*p!=':') {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Error in SWIFT data: no tag name");
      GWEN_Text_DumpString(GWEN_Buffer_GetStart(lbuf),
                           GWEN_Buffer_GetUsedBytes(lbuf), 2);
      GWEN_Buffer_free(lbuf);
      return -1;
    }
    p++;
    p2=p;
    while (*p2 && *p2!=':')
      p2++;
    if (*p2!=':') {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Error in SWIFT data: incomplete tag name");
      GWEN_Text_DumpString(GWEN_Buffer_GetStart(lbuf),
                           GWEN_Buffer_GetUsedBytes(lbuf), 2);
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


int _readDocument(GWEN_FAST_BUFFER *fb, AHB_SWIFT_TAG_LIST *tl, unsigned int maxTags)
{
  int rv;
  int c;
  int isFullSwift=0;
  int subDocs=0;

  /* check for first character being a curly bracket */
  for (;;) {
    GWEN_FASTBUFFER_PEEKBYTE(fb, c);
    if (c<0) {
      if (c==GWEN_ERROR_EOF) {
        DBG_INFO(AQBANKING_LOGDOMAIN,
                 "EOF met, empty document");
        return 1;
      }
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Error reading from BIO (%d)", c);
      return c;
    }
    if (c=='{') {
      isFullSwift=1;
      break;
    }
    else if (c>3)
      /* some SWIFT documents contain 01 at the beginning and 03 at the end,
       * we simply skip those characters here */
      break;
    GWEN_FASTBUFFER_READBYTE(fb, c);
  } /* for */

  if (isFullSwift) {
    /* read header, seek block 4 */
    for (;;) {
      int err;
      char swhead[4];
      unsigned int bsize;
      int curls=0;

      /* skip everything before curly bracket */
      for (;;) {
        GWEN_FASTBUFFER_PEEKBYTE(fb, c);
        if (c<0) {
          if (c==GWEN_ERROR_EOF) {
            DBG_INFO(AQBANKING_LOGDOMAIN, "EOF met, empty block");
            if (subDocs>0) {
              DBG_INFO(AQBANKING_LOGDOMAIN, "We got %d text blocks, returning", subDocs);
              return 0;
            }
            return 1;
          }
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Error reading from BIO (%d)", c);
          return c;
        }
        if (c=='{') {
          break;
        }
        GWEN_FASTBUFFER_READBYTE(fb, c);
      } /* for */

      /* read block start ("{n:...") */
      bsize=3;

      GWEN_FASTBUFFER_READFORCED(fb, err, swhead, bsize);
      if (err<0) {
        DBG_ERROR_ERR(AQBANKING_LOGDOMAIN, err);
        return err;
      }
      if (swhead[2]!=':') {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Not a SWIFT block");
        GWEN_Text_DumpString(swhead, 4, 2);
        return GWEN_ERROR_BAD_DATA;
      }

      /* handle block */
      DBG_DEBUG(0, "Reading block %d", swhead[1]-'0');
      if (swhead[1]=='4') {
        /* read document from block 4 */
        rv=_readTextBlock(fb, tl, maxTags);
        if (rv) {
          DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
          return rv;
        }
        subDocs++;
      }

      /* skip block */
      for (;;) {
        GWEN_FASTBUFFER_READBYTE(fb, c);
        if (c<0) {
          if (c==GWEN_ERROR_EOF) {
            DBG_ERROR(AQBANKING_LOGDOMAIN,
                      "EOF met (%d)", c);
            return GWEN_ERROR_EOF;
          }
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Error reading from BIO (%d)", c);
          return GWEN_ERROR_READ;
        }
        if (c=='{')
          curls++;
        else if (c=='}') {
          if (curls==0)
            break;
          else
            curls--;
        }
      } /* for */
    } /* for */
  }
  else {
    /* not a full swift document, just read the SWIFT document directly */
    rv=_readTextBlock(fb, tl, maxTags);
    if (rv)
      return rv;
  }

  return 0;
}



int _pluginImport(GWEN_DBIO *dbio,
                  GWEN_SYNCIO *sio,
                  GWEN_DB_NODE *data,
                  GWEN_DB_NODE *cfg,
                  uint32_t flags)
{
  int rv;
  const char *p;
  int skipFileLines;
  int skipDocLines;
  GWEN_FAST_BUFFER *fb;
  int docsImported=0;

  p=GWEN_DB_GetCharValue(cfg, "type", 0, "mt940");
  if (strcasecmp(p, "mt940")!=0 &&
      strcasecmp(p, "mt942")!=0 &&
      strcasecmp(p, "mt535")!=0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Type \"%s\" not supported by plugin \"%s\"",
              p,
              GWEN_DBIO_GetName(dbio));
    return GWEN_ERROR_INVALID;
  }

  skipFileLines=GWEN_DB_GetIntValue(cfg, "skipFileLines", 0, 0);
  skipDocLines=GWEN_DB_GetIntValue(cfg, "skipDocLines", 0, 0);

  fb=GWEN_FastBuffer_new(256, sio);

  /* skip lines at the beginning if requested */
  if (skipFileLines>0) {
    int i;
    GWEN_BUFFER *lbuf;

    lbuf=GWEN_Buffer_new(0, 256, 0, 1);
    for (i=0; i<skipFileLines; i++) {
      int err;

      err=GWEN_FastBuffer_ReadLineToBuffer(fb, lbuf);
      if (err<0) {
        if (err==GWEN_ERROR_EOF && i==0)
          break;
        DBG_INFO(AQBANKING_LOGDOMAIN,
                 "Error in report, aborting (%d)", err);
        GWEN_Buffer_free(lbuf);
        GWEN_FastBuffer_free(fb);
        return err;
      }
      GWEN_Buffer_Reset(lbuf);
    }
    GWEN_Buffer_free(lbuf);

    if (i<skipFileLines) {
      /* not enough lines to skip, assume end of document */

      GWEN_FastBuffer_free(fb);
      DBG_INFO(AQBANKING_LOGDOMAIN, "To few lines in file");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                           I18N("Empty SWIFT file, aborting"));
      return GWEN_ERROR_EOF;
    }
  }

  for (;;) {
    AHB_SWIFT_TAG_LIST *tl;

    /* check for user abort */
    rv=GWEN_Gui_ProgressAdvance(0, GWEN_GUI_PROGRESS_NONE);
    if (rv==GWEN_ERROR_USER_ABORTED) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "User aborted");
      GWEN_FastBuffer_free(fb);
      return rv;
    }

    GWEN_Gui_ProgressLog2(0, GWEN_LoggerLevel_Debug,
                          I18N("Reading SWIFT document %d"), docsImported+1);

    /* skip lines at the beginning if requested */
    if (skipDocLines>0) {
      int i;
      GWEN_BUFFER *lbuf;

      lbuf=GWEN_Buffer_new(0, 256, 0, 1);
      for (i=0; i<skipDocLines; i++) {
        int err;

        err=GWEN_FastBuffer_ReadLineToBuffer(fb, lbuf);
        if (err<0) {
          if (err==GWEN_ERROR_EOF && i==0)
            break;
          DBG_INFO(AQBANKING_LOGDOMAIN,
                   "Error in report, aborting (%d)", err);
          GWEN_Buffer_free(lbuf);
          GWEN_FastBuffer_free(fb);
          return err;
        }
        GWEN_Buffer_Reset(lbuf);
      }
      GWEN_Buffer_free(lbuf);

      if (i<skipDocLines)
        /* not enough lines to skip, assume end of document */
        break;
    }

    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Debug,
                         I18N("Parsing SWIFT data"));
    tl=AHB_SWIFT_Tag_List_new();
    assert(tl);
    rv=_readDocument(fb, tl, 0);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Error in report, aborting");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                           I18N("Error parsing SWIFT data"));
      GWEN_FastBuffer_free(fb);
      AHB_SWIFT_Tag_List_free(tl);
      return rv;
    }

    if (rv==1) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "End of document reached");
      AHB_SWIFT_Tag_List_free(tl);
      if (docsImported==0) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Empty document, aborting");
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                             I18N("Empty SWIFT document, aborting"));
        GWEN_FastBuffer_free(fb);
        return GWEN_ERROR_EOF;
      }
      break;
    }

    /* now all tags have been read, transform them */
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Debug,
                         I18N("Importing SWIFT data"));

    DBG_INFO(AQBANKING_LOGDOMAIN, "*** before branching ***");
    if (strcasecmp(p, "mt940")==0 || strcasecmp(p, "mt942")==0)
      rv=AHB_SWIFT940_Import(tl, data, cfg, flags);
    if (strcasecmp(p, "mt535")==0)
      rv=AHB_SWIFT535_Import(tl, data, cfg, flags);


    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Error importing SWIFT MT940/942/535");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                           I18N("Error importing SWIFT data"));
      GWEN_FastBuffer_free(fb);
      AHB_SWIFT_Tag_List_free(tl);
      return rv;
    }
    AHB_SWIFT_Tag_List_free(tl);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Debug,
                         I18N("Swift document successfully imported"));
    docsImported++;
  } /* for */

  GWEN_FastBuffer_free(fb);
  DBG_INFO(AQBANKING_LOGDOMAIN, "SWIFT MT940/942/535 successfully imported");

  return 0;
}



int _pluginExport(GWEN_DBIO *dbio,
                  GWEN_SYNCIO *sio,
                  GWEN_DB_NODE *data,
                  GWEN_DB_NODE *cfg,
                  uint32_t flags)
{
  DBG_ERROR(AQBANKING_LOGDOMAIN, "Export: Not yet implemented");
  return GWEN_ERROR_NOT_SUPPORTED;
}



GWEN_DBIO_CHECKFILE_RESULT _pluginCheckFile(GWEN_DBIO *dbio, const char *fname)
{
  int i;
  GWEN_DBIO_CHECKFILE_RESULT res;
  GWEN_BUFFER *lbuf;
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(dbio);
  assert(fname);

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  baseIo=sio;

  sio=GWEN_SyncIo_Buffered_new(baseIo);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    /* error */
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "open(%s): %s", fname, strerror(errno));
    GWEN_SyncIo_free(sio);
    return GWEN_DBIO_CheckFileResultNotOk;
  }

  /* search for ":20:" tag */
  lbuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=GWEN_DBIO_CheckFileResultNotOk;
  for (i=0; i<20; i++) {
    int err;
    const char *p;

    err=GWEN_SyncIo_Buffered_ReadLineToBuffer(sio, lbuf);
    if (err<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Error in report, aborting");
      res=GWEN_DBIO_CheckFileResultNotOk;
      break;
    }
    else if (err==0)
      break;
    p=GWEN_Buffer_GetStart(lbuf);
    if (strstr(p, ":20:")) {
      /* don't be too sure about the file being importable... */
      res=GWEN_DBIO_CheckFileResultUnknown;
      break;
    }
    GWEN_Buffer_Reset(lbuf);
  } /* for */
  GWEN_Buffer_free(lbuf);

  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  return res;
}



GWEN_DBIO *_pluginFactory(GWEN_PLUGIN *pl)
{
  GWEN_DBIO *dbio;

  dbio=GWEN_DBIO_new("swift", "Imports SWIFT data");
  GWEN_DBIO_SetImportFn(dbio, _pluginImport);
  GWEN_DBIO_SetExportFn(dbio, _pluginExport);
  GWEN_DBIO_SetCheckFileFn(dbio, _pluginCheckFile);
  return dbio;
}



GWEN_PLUGIN *dbio_swift_factory(GWEN_PLUGIN_MANAGER *pm,
                                const char *modName,
                                const char *fileName)
{
  GWEN_PLUGIN *pl;

  pl=GWEN_DBIO_Plugin_new(pm, modName, fileName);
  assert(pl);

  GWEN_DBIO_Plugin_SetFactoryFn(pl, _pluginFactory);

  return pl;

}





int AHB_SWIFT_SetCharValue(GWEN_DB_NODE *db, uint32_t flags, const char *name, const char *s)
{
  GWEN_BUFFER *vbuf;
  int rv;

  vbuf=GWEN_Buffer_new(0, strlen(s)+32, 0, 1);
  _iso8859_1ToUtf8(s, -1, vbuf);
  rv=GWEN_DB_SetCharValue(db, flags, name, GWEN_Buffer_GetStart(vbuf));
  GWEN_Buffer_free(vbuf);
  return rv;
}



void _iso8859_1ToUtf8(const char *p, int size, GWEN_BUFFER *buf)
{
  while (*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    if (c<32 || c==127)
      c=32;
    else {
      /* Dirty hack to support Unicode code points */
      /* U+00A0..U+00FF already in UTF-8 encoding. */
      /* E.g. German Umlaute from Consorsbank      */
      unsigned int c2 = (unsigned char)(*p);
      if ((c & ~0x01)==0xC2 && (c2 & ~0x3F)==0x80) {
        GWEN_Buffer_AppendByte(buf, c);
        c=(unsigned char)(*(p++));
      }
      else if (c & 0x80) {
        GWEN_Buffer_AppendByte(buf, 0xc0 | c>>6);
        c &= ~0x40;
      }
    }
    GWEN_Buffer_AppendByte(buf, c);
    if (size!=-1)
      size--;
  } /* while */
}



GWEN_DATE *AHB_SWIFT_ReadDateYYMMDD(const char **pCurrentChar, unsigned int *pBytesLeft)
{
  const char *p;
  unsigned int bleft;
  int dateYear, dateMonth, dateDay;
  GWEN_DATE *dt=NULL;

  p=*pCurrentChar;
  bleft=*pBytesLeft;

  if (bleft<6) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No date (%s)", p);
    return NULL;
  }
  dateYear=((p[0]-'0')*10) + (p[1]-'0');
  if (dateYear>AHB_SWIFT_CENTURY_CUTOFF_YEAR)
    dateYear+=1900;
  else
    dateYear+=2000;
  dateMonth=((p[2]-'0')*10) + (p[3]-'0');
  dateDay=((p[4]-'0')*10) + (p[5]-'0');

  dt=_dateFromYMD(dateYear, dateMonth, dateDay);
  if (dt==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Invalid date (%s)", p);
    return NULL;
  }

  p+=6;
  bleft-=6;

  *pCurrentChar=p;
  *pBytesLeft=bleft;
  return dt;
}



GWEN_DATE *AHB_SWIFT_ReadDateMMDDWithReference(const char **pCurrentChar, unsigned int *pBytesLeft, const GWEN_DATE *refDate)
{
  const char *p;
  unsigned int bleft;
  int dateYear, dateMonth, dateDay;
  GWEN_DATE *dt=NULL;

  p=*pCurrentChar;
  bleft=*pBytesLeft;

  if (*p && isdigit(*p)) {
    int refYear, refMonth;

    refYear=GWEN_Date_GetYear(refDate);
    refMonth=GWEN_Date_GetMonth(refDate);

    if (bleft<4) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Bad date (%s)", p);
      return NULL;
    }
    dateMonth=((p[0]-'0')*10) + (p[1]-'0');
    dateDay=((p[2]-'0')*10) + (p[3]-'0');
    /* use year from reference date.
     * However: if reference date and this date are in different years
     * the year might be too high.
     * We detect this case by comparing the months: If this month
     * and the reference month differ by more than 7 months then this year
     * will be adjusted.
     */
    if (dateMonth-refMonth>7) {
      /* this date before reference date */
      dateYear=refYear-1;
    }
    else if (refMonth-dateMonth>7) {
      /* reference date before this date */
      dateYear=refYear+1;
    }
    else
      dateYear=refYear;

    dt=_dateFromYMD(dateYear, dateMonth, dateDay);
    if (dt==NULL) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid date (%s)", p);
      return NULL;
    }
    p+=4;
    bleft-=4;
  }

  *pCurrentChar=p;
  *pBytesLeft=bleft;
  return dt;
}



GWEN_DATE *_dateFromYMD(int dateYear, int dateMonth, int dateDay)
{
  GWEN_DATE *dt=NULL;

  if (dateDay==30 && dateMonth==2) {
    /* date is Feb 30, this date is invalid. However, some banks use this
     * to indicate the last day of February, so we move along */
    dateDay=1;
    dateMonth=3;
    dt=GWEN_Date_fromGregorian(dateYear, dateMonth, dateDay);
    if (dt==NULL) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Bad date %04d/%02d/%02d", dateYear, dateMonth, dateDay);
      return NULL;
    }
    /* subtract a day to get the last day in FEB */
    GWEN_Date_SubDays(dt, 1);
  }
  else {
    dt=GWEN_Date_fromGregorian(dateYear, dateMonth, dateDay);
    if (dt==NULL) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Bad date %04d/%02d/%02d", dateYear, dateMonth, dateDay);
      return NULL;
    }
  }

  return dt;
}





