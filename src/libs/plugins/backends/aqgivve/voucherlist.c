/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#include "voucherlist_p.h"

AG_VOUCHERLIST * AG_VOUCHERLIST_new()
{
    AG_VOUCHERLIST *l = malloc ( sizeof ( AG_VOUCHERLIST ) );
    l->cards = malloc ( 1 );
    l->total_entries = 0;

    return l;
}

void AG_VOUCHERLIST_free ( AG_VOUCHERLIST* list, int free_vouchers )
{
    if ( list ) {
        if (free_vouchers) {
        for (int n = 0; n < list->total_entries; n++) {
                AG_VOUCHER_free ( list->cards[n] );
        }
        }
            
        free ( list->cards );
        free ( list );
    }
}

void AG_VOUCHERLIST_AddCard ( AG_VOUCHERLIST* list, AG_VOUCHER* card )
{
    if ( list && card ) {
        list->total_entries++;
        list->cards = realloc ( list->cards, list->total_entries * sizeof ( AG_VOUCHER * ) );
        list->cards[list->total_entries - 1] = card;

    }
}

int AG_VOUCHERLIST_Get_TotalEntries ( AG_VOUCHERLIST* list )
{
    int t = 0;
    if ( list ) {
        t = list->total_entries;
    }
    return t;
}


AG_VOUCHER *AG_VOUCHERLIST_Get_Card_By_ID ( AG_VOUCHERLIST *list, const char* id )
{
    AG_VOUCHER *res;
    res = NULL;
    if ( list ) {
        int total_entries = AG_VOUCHERLIST_Get_TotalEntries ( list );
        int n = 0;
        while ( !res && ( n < total_entries ) ) {
            AG_VOUCHER *c = AG_VOUCHERLIST_Get_Card_By_Index ( list, n );
            if ( c ) {
                const char *c_id = AG_VOUCHER_GetID ( c );
                if ( strcmp ( c_id, id ) == 0 ) {
                    res = c;
                }
            }
            n++;
        }
    }

    return res;

}

AG_VOUCHER *AG_VOUCHERLIST_Get_Card_By_Index ( AG_VOUCHERLIST *list, int index )
{
    AG_VOUCHER *res = NULL;
    if ( list ) {
        if ( index < list->total_entries ) {
            res = list->cards[index];
        }
    }
    return res;
}
