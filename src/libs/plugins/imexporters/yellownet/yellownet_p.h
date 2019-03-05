/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_IMEX_YN_P_H
#define AQHBCI_IMEX_YN_P_H


#include "yellownet.h"

#include <gwenhywfar/dbio.h>
#include <aqbanking/backendsupport/imexporter_be.h>


typedef struct AB_IMEXPORTER_YELLOWNET AB_IMEXPORTER_YELLOWNET;
struct AB_IMEXPORTER_YELLOWNET {
  int dummy;
};


static void GWENHYWFAR_CB AB_ImExporterYN_FreeData(void *bp, void *p);

static int AB_ImExporterYN_Import(AB_IMEXPORTER *ie,
                                  AB_IMEXPORTER_CONTEXT *ctx,
                                  GWEN_SYNCIO *sio,
                                  GWEN_DB_NODE *params);

static int AB_ImExporterYN_CheckFile(AB_IMEXPORTER *ie, const char *fname);


static AB_VALUE *AB_ImExporterYN__ReadValue(AB_IMEXPORTER *ie,
                                            GWEN_XMLNODE *node,
                                            int value);

static GWEN_DATE *AB_ImExporterYN__ReadDate(AB_IMEXPORTER *ie, GWEN_XMLNODE *node, int value);

static AB_IMEXPORTER_ACCOUNTINFO *AB_ImExporterYN__ReadAccountInfo(AB_IMEXPORTER *ie,
                                                                   AB_IMEXPORTER_CONTEXT *ctx,
                                                                   GWEN_XMLNODE *doc);


static AB_TRANSACTION *AB_ImExporterYN__ReadLNE_LNS(AB_IMEXPORTER *ie,
                                                    AB_IMEXPORTER_ACCOUNTINFO *ai,
                                                    GWEN_XMLNODE *node);

static int AB_ImExporterYN__ReadTransactions(AB_IMEXPORTER *ie,
                                             AB_IMEXPORTER_ACCOUNTINFO *ai,
                                             GWEN_XMLNODE *doc);

static int AB_ImExporterYN__ReadAccountStatus(AB_IMEXPORTER *ie,
                                              AB_IMEXPORTER_ACCOUNTINFO *ai,
                                              GWEN_XMLNODE *doc);


#endif /* AQHBCI_IMEX_YN_P_H */
