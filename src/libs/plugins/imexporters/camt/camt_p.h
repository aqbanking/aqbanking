/***************************************************************************
    begin       : Sat Dec 15 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_IMEX_CAMT_P_H
#define AQBANKING_IMEX_CAMT_P_H


#include "camt.h"

#include <aqbanking/imexporter_be.h>


typedef struct AH_IMEXPORTER_CAMT AH_IMEXPORTER_CAMT;
struct AH_IMEXPORTER_CAMT {
  int dummy;
};


static void GWENHYWFAR_CB AH_ImExporterCAMT_FreeData(void *bp, void *p);

static int AH_ImExporterCAMT_Import(AB_IMEXPORTER *ie,
                                    AB_IMEXPORTER_CONTEXT *ctx,
                                    GWEN_SYNCIO *sio,
                                    GWEN_DB_NODE *params);

static int AH_ImExporterCAMT_Export(AB_IMEXPORTER *ie,
                                    AB_IMEXPORTER_CONTEXT *ctx,
                                    GWEN_SYNCIO *sio,
                                    GWEN_DB_NODE *params);

static int AH_ImExporterCAMT_CheckFile(AB_IMEXPORTER *ie, const char *fname);


static int AH_ImExporterCAMT_Import_052_001_02(AB_IMEXPORTER *ie,
                                               AB_IMEXPORTER_CONTEXT *ctx,
                                               GWEN_DB_NODE *params,
                                               GWEN_XMLNODE *xmlRoot);



#endif /* AQBANKING_IMEX_CAMT_P_H */
