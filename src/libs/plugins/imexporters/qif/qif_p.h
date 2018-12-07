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


#ifndef AQHBCI_IMEX_QIF_P_H
#define AQHBCI_IMEX_QIF_P_H


#include <gwenhywfar/db.h>
#include <aqbanking/imexporter_be.h>
#include <aqbanking/banking.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>


typedef struct AH_IMEXPORTER_QIF AH_IMEXPORTER_QIF;
struct AH_IMEXPORTER_QIF {
  GWEN_DB_NODE *dbData;
  AB_IMEXPORTER_ACCOUNTINFO *currentAccount;
};


AB_IMEXPORTER* qif_factory(AB_BANKING *ab, GWEN_DB_NODE *db);
void GWENHYWFAR_CB AH_ImExporterQIF_FreeData(void *bp, void *p);

int AH_ImExporterQIF_Import(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_BUFFEREDIO *bio,
                            GWEN_DB_NODE *params);
int AH_ImExporterQIF_Export(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_BUFFEREDIO *bio,
                            GWEN_DB_NODE *params);

int AH_ImExporterQIF_CheckFile(AB_IMEXPORTER *ie, const char *fname);



int AH_ImExporterQIF__ImportAccount(AB_IMEXPORTER *ie,
                                    AB_IMEXPORTER_CONTEXT *ctx,
                                    GWEN_BUFFEREDIO *bio,
                                    GWEN_BUFFER *buf,
                                    GWEN_DB_NODE *params);



#endif /* AQHBCI_IMEX_QIF_P_H */
