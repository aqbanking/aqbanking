#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#undef BUILDING_AQHBCI_DLL
#undef BUILDING_DLL

#include "banking/provider.h"
#include "banking/provider_p.h"

#include <gwenhywfar/plugin.h>


/* interface to gwens plugin loader */
GWEN_PLUGIN *provider_aqhbci_factory(GWEN_PLUGIN_MANAGER *pm,
				     const char *name,
				     const char *fileName) {
  return GWEN_Plugin_new(pm, name, fileName);
}


AB_PROVIDER *aqhbci_factory(AB_BANKING *ab){
  return AH_Provider_new(ab, AH_PROVIDER_NAME);
}
