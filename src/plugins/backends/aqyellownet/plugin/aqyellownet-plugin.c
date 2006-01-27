#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#undef BUILDING_AQYELLOWNET
#undef BUILDING_DLL

#include "provider.h"
#include "aqyellownet.h"


#include <gwenhywfar/plugin.h>


/* interface to gwens plugin loader */
AQBANKING_EXPORT
GWEN_PLUGIN *provider_aqyellownet_factory(GWEN_PLUGIN_MANAGER *pm,
                                          const char *name,
                                          const char *fileName) {
  return GWEN_Plugin_new(pm, name, fileName);
}


AQBANKING_EXPORT
AB_PROVIDER *aqyellownet_factory(AB_BANKING *ab){
  return AY_Provider_new(ab);
}
