/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_VOUCHER_P_H
#define AG_VOUCHER_P_H

#include "provider.h"
#include "voucherowner.h"
#include "voucher.h"



struct AG_VOUCHER {
    char *id;
    AG_VOUCHEROWNER *owner;
};

#endif
