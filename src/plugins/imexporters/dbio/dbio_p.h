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


#ifndef AQHBCI_IMEX_DBIO_P_H
#define AQHBCI_IMEX_DBIO_P_H


#include <gwenhywfar/dbio.h>
#include <aqbanking/imexporter_be.h>


typedef struct AH_IMEXPORTER_DBIO AH_IMEXPORTER_DBIO;
struct AH_IMEXPORTER_DBIO {
  GWEN_DB_NODE *dbData;
};


AB_IMEXPORTER* dbio_factory(AB_BANKING *ab, GWEN_DB_NODE *db);
void AH_ImExporterDBIO_FreeData(void *bp, void *p);

int AH_ImExporterDBIO_Import(AB_IMEXPORTER *ie,
			     AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_BUFFEREDIO *bio,
			     GWEN_DB_NODE *params);

int AH_ImExporterDBIO__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                       GWEN_DB_NODE *db,
                                       GWEN_DB_NODE *dbParams);

#endif /* AQHBCI_IMEX_DBIO_P_H */
