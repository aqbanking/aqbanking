/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_IMEX_SWIFT_P_H
#define AQHBCI_IMEX_SWIFT_P_H


#include <gwenhywfar/dbio.h>
#include <aqbanking/imexporter_be.h>


typedef struct AH_IMEXPORTER_SWIFT AH_IMEXPORTER_SWIFT;
struct AH_IMEXPORTER_SWIFT {
  GWEN_DBIO *dbio;
};


AQBANKING_EXPORT
GWEN_PLUGIN *imexporter_swift_factory(GWEN_PLUGIN_MANAGER *pm,
				      const char *name,
				      const char *fileName);

static
AB_IMEXPORTER *AB_Plugin_ImExporterSWIFT_Factory(GWEN_PLUGIN *pl,
						 AB_BANKING *ab);


static
void GWENHYWFAR_CB AH_ImExporterSWIFT_FreeData(void *bp, void *p);

static
int AH_ImExporterSWIFT_Import(AB_IMEXPORTER *ie,
                              AB_IMEXPORTER_CONTEXT *ctx,
			      GWEN_SYNCIO *sio,
			      GWEN_DB_NODE *params);

static
int AH_ImExporterSWIFT_CheckFile(AB_IMEXPORTER *ie, const char *fname);


static
int AH_ImExporterSWIFT__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                        GWEN_DB_NODE *db,
					GWEN_DB_NODE *dbParams);


#endif /* AQHBCI_IMEX_SWIFT_P_H */
