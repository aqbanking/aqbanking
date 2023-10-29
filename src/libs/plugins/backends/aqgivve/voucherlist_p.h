/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_VOUCHERLIST_P_H
#define AG_VOUCHERLIST_P_H

#include "provider.h"
#include "voucherlist.h"

struct AG_VOUCHERLIST {
    AG_VOUCHER **cards;      //array of cards;
    int total_entries;
};

#endif

