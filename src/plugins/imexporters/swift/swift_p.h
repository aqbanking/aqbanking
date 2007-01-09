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


#ifndef AQHBCI_IMEX_SWIFT_P_H
#define AQHBCI_IMEX_SWIFT_P_H


#include <gwenhywfar/dbio.h>
#include <aqbanking/imexporter_be.h>


typedef struct AH_IMEXPORTER_SWIFT AH_IMEXPORTER_SWIFT;
struct AH_IMEXPORTER_SWIFT {
  GWEN_DB_NODE *dbData;
  GWEN_DBIO *dbio;
};


AB_IMEXPORTER* swift_factory(AB_BANKING *ab, GWEN_DB_NODE *db);
void GWENHYWFAR_CB AH_ImExporterSWIFT_FreeData(void *bp, void *p);

int AH_ImExporterSWIFT_Import(AB_IMEXPORTER *ie,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              GWEN_BUFFEREDIO *bio,
                              GWEN_DB_NODE *params);

int AH_ImExporterSWIFT_CheckFile(AB_IMEXPORTER *ie, const char *fname);


int AH_ImExporterSWIFT__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                        GWEN_DB_NODE *db,
                                        GWEN_DB_NODE *dbParams);


#endif /* AQHBCI_IMEX_SWIFT_P_H */
