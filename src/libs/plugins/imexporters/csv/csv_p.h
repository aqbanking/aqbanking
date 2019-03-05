/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_IMEX_CSV_P_H
#define AQHBCI_IMEX_CSV_P_H

#include "csv.h"

#include <gwenhywfar/dbio.h>
#include <aqbanking/backendsupport/imexporter_be.h>


typedef struct AH_IMEXPORTER_CSV AH_IMEXPORTER_CSV;
struct AH_IMEXPORTER_CSV {
  GWEN_DBIO *dbio;
};


static void GWENHYWFAR_CB AH_ImExporterCSV_FreeData(void *bp, void *p);

static int AH_ImExporterCSV_Import(AB_IMEXPORTER *ie,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   GWEN_SYNCIO *sio,
                                   GWEN_DB_NODE *params);

static int AH_ImExporterCSV_Export(AB_IMEXPORTER *ie,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   GWEN_SYNCIO *sio,
                                   GWEN_DB_NODE *params);

static int AH_ImExporterCSV_CheckFile(AB_IMEXPORTER *ie, const char *fname);

static int AH_ImExporterCSV_GetEditProfileDialog(AB_IMEXPORTER *ie,
                                                 GWEN_DB_NODE *params,
                                                 const char *testFileName,
                                                 GWEN_DIALOG **pDlg);

static int AH_ImExporterCSV__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                             GWEN_DB_NODE *db,
                                             GWEN_DB_NODE *dbParams);

static AB_VALUE *AH_ImExporterCSV__ValueFromDb(GWEN_DB_NODE *dbV,
                                               int commaThousands,
                                               int commaDecimal);


#endif /* AQHBCI_IMEX_CSV_P_H */
