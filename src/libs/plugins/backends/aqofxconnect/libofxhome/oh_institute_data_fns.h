/***************************************************************************
    begin       : Tue Aug 17 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef OH_INSTITUTE_DATA_FNS_H
#define OH_INSTITUTE_DATA_FNS_H


#include <aqofxconnect/aqofxconnect.h>

#include <gwenhywfar/xml.h>



AQOFXCONNECT_API OH_INSTITUTE_DATA *OH_InstituteData_fromXml(GWEN_XMLNODE *node);

AQOFXCONNECT_API int OH_InstituteData_ReadXml(OH_INSTITUTE_DATA *oh, GWEN_XMLNODE *node);



#endif

