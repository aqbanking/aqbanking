/***************************************************************************
    begin       : Sun Apr 13 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "aqbanking_plugins.h"
#include <gwenhywfar/plugin.h>



/* declarations of the plugin factory functions */
GWEN_PLUGIN *provider_aqhbci_factory(GWEN_PLUGIN_MANAGER *pm,
				     const char *name,
				     const char *fileName);

GWEN_PLUGIN *provider_aqnone_factory(GWEN_PLUGIN_MANAGER *pm,
				     const char *name,
				     const char *fileName);

GWEN_PLUGIN *provider_aqofxconnect_factory(GWEN_PLUGIN_MANAGER *pm,
					   const char *name,
					   const char *fileName);



GWEN_PLUGIN *bankinfo_at_factory(GWEN_PLUGIN_MANAGER *pm,
				 const char *name,
				 const char *fileName);

GWEN_PLUGIN *bankinfo_ca_factory(GWEN_PLUGIN_MANAGER *pm,
				 const char *name,
				 const char *fileName);

GWEN_PLUGIN *bankinfo_ch_factory(GWEN_PLUGIN_MANAGER *pm,
				 const char *name,
				 const char *fileName);

GWEN_PLUGIN *bankinfo_de_factory(GWEN_PLUGIN_MANAGER *pm,
				 const char *name,
				 const char *fileName);

GWEN_PLUGIN *bankinfo_us_factory(GWEN_PLUGIN_MANAGER *pm,
				 const char *name,
				 const char *fileName);




int AB_Plugins_Init() {
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *p;

  /* providers */
  pm=GWEN_PluginManager_FindPluginManager("provider");
  if (pm) {
#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQHBCI
    p=provider_aqhbci_factory(pm, "aqhbci", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQNONE
    p=provider_aqhbci_factory(pm, "aqnone", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQOFXCONNECT
    p=provider_aqhbci_factory(pm, "aqofxconnect", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
  }

  /* imexporters */
  pm=GWEN_PluginManager_FindPluginManager("imexporters");
  if (pm) {
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_CSV
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_DTAUS
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_ERI2
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_OFX
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_OPENHBCI1
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_SWIFT
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_XMLDB
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_YELLOWNET
#endif
  }

  /* bankinfo */
  pm=GWEN_PluginManager_FindPluginManager("bankinfo");
  if (pm) {
#ifdef AQBANKING_WITH_PLUGIN_BANKINFO_AT
#endif
#ifdef AQBANKING_WITH_PLUGIN_BANKINFO_CA
#endif
#ifdef AQBANKING_WITH_PLUGIN_BANKINFO_CH
#endif
#ifdef AQBANKING_WITH_PLUGIN_BANKINFO_DE
#endif
#ifdef AQBANKING_WITH_PLUGIN_BANKINFO_US
#endif
  }

  /* DBIO parsers */
  pm=GWEN_PluginManager_FindPluginManager("dbio");
  if (pm) {
#ifdef AQBANKING_WITH_PLUGIN_PARSERS_DTAUS
#endif
#ifdef AQBANKING_WITH_PLUGIN_PARSERS_SWIFT
#endif
  }

  return 0;
}


