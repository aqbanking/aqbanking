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


/**
 * Creates an OFX data cache for www.ofxhome.com.
 * @param datafolder cache folder
 */
AQOFXCONNECT_API OFXHOME *OfxHome_new(const char *dataFolder);


/**
 * Release the OFX data cache (this doesn't remove the files from the data folder)
 *
 * @param ofh pointer to the OFX data cache object to release
 */
AQOFXCONNECT_API void OfxHome_free(OFXHOME *ofh);


/**
 * Returns a list of server specs (containing name and id). If there already is a list
 * in the data folder and it is reasonably new it will be loaded. Otherwise the list
 * is downloaded from "www.ofxhome.com" and saved for later reference.
 *
 * @param ofh pointer to the OFX data cache object
 *
 * @return pointer to the list of server specs (or NULL on error)
 */
AQOFXCONNECT_API const OH_INSTITUTE_SPEC_LIST *OfxHome_GetSpecs(OFXHOME *ofh);


/**
 * Returns information about the server of the given id.
 * If this data is already in the data folder and is reasonably new
 * it will be loaded. Otherwise it will be downloaded from "www.ofxhome.com"
 * and saved for later reference.
 *
 * @param ofh pointer to the OFX data cache object
 *
 * @param fid id of the server for which information is to be retrieved (this id can
 *   only be taken from a server spec retrieved via @ref OfxHome_GetSpecs).
 *
 * @return pointer to the list of server specs (or NULL on error)
 */
AQOFXCONNECT_API const OH_INSTITUTE_DATA *OfxHome_GetData(OFXHOME *ofh, int fid);


#endif



