/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "voucherowner_p.h"



AG_VOUCHEROWNER *AG_VOUCHEROWNER_new(const char *name)
{
  AG_VOUCHEROWNER *o = malloc(sizeof(AG_VOUCHEROWNER));
  o->name = NULL;
  AG_VOUCHEROWNER_SetName(o, name);
  return o;
}



void AG_VOUCHEROWNER_SetName(AG_VOUCHEROWNER *o, const char *name)
{
  if (o) {
    if (o->name) {
      free(o->name);
    }

    o->name = strdup(name);
  }
}



void AG_VOUCHEROWNER_free(AG_VOUCHEROWNER *o)
{
  if (o) {
    if (o->name)
      free(o->name);

    free(o);
  }
}



const char *AG_VOUCHEROWNER_GetName(const AG_VOUCHEROWNER *o)
{
  char *name = NULL;
  if (o) {
    name = o->name;
  }
  return name;
}

