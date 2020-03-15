/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AO_V1_N_MESSAGE_H
#define AO_V1_N_MESSAGE_H


/* plugin headers */
#include <aqofxconnect/aqofxconnect.h>

/* aqbanking headers */
#include <aqbanking/backendsupport/user.h>

/* gwenhywfar headers */
#include <gwenhywfar/xml.h>
#include <gwenhywfar/buffer.h>



int AO_V1_AddOfxHeaders(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *buf);


#endif



