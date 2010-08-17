/***************************************************************************
    begin       : Tue Aug 17 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef OFXHOME_H
#define OFXHOME_H


#include <aqofxconnect/aqofxconnect.h>
#include <aqofxconnect/oh_institute_data.h>
#include <aqofxconnect/oh_institute_spec.h>

#include <gwenhywfar/xml.h>

typedef struct OFXHOME OFXHOME;


AQOFXCONNECT_API OFXHOME *OfxHome_new(const char *dataFolder);
AQOFXCONNECT_API void OfxHome_free(OFXHOME *ofh);


AQOFXCONNECT_API const OH_INSTITUTE_SPEC_LIST *OfxHome_GetSpecs(OFXHOME *ofh);
AQOFXCONNECT_API const OH_INSTITUTE_DATA *OfxHome_GetData(OFXHOME *ofh, int fid);


#endif



