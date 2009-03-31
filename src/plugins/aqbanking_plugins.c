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
#include "banking_l.h"
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



GWEN_PLUGIN *imexporters_csv_factory(GWEN_PLUGIN_MANAGER *pm,
				     const char *name,
				     const char *fileName);

GWEN_PLUGIN *imexporters_dtaus_factory(GWEN_PLUGIN_MANAGER *pm,
				       const char *name,
				       const char *fileName);

GWEN_PLUGIN *imexporters_eri2_factory(GWEN_PLUGIN_MANAGER *pm,
				      const char *name,
				      const char *fileName);

GWEN_PLUGIN *imexporters_ofx_factory(GWEN_PLUGIN_MANAGER *pm,
				     const char *name,
				     const char *fileName);

GWEN_PLUGIN *imexporters_openhbci1_factory(GWEN_PLUGIN_MANAGER *pm,
					   const char *name,
					   const char *fileName);

GWEN_PLUGIN *imexporters_swift_factory(GWEN_PLUGIN_MANAGER *pm,
				       const char *name,
				       const char *fileName);

GWEN_PLUGIN *imexporters_xmldb_factory(GWEN_PLUGIN_MANAGER *pm,
				       const char *name,
				       const char *fileName);

GWEN_PLUGIN *imexporters_yellownet_factory(GWEN_PLUGIN_MANAGER *pm,
					   const char *name,
					   const char *fileName);

GWEN_PLUGIN *imexporters_sepa_factory(GWEN_PLUGIN_MANAGER *pm,
				      const char *name,
				      const char *fileName);

GWEN_PLUGIN *imexporters_ctxfile_factory(GWEN_PLUGIN_MANAGER *pm,
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

GWEN_PLUGIN *dbio_dtaus_factory(GWEN_PLUGIN_MANAGER *pm,
				const char *modName,
				const char *fileName);

GWEN_PLUGIN *dbio_swift_factory(GWEN_PLUGIN_MANAGER *pm,
				const char *modName,
				const char *fileName);




int AB_Plugins_Init() {
#ifdef AQBANKING_ENABLE_INIT_PLUGINS
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *p;

  /* initialize AqBanking's plugin system (register plugin managers etc) */
  AB_Banking_PluginSystemInit();

  /* providers */
  pm=GWEN_PluginManager_FindPluginManager("provider");
  if (pm) {
#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQHBCI
    p=provider_aqhbci_factory(pm, "aqhbci", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQNONE
    p=provider_aqnone_factory(pm, "aqnone", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQOFXCONNECT
    p=provider_aqofxconnect_factory(pm, "aqofxconnect", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
  }

  /* imexporters */
  pm=GWEN_PluginManager_FindPluginManager("imexporters");
  if (pm) {
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_CSV
    p=imexporters_csv_factory(pm, "csv", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_DTAUS
    p=imexporters_dtaus_factory(pm, "dtaus", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_ERI2
    p=imexporters_eri2_factory(pm, "eri2", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_OFX
    p=imexporters_ofx_factory(pm, "ofx", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_OPENHBCI1
    p=imexporters_openhbci1_factory(pm, "openhbci1", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_SWIFT
    p=imexporters_swift_factory(pm, "swift", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_XMLDB
    p=imexporters_xmldb_factory(pm, "xmldb", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_YELLOWNET
    p=imexporters_yellownet_factory(pm, "yellownet", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_SEPA
    p=imexporters_sepa_factory(pm, "sepa", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_CTXFILE
    p=imexporters_ctxfile_factory(pm, "ctxfile", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
  }

  /* bankinfo */
  pm=GWEN_PluginManager_FindPluginManager("bankinfo");
  if (pm) {
#ifdef AQBANKING_WITH_PLUGIN_BANKINFO_AT
    p=bankinfo_at_factory(pm, "at", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_BANKINFO_CA
    p=bankinfo_ca_factory(pm, "ca", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_BANKINFO_CH
    p=bankinfo_ch_factory(pm, "ch", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_BANKINFO_DE
    p=bankinfo_de_factory(pm, "de", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_BANKINFO_US
    p=bankinfo_us_factory(pm, "us", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
  }

  /* DBIO parsers */
  pm=GWEN_PluginManager_FindPluginManager("dbio");
  if (pm) {
#ifdef AQBANKING_WITH_PLUGIN_PARSERS_DTAUS
    p=dbio_dtaus_factory(pm, "dtaus", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
#ifdef AQBANKING_WITH_PLUGIN_PARSERS_SWIFT
    p=dbio_swift_factory(pm, "swift", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
  }

#else
  /* only initialize AqBanking's plugin system */
  AB_Banking_PluginSystemInit();
#endif
  return 0;
}


