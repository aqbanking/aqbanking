/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Apr 29 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dtaus_p.h"
#include "dtaus-import_p.h"
#include "dtaus-export_p.h"
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>



GWEN_DBIO *dtaus_factory() {
  GWEN_DBIO *dbio;

  dbio=GWEN_DBIO_new("dtaus", "Imports and exports DTAUS data");
  GWEN_DBIO_SetImportFn(dbio, AHB_DTAUS__Import);
  GWEN_DBIO_SetExportFn(dbio, AHB_DTAUS__Export);
  return dbio;
}










