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


#ifndef AQBANKING_PLUGIN_OFX_P_H
#define AQBANKING_PLUGIN_OFX_P_H

#include <aqbanking/imexporter.h>
#include <libofx/libofx.h>


typedef struct AH_IMEXPORTER_OFX AH_IMEXPORTER_OFX;
struct AH_IMEXPORTER_OFX {
  GWEN_DB_NODE *dbData;
  AB_IMEXPORTER_CONTEXT *context;
  AB_IMEXPORTER_ACCOUNTINFO *lastAccountInfo;
};


AB_IMEXPORTER* ofx_factory(AB_BANKING *ab, GWEN_DB_NODE *db);

void GWENHYWFAR_CB AH_ImExporterOFX_FreeData(void *bp, void *p);

int AH_ImExporterOFX_Import(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_IO_LAYER *io,
			    GWEN_DB_NODE *params,
			    uint32_t guiid);

int AH_ImExporterOFX_CheckFile(AB_IMEXPORTER *ie, const char *fname, uint32_t guiid);


int AH_ImExporterOFX_StatusCallback_cb(const struct OfxStatusData data,
                                       void *user_data);
int AH_ImExporterOFX_AccountCallback_cb(const struct OfxAccountData data,
                                        void *user_data);
int AH_ImExporterOFX_SecurityCallback_cb(const struct OfxSecurityData data,
                                         void *user_data);
int AH_ImExporterOFX_TransactionCallback_cb(const struct OfxTransactionData data,
                                           void *user_data);
int AH_ImExporterOFX_StatementCallback_cb(const struct OfxStatementData data,
                                          void *user_data);


#endif /* AQBANKING_PLUGIN_OFX_P_H */



