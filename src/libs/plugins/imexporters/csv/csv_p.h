/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_IMEX_CSV_P_H
#define AQHBCI_IMEX_CSV_P_H

#include "csv.h"

#include <gwenhywfar/dbio.h>
#include <aqbanking/backendsupport/imexporter_be.h>


typedef struct AH_IMEXPORTER_CSV AH_IMEXPORTER_CSV;
struct AH_IMEXPORTER_CSV {
  GWEN_DBIO *dbio;
};




#endif /* AQHBCI_IMEX_CSV_P_H */
