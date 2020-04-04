/***************************************************************************
    begin       : Sat Apr 04 2020
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_IMEX_XML_SEPA_IMP_H
#define AQBANKING_IMEX_XML_SEPA_IMP_H


#include <aqbanking/backendsupport/imexporter_be.h>


int AB_ImExporterXML_ImportSepa(AB_IMEXPORTER *ie,
                                AB_IMEXPORTER_CONTEXT *ctx,
                                GWEN_SYNCIO *sio,
                                GWEN_DB_NODE *dbParams);




#endif /* AQBANKING_IMEX_XML_SEPA_IMP_H */
