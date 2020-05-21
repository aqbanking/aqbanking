/***************************************************************************
 begin       : Wed Jul 31 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_TRANSPORTHBCI_P_H
#define AQFINTS_TRANSPORTHBCI_P_H


#include "transporthbci.h"

#include <gwenhywfar/syncio.h>


typedef struct AQFINTS_TRANSPORT_HBCI AQFINTS_TRANSPORT_HBCI;
struct AQFINTS_TRANSPORT_HBCI {
  GWEN_SYNCIO *ioLayer;
};



#endif

