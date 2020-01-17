/***************************************************************************
 begin       : Thu Jan 16 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AO_IO_NETWORK_H
#define AO_IO_NETWORK_H


/* plugin headers */
#include <aqofxconnect/aqofxconnect.h>

/* aqbanking headers */
#include <aqbanking/backendsupport/user.h>

/* gwenhywfar headers */
#include <gwenhywfar/buffer.h>



int AO_V2_SendAndReceive(AB_PROVIDER *pro, AB_USER *u, const uint8_t *p, unsigned int plen, GWEN_BUFFER **pRbuf);


#endif

