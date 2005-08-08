#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#undef BUILDING_AQOFXCONNECT_DLL
#undef BUILDING_DLL

#include "provider.h"
#include "provider_p.h"
#include "aqofxconnect_l.h"


#include <gwenhywfar/plugin.h>


/* interface to gwens plugin loader */
GWEN_PLUGIN *provider_aqofxconnect_factory(GWEN_PLUGIN_MANAGER *pm,
                                           const char *name,
                                           const char *fileName) {
  return GWEN_Plugin_new(pm, name, fileName);
}


AB_PROVIDER *aqofxconnect_factory(AB_BANKING *ab){
  return AO_Provider_new(ab);
}
