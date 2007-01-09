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


#ifndef AQHBCI_IMEX_OHBCI1_P_H
#define AQHBCI_IMEX_OHBCI1_P_H


#include <gwenhywfar/dbio.h>
#include <aqbanking/imexporter_be.h>


typedef struct AH_IMEXPORTER_OPENHBCI1 AH_IMEXPORTER_OPENHBCI1;
struct AH_IMEXPORTER_OPENHBCI1 {
  GWEN_DB_NODE *dbData;
  GWEN_DBIO *dbio;
};


AB_IMEXPORTER* csv_factory(AB_BANKING *ab, GWEN_DB_NODE *db);
void GWENHYWFAR_CB AH_ImExporterOpenHBCI1_FreeData(void *bp, void *p);

int AH_ImExporterOpenHBCI1_Import(AB_IMEXPORTER *ie,
                                  AB_IMEXPORTER_CONTEXT *ctx,
                                  GWEN_BUFFEREDIO *bio,
                                  GWEN_DB_NODE *params);
int AH_ImExporterOpenHBCI1_Export(AB_IMEXPORTER *ie,
                                  AB_IMEXPORTER_CONTEXT *ctx,
                                  GWEN_BUFFEREDIO *bio,
                                  GWEN_DB_NODE *params);

int AH_ImExporterOpenHBCI1_CheckFile(AB_IMEXPORTER *ie, const char *fname);


int AH_ImExporterOpenHBCI1__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                            GWEN_DB_NODE *db,
                                            GWEN_DB_NODE *dbParams);

#endif /* AQHBCI_IMEX_OHBCI1_P_H */
