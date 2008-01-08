/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: ofx.c 1411 2008-01-06 17:54:41Z martin $
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ofx-internal_p.h"

#include "ofxxmlctx_l.h"

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>
#include <aqbanking/imexporter_be.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/xml.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif



GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_OFX);


AB_IMEXPORTER *ofx_factory(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_OFX *ieh;

  ie=AB_ImExporter_new(ab, "ofx");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_OFX, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_OFX, ie, ieh,
		       AH_ImExporterOFX_FreeData);
  ieh->dbData=db;

  AB_ImExporter_SetImportFn(ie, AH_ImExporterOFX_Import);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterOFX_CheckFile);
  return ie;
}



void GWENHYWFAR_CB AH_ImExporterOFX_FreeData(void *bp, void *p){
  AH_IMEXPORTER_OFX *ieh;

  ieh=(AH_IMEXPORTER_OFX*)p;
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterOFX_Import(AB_IMEXPORTER *ie,
			    AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_IO_LAYER *io,
			    GWEN_DB_NODE *params,
			    uint32_t guiid){
  AH_IMEXPORTER_OFX *ieh;
  int rv;
  GWEN_XML_CONTEXT *xmlCtx;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_OFX, ie);
  assert(ieh);

  /* this context does the real work, it sets some callbacks which
   * make GWEN's normal XML code read an OFX file */
  xmlCtx=AIO_OfxXmlCtx_new(0, guiid, GWEN_TIMEOUT_FOREVER, ctx);
  assert(xmlCtx);

  /* read OFX file into context */
  rv=GWEN_XML_ReadFromIo(xmlCtx, io);
  GWEN_XmlCtx_free(xmlCtx);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AH_ImExporterOFX_CheckFile(AB_IMEXPORTER *ie, const char *fname, uint32_t guiid){
  int fd;
  GWEN_BUFFEREDIO *bio;

  assert(ie);
  assert(fname);

  fd=open(fname, O_RDONLY);
  if (fd==-1) {
    /* error */
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "open(%s): %s", fname, strerror(errno));
    return GWEN_ERROR_NOT_FOUND;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 256);

  while(!GWEN_BufferedIO_CheckEOF(bio)) {
    char lbuffer[256];
    int err;

    err=GWEN_BufferedIO_ReadLine(bio, lbuffer, sizeof(lbuffer));
    if (err) {
      DBG_INFO(AQBANKING_LOGDOMAIN,
	       "File \"%s\" is not supported by this plugin",
               fname);
      GWEN_BufferedIO_Close(bio);
      GWEN_BufferedIO_free(bio);
      return GWEN_ERROR_BAD_DATA;
    }
    if (-1!=GWEN_Text_ComparePattern(lbuffer, "*<OFX>*", 0) ||
        -1!=GWEN_Text_ComparePattern(lbuffer, "*<OFC>*", 0)) {
      /* match */
      DBG_INFO(AQBANKING_LOGDOMAIN,
               "File \"%s\" is supported by this plugin",
               fname);
      GWEN_BufferedIO_Close(bio);
      GWEN_BufferedIO_free(bio);
      return 0;
    }
  } /* while */

  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  return GWEN_ERROR_BAD_DATA;
}





