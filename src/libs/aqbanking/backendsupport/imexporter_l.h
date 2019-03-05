/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_IMEXPORTER_L_H
#define AQBANKING_IMEXPORTER_L_H

#define AB_IMEXPORTER_FOLDER "imexporters"


#include <aqbanking/backendsupport/imexporter.h>
#include <aqbanking/backendsupport/imexporter_be.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/libloader.h>


GWEN_LIST_FUNCTION_DEFS(AB_IMEXPORTER, AB_ImExporter)


void AB_ImExporter_SetLibLoader(AB_IMEXPORTER *ie, GWEN_LIBLOADER *ll);


#endif /* AQBANKING_IMEXPORTER_L_H */


