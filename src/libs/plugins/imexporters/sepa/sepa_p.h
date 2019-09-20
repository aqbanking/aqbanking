/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_IMEX_SEPA_P_H
#define AQHBCI_IMEX_SEPA_P_H


#include "sepa.h"

#include <aqbanking/backendsupport/imexporter_be.h>


typedef struct AH_IMEXPORTER_SEPA AH_IMEXPORTER_SEPA;
struct AH_IMEXPORTER_SEPA {
  int dummy;
};

typedef struct AH_IMEXPORTER_SEPA_PMTINF AH_IMEXPORTER_SEPA_PMTINF;
struct AH_IMEXPORTER_SEPA_PMTINF {
  GWEN_LIST_ELEMENT(AH_IMEXPORTER_SEPA_PMTINF)
  int tcount;
  AB_VALUE *value;
  char *ctrlsum;
  const GWEN_DATE *date;
  uint32_t transDate;
  const char *localName;
  const char *localIban;
  const char *localBic;
  AB_TRANSACTION_SEQUENCE sequenceType;
  const char *creditorSchemeId;
  AB_TRANSACTION_LIST2 *transactions;
};

/* these functions are not part of the public API */
static void AH_ImExporter_Sepa_PmtInf_free(AH_IMEXPORTER_SEPA_PMTINF *pmtinf);
GWEN_LIST_FUNCTION_DEFS(AH_IMEXPORTER_SEPA_PMTINF, AH_ImExporter_Sepa_PmtInf)


static void GWENHYWFAR_CB AH_ImExporterSEPA_FreeData(void *bp, void *p);


static int AH_ImExporterSEPA_Import(AB_IMEXPORTER *ie,
                                    AB_IMEXPORTER_CONTEXT *ctx,
                                    GWEN_SYNCIO *sio,
                                    GWEN_DB_NODE *params);

static int AH_ImExporterSEPA_Export(AB_IMEXPORTER *ie,
                                    AB_IMEXPORTER_CONTEXT *ctx,
                                    GWEN_SYNCIO *sio,
                                    GWEN_DB_NODE *params);

static int AH_ImExporterSEPA_CheckFile(AB_IMEXPORTER *ie, const char *fname);


static int AH_ImExporterSEPA_XmlSetCharValueEscaped(GWEN_XMLNODE *n, const char *varName, const char *value);


static int AH_ImExporterSEPA_Export_Pain_001(AB_IMEXPORTER *ie,
                                             AB_IMEXPORTER_CONTEXT *ctx,
                                             GWEN_XMLNODE *painNode,
                                             uint32_t doctype[],
                                             GWEN_DB_NODE *params);

static int AH_ImExporterSEPA_Export_Pain_008(AB_IMEXPORTER *ie,
                                             AB_IMEXPORTER_CONTEXT *ctx,
                                             GWEN_XMLNODE *painNode,
                                             uint32_t doctype[],
                                             GWEN_DB_NODE *params);


#endif /* AQHBCI_IMEX_SEPA_P_H */
