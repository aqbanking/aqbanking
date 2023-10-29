/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AG_META_P_H
#define AG_META_P_H

#include "meta.h"


struct AG_META {
    int total_pages;
    int current_page;
    int page_size;
    int total_entries;
};



#endif

