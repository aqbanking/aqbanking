/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_VOUCHER_H
#define AG_VOUCHER_H

#include "provider.h"
#include "voucher.h"
#include "voucherowner.h"




typedef struct AG_VOUCHER AG_VOUCHER;

AG_VOUCHER *AG_VOUCHER_new (const char *id, AG_VOUCHEROWNER *o);
void AG_VOUCHER_SetID ( AG_VOUCHER *card, const char *id);
void AG_VOUCHER_SetOwner ( AG_VOUCHER *card, AG_VOUCHEROWNER *o);
const char *AG_VOUCHER_GetID ( AG_VOUCHER *card);
const AG_VOUCHEROWNER *AG_VOUCHER_GetOwner (const AG_VOUCHER *card);
void AG_VOUCHER_free ( AG_VOUCHER *card);

#endif
