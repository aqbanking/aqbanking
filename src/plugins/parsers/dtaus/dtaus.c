/***************************************************************************
 begin       : Thu Apr 29 2004
 copyright   : (C) 2004-2010 by Martin Preuss
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
#include <gwenhywfar/dbio_be.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>



GWEN_DBIO *AHB_DTAUS_Factory(GWEN_PLUGIN *pl) {
  GWEN_DBIO *dbio;

  dbio=GWEN_DBIO_new("dtaus", "Imports and exports DTAUS data");
  GWEN_DBIO_SetImportFn(dbio, AHB_DTAUS__Import);
  GWEN_DBIO_SetExportFn(dbio, AHB_DTAUS__Export);
  GWEN_DBIO_SetCheckFileFn(dbio, AHB_DTAUS__CheckFile);
  return dbio;
}



GWEN_PLUGIN *dbio_dtaus_factory(GWEN_PLUGIN_MANAGER *pm,
                                const char *modName,
                                const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=GWEN_DBIO_Plugin_new(pm, modName, fileName);
  assert(pl);

  GWEN_DBIO_Plugin_SetFactoryFn(pl, AHB_DTAUS_Factory);

  return pl;

}








