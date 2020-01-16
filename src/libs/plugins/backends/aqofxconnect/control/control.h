/***************************************************************************
 begin       : Thu Jan 16 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AO_CONTROL_BE_H
#define AO_CONTROL_BE_H


#include <aqofxconnect/aqofxconnect.h>

#include <aqbanking/backendsupport/provider_be.h>



int AO_Control(AB_PROVIDER *pro, int argc, char **argv);


#endif

