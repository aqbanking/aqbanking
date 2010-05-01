/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"
#include <gwenhywfar/gwenhywfar.h>
#include <aqbanking/banking.h>


static
int versions(AB_BANKING *ab,
	     GWEN_DB_NODE *dbArgs,
	     int argc,
	     char **argv) {
  int vmajor, vminor, vpatchLevel, vbuild;

  fprintf(stdout, "Versions:\n");
  fprintf(stdout, " AqBanking-CLI: %s\n",
	  AQBANKING_CLI_VERSION_STRING);
  GWEN_Version(&vmajor,
	       &vminor,
	       &vpatchLevel,
	       &vbuild);
  fprintf(stdout, " Gwenhywfar   : %d.%d.%d.%d\n",
	  vmajor, vminor, vpatchLevel, vbuild);

  AB_Banking_GetVersion(&vmajor,
			&vminor,
			&vpatchLevel,
			&vbuild);
  fprintf(stdout, " AqBanking    : %d.%d.%d.%d\n",
	  vmajor, vminor, vpatchLevel, vbuild);

  return 0;
}






