/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: xmldb_p.h 566 2005-08-23 06:25:03Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_IMEX_XMLDB_P_H
#define AQHBCI_IMEX_XMLDB_P_H


#include <gwenhywfar/dbio.h>
#include <aqbanking/imexporter_be.h>


typedef struct AH_IMEXPORTER_XMLDB AH_IMEXPORTER_XMLDB;
struct AH_IMEXPORTER_XMLDB {
  GWEN_DB_NODE *dbData;
  GWEN_DBIO *dbio;
};


AB_IMEXPORTER* xmldb_factory(AB_BANKING *ab, GWEN_DB_NODE *db);
void GWENHYWFAR_CB AH_ImExporterXMLDB_FreeData(void *bp, void *p);

int AH_ImExporterXMLDB_Import(AB_IMEXPORTER *ie,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              GWEN_BUFFEREDIO *bio,
                              GWEN_DB_NODE *params);

int AH_ImExporterXMLDB_Export(AB_IMEXPORTER *ie,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              GWEN_BUFFEREDIO *bio,
                              GWEN_DB_NODE *params);

int AH_ImExporterXMLDB_CheckFile(AB_IMEXPORTER *ie, const char *fname);


#endif /* AQHBCI_IMEX_XMLDB_P_H */
