/***************************************************************************
 begin       : Wed Dec 05 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQPAYPAL_CONTROL_BE_H
#define AQPAYPAL_CONTROL_BE_H


#include <aqbanking/backendsupport/provider_be.h>


int APY_Control(AB_PROVIDER *pro, int argc, char **argv);


#endif

