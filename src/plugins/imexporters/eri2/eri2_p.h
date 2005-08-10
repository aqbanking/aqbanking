/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Thu 21-07-2005
    copyright   : (C) 2005 by Peter de Vrijer
    email       : pdevrijer@home.nl

 ***************************************************************************
 *    Please see the file COPYING in this directory for license details    *
 ***************************************************************************/

#ifndef AQBANKING_IMEX_ERI2_P_H
#define AQBANKING_IMEX_ERI2_P_H

#define AB_ERI2_XMLFILE "eriformat.xml"

#include <aqbanking/imexporter_be.h>
#include <aqbanking/banking.h>
#include <gwenhywfar/msgengine.h>


typedef struct AB_IMEXPORTER_ERI2 AB_IMEXPORTER_ERI2;
struct AB_IMEXPORTER_ERI2 {
  GWEN_DB_NODE *dbData;
  GWEN_MSGENGINE *msgEngine;
};


AB_IMEXPORTER *eri2_factory(AB_BANKING *ab, GWEN_DB_NODE *db);
void AB_ImExporterERI2_FreeData(void *bp, void *p);
int AB_ImExporterERI2_Import(AB_IMEXPORTER *ie,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             GWEN_BUFFEREDIO *bio,
                             GWEN_DB_NODE *params);
int AB_ImExporterERI2__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                       GWEN_DB_NODE *db,
                                       GWEN_DB_NODE *dbParams);
int AB_ImExporterERI2__HandleRec1(GWEN_DB_NODE *dbT,
                                  GWEN_DB_NODE *dbParams,
                                  AB_TRANSACTION *t);
int AB_ImExporterERI2__HandleRec2(GWEN_DB_NODE *dbT,
                                  GWEN_DB_NODE *dbParams,
                                  AB_TRANSACTION *t);
int AB_ImExporterERI2__HandleRec3(GWEN_DB_NODE *dbT,
                                  GWEN_DB_NODE *dbParams,
                                  AB_TRANSACTION *t);
void AB_ImExporterERI2__AddPurpose(AB_TRANSACTION *t, const char *s);
void AB_ImExporterERI2__AddTransaction(AB_IMEXPORTER_CONTEXT *ctx,
                                       AB_TRANSACTION *t,
                                       GWEN_DB_NODE *params);

int AB_ImExporterERI2_CheckFile(AB_IMEXPORTER *ie, const char *fname);
int AB_ImExporterERI2_Export(AB_IMEXPORTER *ie,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             GWEN_BUFFEREDIO *bio,
                             GWEN_DB_NODE *params);







#endif /* AQBANKING_IMEX_ERI2_P_H */
