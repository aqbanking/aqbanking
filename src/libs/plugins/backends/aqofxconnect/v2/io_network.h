/***************************************************************************
 begin       : Thu Jan 16 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AO_IO_NETWORK_H
#define AO_IO_NETWORK_H




int AO_V2_SendAndReceive(AB_USER *u, const uint8_t *p, unsigned int plen, GWEN_BUFFER **pRbuf);


#endif

