/***************************************************************************
 $RCSfile: user.c,v $
                             -------------------
    cvs         : $Id: user.c,v 1.4 2006/01/17 22:58:29 aquamaniac Exp $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_l.h"
#include "aqebics.h"

#include <aqbanking/system.h>
#include <aqbanking/provider_be.h>
#include <gwenhywfar/plugin.h>


AB_PROVIDER *AB_Plugin_ProviderEBICS_Factory(GWEN_PLUGIN *pl, AB_BANKING *ab){
  return EBC_Provider_new(ab);
}



/* interface to gwens plugin loader */
AQBANKING_EXPORT
GWEN_PLUGIN *provider_aqebics_factory(GWEN_PLUGIN_MANAGER *pm,
                                      const char *name,
                                      const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_Provider_new(pm, name, fileName);
  AB_Plugin_Provider_SetFactoryFn(pl, AB_Plugin_ProviderEBICS_Factory);
  return pl;
}


int EBC_Plugins_Init() {
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *p;

  /* providers */
  pm=GWEN_PluginManager_FindPluginManager("provider");
  if (pm) {
    p=provider_aqebics_factory(pm, "aqebics", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
  }
  return 0;
}


