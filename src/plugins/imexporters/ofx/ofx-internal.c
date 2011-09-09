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
#include <gwenhywfar/syncio_file.h>

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



GWEN_PLUGIN *imexporter_ofx_factory(GWEN_PLUGIN_MANAGER *pm,
				    const char *name,
				    const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_ImExporter_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_ImExporter_SetFactoryFn(pl, AB_Plugin_ImExporterOFX_Factory);

  return pl;
}



AB_IMEXPORTER *AB_Plugin_ImExporterOFX_Factory(GWEN_PLUGIN *pl,
					       AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_OFX *ieh;

  ie=AB_ImExporter_new(ab, "ofx");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_OFX, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_OFX, ie, ieh,
		       AH_ImExporterOFX_FreeData);
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
                            GWEN_SYNCIO *sio,
			    GWEN_DB_NODE *params){
  AH_IMEXPORTER_OFX *ieh;
  int rv;
  GWEN_XML_CONTEXT *xmlCtx;
  const char *s;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_OFX, ie);
  assert(ieh);

  /* this context does the real work, it sets some callbacks which
   * make GWEN's normal XML code read an OFX file */
  xmlCtx=AIO_OfxXmlCtx_new(0, ctx);
  assert(xmlCtx);

  /* possibly set charset */
  s=GWEN_DB_GetCharValue(params, "charset", 0, NULL);
  if (s && *s)
    AIO_OfxXmlCtx_SetCharset(xmlCtx, s);

  /* read OFX file into context */
  rv=GWEN_XMLContext_ReadFromIo(xmlCtx, sio);
  GWEN_XmlCtx_free(xmlCtx);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AH_ImExporterOFX_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  AH_IMEXPORTER_OFX *ieh;
  GWEN_SYNCIO *sio;
  int rv;
  uint8_t tbuf[256];

  assert(ie);
  assert(fname);

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_OFX, ie);
  assert(ieh);

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  rv=GWEN_SyncIo_Read(sio, tbuf, sizeof(tbuf)-1);
  if (rv<1) {
    DBG_INFO(GWEN_LOGDOMAIN,
	     "File \"%s\" is not supported by this plugin",
	     fname);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return GWEN_ERROR_BAD_DATA;
  }
  tbuf[rv-1]=0;
  if (-1!=GWEN_Text_ComparePattern((const char*)tbuf, "*<OFX>*", 0) ||
      -1!=GWEN_Text_ComparePattern((const char*)tbuf, "*<OFC>*", 0)) {
    /* match */
    DBG_INFO(GWEN_LOGDOMAIN,
	     "File \"%s\" is supported by this plugin",
	     fname);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return 0;
  }
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);
  return GWEN_ERROR_BAD_DATA;
}





