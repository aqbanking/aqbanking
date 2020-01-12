/***************************************************************************
    begin       : Sun Jun 02 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_TAN_MECHANISM_CHIPTAN_OPT_H
#define AH_TAN_MECHANISM_CHIPTAN_OPT_H


#include "tanmechanism.h"



AH_TAN_MECHANISM *AH_TanMechanism_ChipTanOpt_new(const AH_TAN_METHOD *tanMethod, int tanMethodId);
AH_TAN_MECHANISM *AH_TanMechanism_ChipTanUSB_new(const AH_TAN_METHOD *tanMethod, int tanMethodId);// USB_TAN



#endif

