/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_IMEX_XMLDB_P_H
#define AQHBCI_IMEX_XMLDB_P_H


#include "xmldb.h"

#include <gwenhywfar/dbio.h>
#include <aqbanking/backendsupport/imexporter_be.h>


typedef struct AH_IMEXPORTER_XMLDB AH_IMEXPORTER_XMLDB;
struct AH_IMEXPORTER_XMLDB {
  GWEN_DBIO *dbio;
};


static void GWENHYWFAR_CB AH_ImExporterXMLDB_FreeData(void *bp, void *p);


static int AH_ImExporterXMLDB_Import(AB_IMEXPORTER *ie,
                                     AB_IMEXPORTER_CONTEXT *ctx,
                                     GWEN_SYNCIO *sio,
                                     GWEN_DB_NODE *params);

static int AH_ImExporterXMLDB_Export(AB_IMEXPORTER *ie,
                                     AB_IMEXPORTER_CONTEXT *ctx,
                                     GWEN_SYNCIO *sio,
                                     GWEN_DB_NODE *params);

static int AH_ImExporterXMLDB_CheckFile(AB_IMEXPORTER *ie, const char *fname);


#endif /* AQHBCI_IMEX_XMLDB_P_H */
