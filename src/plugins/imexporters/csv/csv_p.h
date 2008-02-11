/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_IMEX_CSV_P_H
#define AQHBCI_IMEX_CSV_P_H

#include <gwenhywfar/dbio.h>
#include <aqbanking/imexporter_be.h>


typedef struct AH_IMEXPORTER_CSV AH_IMEXPORTER_CSV;
struct AH_IMEXPORTER_CSV {
  GWEN_DB_NODE *dbData;
  GWEN_DBIO *dbio;
};


static
AB_IMEXPORTER* csv_factory(AB_BANKING *ab, GWEN_DB_NODE *db);

static
void GWENHYWFAR_CB AH_ImExporterCSV_FreeData(void *bp, void *p);

static
int AH_ImExporterCSV_Import(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_IO_LAYER *io,
			    GWEN_DB_NODE *params,
			    uint32_t guiid);

static
int AH_ImExporterCSV_Export(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_IO_LAYER *io,
			    GWEN_DB_NODE *params,
			    uint32_t guiid);

static
int AH_ImExporterCSV_CheckFile(AB_IMEXPORTER *ie, const char *fname, uint32_t guiid);


static
int AH_ImExporterCSV__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                      GWEN_DB_NODE *db,
				      GWEN_DB_NODE *dbParams,
				      uint32_t guiid);

static
int AH_ImExporterCSV__Transform_Var(GWEN_DB_NODE *db, int level);

static
int AH_ImExporterCSV__Transform_Group(GWEN_DB_NODE *db, int level);


static
int AH_ImExporterCSV__ExportTransactions(AB_IMEXPORTER *ie,
					 AB_IMEXPORTER_CONTEXT *ctx,
					 GWEN_IO_LAYER *io,
					 GWEN_DB_NODE *params,
					 uint32_t guiid);

static
int AH_ImExporterCSV__ExportNotedTransactions(AB_IMEXPORTER *ie,
					      AB_IMEXPORTER_CONTEXT *ctx,
					      GWEN_IO_LAYER *io,
					      GWEN_DB_NODE *params,
					      uint32_t guiid);


#endif /* AQHBCI_IMEX_CSV_P_H */
