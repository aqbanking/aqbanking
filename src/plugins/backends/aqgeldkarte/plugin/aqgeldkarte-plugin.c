#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#undef BUILDING_AQGELDKARTE

#include "provider.h"
#include "aqgeldkarte_l.h"


#include <gwenhywfar/plugin.h>


/* interface to gwens plugin loader */
GWEN_PLUGIN *provider_aqgeldkarte_factory(GWEN_PLUGIN_MANAGER *pm,
                                          const char *name,
                                          const char *fileName) {
  return GWEN_Plugin_new(pm, name, fileName);
}


AB_PROVIDER *aqgeldkarte_factory(AB_BANKING *ab){
  return AG_Provider_new(ab);
}
