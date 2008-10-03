/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "us_p.h"
#include "../generic/generic_l.h"

#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



GWEN_INHERIT(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_US);

/* interface to gwens plugin loader */
GWEN_PLUGIN *bankinfo_us_factory(GWEN_PLUGIN_MANAGER *pm,
                                 const char *name,
                                 const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_BankInfo_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_BankInfo_SetFactoryFn(pl, AB_Plugin_BankInfoUS_Factory);

  return pl;
}



/* interface to bankinfo plugin */
AB_BANKINFO_PLUGIN *AB_Plugin_BankInfoUS_Factory(GWEN_PLUGIN *pl, AB_BANKING *ab){
  AB_BANKINFO_PLUGIN *bip;
  AB_BANKINFO_PLUGIN_US *bde;

  bip=AB_BankInfoPluginGENERIC_new(ab, "us");
  GWEN_NEW_OBJECT(AB_BANKINFO_PLUGIN_US, bde);
  GWEN_INHERIT_SETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_US,
                       bip, bde, AB_BankInfoPluginUS_FreeData);

  bde->banking=ab;
  return bip;
}



void GWENHYWFAR_CB AB_BankInfoPluginUS_FreeData(void *bp, void *p){
  AB_BANKINFO_PLUGIN_US *bde;

  bde=(AB_BANKINFO_PLUGIN_US*)p;

  GWEN_FREE_OBJECT(bde);
}



