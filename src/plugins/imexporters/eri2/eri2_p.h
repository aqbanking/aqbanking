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

/* for debugging */
#define ERI2DEBUG

#define AH_IMEXPORTER_ERI2_MAXLEVEL 16

#include <aqbanking/imexporter_be.h>
#include <aqbanking/banking.h>
#include <gwenhywfar/msgengine.h>


typedef struct AB_IMEXPORTER_ERI2 AB_IMEXPORTER_ERI2;
struct AB_IMEXPORTER_ERI2 {
  GWEN_MSGENGINE *msgEngine;
};


AQBANKING_EXPORT
GWEN_PLUGIN *imexporter_eri2_factory(GWEN_PLUGIN_MANAGER *pm,
				     const char *name,
				     const char *fileName);

static
AB_IMEXPORTER *AB_Plugin_ImExporterERI2_Factory(GWEN_PLUGIN *pl,
						AB_BANKING *ab);

static
void GWENHYWFAR_CB AB_ImExporterERI2_FreeData(void *bp, void *p);

static
int AB_ImExporterERI2_Import(AB_IMEXPORTER *ie,
                             AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_SYNCIO *sio,
			     GWEN_DB_NODE *params);

static
int AB_ImExporterERI2__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
				       GWEN_DB_NODE *db,
				       GWEN_DB_NODE *dbParams);

static
int AB_ImExporterERI2__HandleRec1(GWEN_DB_NODE *dbT,
                                  GWEN_DB_NODE *dbParams,
                                  AB_TRANSACTION *t);

static
int AB_ImExporterERI2__HandleRec2(GWEN_DB_NODE *dbT,
                                  GWEN_DB_NODE *dbParams,
                                  AB_TRANSACTION *t);

static
int AB_ImExporterERI2__HandleRec3(GWEN_DB_NODE *dbT,
                                  GWEN_DB_NODE *dbParams,
                                  AB_TRANSACTION *t);

static
int AB_ImExporterERI2__HandleRec4(GWEN_DB_NODE *dbT,
                                  GWEN_DB_NODE *dbParams,
                                  AB_TRANSACTION *t);

static
void AB_ImExporterERI2__AddPurpose(AB_TRANSACTION *t, const char *s);

static
void AB_ImExporterERI2__AddTransaction(AB_IMEXPORTER_CONTEXT *ctx,
                                       AB_TRANSACTION *t,
                                       GWEN_DB_NODE *params);

static
int AB_ImExporterERI2_CheckFile(AB_IMEXPORTER *ie, const char *fname);

static
int AB_ImExporterERI2_Export(AB_IMEXPORTER *ie,
			     AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_SYNCIO *sio,
			     GWEN_DB_NODE *params);





#endif /* AQBANKING_IMEX_ERI2_P_H */
