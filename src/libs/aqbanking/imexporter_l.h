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


#ifndef AQBANKING_IMEXPORTER_L_H
#define AQBANKING_IMEXPORTER_L_H

#define AB_IMEXPORTER_FOLDER "imexporters"


#include <aqbanking/imexporter.h>
#include <aqbanking/imexporter_be.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/libloader.h>


GWEN_LIST_FUNCTION_LIB_DEFS(AB_IMEXPORTER, AB_ImExporter, AQBANKING_API)


void AB_ImExporter_SetLibLoader(AB_IMEXPORTER *ie, GWEN_LIBLOADER *ll);


#endif /* AQBANKING_IMEXPORTER_L_H */


