/***************************************************************************
 begin       : Wed Jul 31 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_TRANSPORTHBCI_H
#define AQFINTS_TRANSPORTHBCI_H


#include "transport/transport.h"

#include <gwenhywfar/syncio.h>



/**
 * Create a transport layer for traditional HBCI (port 3000).
 */
AQFINTS_TRANSPORT *AQFINTS_TransportHbci_new(const char *url);

/**
 * Create a transport layer for traditional HBCI (port 3000) using an already open GWEN_SYNCIO.
 */
AQFINTS_TRANSPORT *AQFINTS_TransportHbci_fromSyncIo(GWEN_SYNCIO *sio);



#endif

