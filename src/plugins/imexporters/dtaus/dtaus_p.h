/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_IMEX_DTAUS_P_H
#define AQHBCI_IMEX_DTAUS_P_H


#include <gwenhywfar/dbio.h>
#include <aqbanking/imexporter_be.h>


typedef struct AH_IMEXPORTER_DTAUS AH_IMEXPORTER_DTAUS;
struct AH_IMEXPORTER_DTAUS {
  GWEN_DBIO *dbio;
};


AQBANKING_EXPORT
GWEN_PLUGIN *imexporter_dtaus_factory(GWEN_PLUGIN_MANAGER *pm,
				      const char *name,
				      const char *fileName);

static
AB_IMEXPORTER *AB_Plugin_ImExporterDTAUS_Factory(GWEN_PLUGIN *pl,
						 AB_BANKING *ab);

static
void GWENHYWFAR_CB AH_ImExporterDTAUS_FreeData(void *bp, void *p);

static
int AH_ImExporterDTAUS_Import(AB_IMEXPORTER *ie,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              GWEN_SYNCIO *sio,
			      GWEN_DB_NODE *params);

static
int AH_ImExporterDTAUS_Export(AB_IMEXPORTER *ie,
			      AB_IMEXPORTER_CONTEXT *ctx,
			      GWEN_SYNCIO *sio,
			      GWEN_DB_NODE *params);

static
int AH_ImExporterDTAUS_CheckFile(AB_IMEXPORTER *ie, const char *fname);

static
int AH_ImExporterDTAUS__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                        GWEN_DB_NODE *db,
                                        GWEN_DB_NODE *dbParams);

#endif /* AQHBCI_IMEX_DTAUS_P_H */
