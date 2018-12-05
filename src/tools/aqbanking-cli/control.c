/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



static int control(AB_BANKING *ab, const char *ctrlBackend, GWEN_DB_NODE *dbArgs, int argc, char **argv) {
  int rv;

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }


  rv=AB_Banking_ProviderControl(ab, ctrlBackend, argc, argv);
  if (rv!=0) {
    DBG_ERROR(0, "Error calling control function (%d)", rv);
    AB_Banking_Fini(ab);
    return 4;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}









