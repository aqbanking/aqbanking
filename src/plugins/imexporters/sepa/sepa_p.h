/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_IMEX_SEPA_P_H
#define AQHBCI_IMEX_SEPA_P_H


#include <aqbanking/imexporter_be.h>


typedef struct AH_IMEXPORTER_SEPA AH_IMEXPORTER_SEPA;
struct AH_IMEXPORTER_SEPA {
  int dummy;
};


AQBANKING_EXPORT
GWEN_PLUGIN *imexporter_sepa_factory(GWEN_PLUGIN_MANAGER *pm,
				     const char *name,
				     const char *fileName);

static
AB_IMEXPORTER *AB_Plugin_ImExporterSEPA_Factory(GWEN_PLUGIN *pl,
						AB_BANKING *ab);


static
void GWENHYWFAR_CB AH_ImExporterSEPA_FreeData(void *bp, void *p);


static
int AH_ImExporterSEPA_Import(AB_IMEXPORTER *ie,
			     AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_SYNCIO *sio,
			     GWEN_DB_NODE *params);

static
int AH_ImExporterSEPA_Export(AB_IMEXPORTER *ie,
			     AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_SYNCIO *sio,
			     GWEN_DB_NODE *params);

static
int AH_ImExporterSEPA_CheckFile(AB_IMEXPORTER *ie, const char *fname);


static
int AH_ImExporterSEPA_Export_Ccm(AB_IMEXPORTER *ie,
				 AB_IMEXPORTER_CONTEXT *ctx,
				 GWEN_SYNCIO *sio,
				 GWEN_DB_NODE *params);

static
int AH_ImExporterSEPA_Export_001_002_03(AB_IMEXPORTER *ie,
                                        AB_IMEXPORTER_CONTEXT *ctx,
                                        GWEN_SYNCIO *sio,
                                        GWEN_DB_NODE *params);

static
int AH_ImExporterSEPA_Export_008_001_01(AB_IMEXPORTER *ie,
                                        AB_IMEXPORTER_CONTEXT *ctx,
                                        GWEN_SYNCIO *sio,
                                        GWEN_DB_NODE *params);


static
int AH_ImExporterSEPA_Export_008_003_02(AB_IMEXPORTER *ie,
                                        AB_IMEXPORTER_CONTEXT *ctx,
                                        GWEN_SYNCIO *sio,
                                        GWEN_DB_NODE *params);


#endif /* AQHBCI_IMEX_SEPA_P_H */
