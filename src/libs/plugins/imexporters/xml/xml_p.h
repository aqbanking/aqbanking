/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_IMEX_XML_P_H
#define AQBANKING_IMEX_XML_P_H


#include "xml.h"

#include <aqbanking/backendsupport/imexporter_be.h>

#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/buffer.h>



typedef struct AB_IMEXPORTER_XML AB_IMEXPORTER_XML;
struct AB_IMEXPORTER_XML {
  int dummy;
};



GWEN_XMLNODE *AB_ImExporterXML_ReadSchemaFromFile(AB_IMEXPORTER *ie, const char *schemaName);
GWEN_XMLNODE *AB_ImExporterXML_DetermineSchema(AB_IMEXPORTER *ie, GWEN_XMLNODE *xmlDocData);
GWEN_XMLNODE *AB_ImExporterXML_ReadXmlFromSio(AB_IMEXPORTER *ie, GWEN_SYNCIO *sio, uint32_t xmlFlags);



#endif /* AQBANKING_IMEX_XML_P_H */
