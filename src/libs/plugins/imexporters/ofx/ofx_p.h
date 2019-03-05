/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_PLUGIN_OFX_P_H
#define AQBANKING_PLUGIN_OFX_P_H

#include "ofx.h"

#include <aqbanking/backendsupport/imexporter_be.h>


typedef struct AH_IMEXPORTER_OFX AH_IMEXPORTER_OFX;
struct AH_IMEXPORTER_OFX {
  GWEN_DB_NODE *dbData;
  AB_IMEXPORTER_CONTEXT *context;
};


static void GWENHYWFAR_CB AH_ImExporterOFX_FreeData(void *bp, void *p);

static int AH_ImExporterOFX_Import(AB_IMEXPORTER *ie,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   GWEN_SYNCIO *sio,
                                   GWEN_DB_NODE *params);

static int AH_ImExporterOFX_CheckFile(AB_IMEXPORTER *ie, const char *fname);


#endif /* AQBANKING_PLUGIN_OFX_P_H */



