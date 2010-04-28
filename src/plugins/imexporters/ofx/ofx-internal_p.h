/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: ofx_p.h 1396 2007-11-22 17:37:27Z martin $
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_PLUGIN_OFX_P_H
#define AQBANKING_PLUGIN_OFX_P_H

#include <aqbanking/imexporter_be.h>


typedef struct AH_IMEXPORTER_OFX AH_IMEXPORTER_OFX;
struct AH_IMEXPORTER_OFX {
  GWEN_DB_NODE *dbData;
  AB_IMEXPORTER_CONTEXT *context;
};


AQBANKING_EXPORT
GWEN_PLUGIN *imexporter_ofx_factory(GWEN_PLUGIN_MANAGER *pm,
				    const char *name,
				    const char *fileName);

static
AB_IMEXPORTER *AB_Plugin_ImExporterOFX_Factory(GWEN_PLUGIN *pl,
					       AB_BANKING *ab);

static
void GWENHYWFAR_CB AH_ImExporterOFX_FreeData(void *bp, void *p);

static
int AH_ImExporterOFX_Import(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_SYNCIO *sio,
			    GWEN_DB_NODE *params);

static
int AH_ImExporterOFX_CheckFile(AB_IMEXPORTER *ie, const char *fname);


#endif /* AQBANKING_PLUGIN_OFX_P_H */



