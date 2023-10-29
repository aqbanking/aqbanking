/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_VOUCHER_OWNER_H
#define AG_VOUCHER_OWNER_H

#include "provider.h"

typedef struct AG_VOUCHEROWNER AG_VOUCHEROWNER;

AG_VOUCHEROWNER *AG_VOUCHEROWNER_new (const char *name);
void AG_VOUCHEROWNER_free ( AG_VOUCHEROWNER *o);
void AG_VOUCHEROWNER_SetName ( AG_VOUCHEROWNER *o, const char *name);
const char *AG_VOUCHEROWNER_GetName (const AG_VOUCHEROWNER *o);


#endif
