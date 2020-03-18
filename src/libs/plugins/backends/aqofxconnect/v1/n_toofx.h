/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AO_V1_N_TOOFX_H
#define AO_V1_N_TOOFX_H


/* plugin headers */
#include <aqofxconnect/aqofxconnect.h>

/* aqbanking headers */
#include <aqbanking/backendsupport/user.h>

/* gwenhywfar headers */
#include <gwenhywfar/xml.h>
#include <gwenhywfar/buffer.h>



int AO_V1_XmlToOfx(GWEN_XMLNODE *xmlNode, GWEN_BUFFER *buf, const char *encoding);


#endif



