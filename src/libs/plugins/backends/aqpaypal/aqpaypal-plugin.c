/***************************************************************************
    begin       : Sat May 08 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_l.h"
#include "aqpaypal.h"

#include <aqbanking/system.h>
#include <aqbanking/provider_be.h>
#include <gwenhywfar/plugin.h>


AB_PROVIDER *AB_Plugin_ProviderPaypal_Factory(GWEN_PLUGIN *pl, AB_BANKING *ab)
{
  return APY_Provider_new(ab);
}



/* interface to gwens plugin loader */
AQBANKING_EXPORT
GWEN_PLUGIN *provider_aqpaypal_factory(GWEN_PLUGIN_MANAGER *pm,
                                       const char *name,
                                       const char *fileName)
{
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_Provider_new(pm, name, fileName);
  AB_Plugin_Provider_SetFactoryFn(pl, AB_Plugin_ProviderPaypal_Factory);
  return pl;
}


