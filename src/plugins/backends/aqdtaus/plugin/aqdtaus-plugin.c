#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#undef BUILDING_AQDTAUS_DLL
#undef BUILDING_DLL

#include "provider.h"
#include "provider_p.h"
#include "aqdtaus_l.h"


#include <gwenhywfar/plugin.h>


/* interface to gwens plugin loader */
GWEN_PLUGIN *provider_aqdtaus_factory(GWEN_PLUGIN_MANAGER *pm,
                                      const char *name,
                                      const char *fileName) {
  return GWEN_Plugin_new(pm, name, fileName);
}


AB_PROVIDER *aqdtaus_factory(AB_BANKING *ab){
  return AD_Provider_new(ab);
}
