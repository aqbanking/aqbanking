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

#include "voucher_p.h"
#include <string.h>



AG_VOUCHER *AG_VOUCHER_new(const char *id, AG_VOUCHEROWNER *o)
{
  AG_VOUCHER *c = malloc(sizeof(AG_VOUCHER));

  c->id = NULL;
  AG_VOUCHER_SetID(c, id);

  c->owner = NULL;
  AG_VOUCHER_SetOwner(c, o);

  return c;
}



void AG_VOUCHER_SetID(AG_VOUCHER *card, const char *id)
{
  if (card) {
    card->id = strdup(id);
  }
}



void AG_VOUCHER_SetOwner(AG_VOUCHER *card, AG_VOUCHEROWNER *o)
{
  if (card) {
    card->owner = o;
  }
}



void AG_VOUCHER_free(AG_VOUCHER *card)
{
  if (card) {
    AG_VOUCHEROWNER_free(card->owner);
    free(card->id);
    free(card);
  }
}



const char *AG_VOUCHER_GetID(AG_VOUCHER *card)
{
  char *id = NULL;
  if (card) {
    id = card->id;
  }
  return id;
}



const AG_VOUCHEROWNER *AG_VOUCHER_GetOwner(const AG_VOUCHER *card)
{
  AG_VOUCHEROWNER *o = NULL;
  if (card) {
    o = card->owner;

  }
  return o;
}


